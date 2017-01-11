#include "STEUDAQGen3DataSender.h"
#include <defines.h>

#include "eudaq/RawDataEvent.hh"

#include <thread>
#include <chrono>
#include <iostream>
#include <bitset>

#define TLU_TRIGGER_AMOUNT 32767

STEUDAQGen3DataSender::STEUDAQGen3DataSender(std::vector<std::shared_ptr<UintCircBuff1MByte>> const & circBuffVec, std::string& rcAddr): 
eudaq::Producer("MyDataSender", rcAddr), 
m_circBuffVec(circBuffVec),
m_killThread(false){
	SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
}

static const uint32_t HIGHEST32BIT = 1 << 31;

void STEUDAQGen3DataSender::monitorBuffer(){
	std::cout << "Started monitoring buffer!" << std::endl;
	bool waitedGracePeriod = false;

	eudaq::RawDataEvent bore(eudaq::RawDataEvent::BORE("USBPIX_M3", m_runNo));
	SendEvent(bore);	
	std::cout << "Sent BORE" << std::endl;

	std::vector<uint32_t> data;
	int currentTriggerNo = -1;
	int triggerRollover = 0;	

	while(true){
		//std::cout << "Monitoring ... buffer has size: " << m_circBuffVec.size() << std::endl;

		for(auto& buffer: m_circBuffVec) {
			uint32_t element;

			while(buffer->pop(element)) {
				//std::cout << "Popped element: " << std::bitset<32>(element) << std::endl;
				
				//In case the highest bit is set it is a trigger
				if( element >> 31 ) {
					if(currentTriggerNo != -1) {
						//eudaq::RawDataEvent event("USBPIX_M3", m_runNo, currentTriggerNo-1+TLU_TRIGGER_AMOUNT*triggerRollover+triggerRollover);
						eudaq::RawDataEvent event("USBPIX_M3", m_runNo, currentTriggerNo);
						event.AddBlock(0, data);
						SendEvent(event);
						//std::cout << "Sent trigger: " << currentTriggerNo-1 << " payload: " << data.size() << std::endl;	
						data.clear();
					}
					/*
					if(currentTriggerNo == TLU_TRIGGER_AMOUNT) {
						triggerRollover++;
						std::cout << "ROLLOVER!!" << std::endl;
					}*/
					
					++currentTriggerNo;//element&(~HIGHEST32BIT);

					//std::cout << "Trigger: " << currentTriggerNo << std::endl;	
				} else {
					data.push_back(element);
				}
			}
		}
		//	std::this_thread::yield();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if(m_killThread && !waitedGracePeriod){
			waitedGracePeriod = true;
			std::this_thread::sleep_for(std::chrono::seconds(10));
		} else if (m_killThread && waitedGracePeriod) {
			std::cout << "KILLING THREAD - YEAH!!!!!" << std::endl;
			return;
		}
	}
}

void STEUDAQGen3DataSender::OnStartRun (unsigned param){
	m_killThread = false;
	m_runNo = param;
	SetConnectionState(eudaq::ConnectionState::STATE_RUNNING, "Running!");
	auto workerThread = startThread();
	workerThread.detach();
}

void STEUDAQGen3DataSender::OnStopRun (){
	m_killThread = true;
	SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
}

void STEUDAQGen3DataSender::OnConfigure(const eudaq::Configuration& config){
	SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
}

std::thread STEUDAQGen3DataSender::startThread() {
	return std::thread([=]{ monitorBuffer(); });
}
