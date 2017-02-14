#include "STEUDAQGen3DataSender.h"
#include <defines.h>

#include "eudaq/RawDataEvent.hh"

#include <thread>
#include <chrono>
#include <iostream>
#include <bitset>

#define TLU_TRIGGER_AMOUNT 32767

STEUDAQGen3DataSender::STEUDAQGen3DataSender(std::string prodName, std::vector<std::shared_ptr<UintCircBuff1MByte>> const & circBuffVec, std::string& rcAddr, int boardID): 
eudaq::Producer(std::move(prodName), rcAddr), 
m_circBuffVec(circBuffVec),
m_killThread(false),
boardID(boardID){
	SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
}

static const uint32_t HIGHEST32BIT = 1 << 31;

void STEUDAQGen3DataSender::monitorBuffer(){
	std::cout << "Started monitoring buffer!" << std::endl;
	bool waitedGracePeriod = false;

	eudaq::RawDataEvent bore(eudaq::RawDataEvent::BORE("USBPIX_GEN3", m_runNo));
	bore.SetTag("board", boardID);
	SendEvent(bore);	
	//std::cout << "Sent BORE" << std::endl;

	std::vector<uint32_t> data;
	int currentTriggerNo = -1;
	int triggerRollover = 0;	

	while(true){
		//std::cout << "Monitoring ... buffer has size: " << m_circBuffVec.size() << std::endl;
		for(auto& buffer: m_circBuffVec) {
			uint32_t element;
			while(buffer->pop(element)) {
				//In case the highest bit is set it is a trigger
				if( element >> 31 ) {
					if(currentTriggerNo != -1) {
						//eudaq::RawDataEvent event("USBPIX_GEN3", m_runNo, currentTriggerNo-1+TLU_TRIGGER_AMOUNT*triggerRollover+triggerRollover);
						eudaq::RawDataEvent event("USBPIX_GEN3", m_runNo, currentTriggerNo);
						event.AddBlock(0, data);
						event.SetTag("board", boardID);
						SendEvent(event);
						data.clear();
					}
					data.push_back(element);
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
			std::cout << "KILLING THREAD" << std::endl;
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
