#include "STEUDAQDataSender.h"
#include <defines.h>

#include "eudaq/RawDataEvent.hh"

#include <thread>
#include <chrono>
#include <iostream>

STEUDAQDataSender::STEUDAQDataSender(std::vector<std::shared_ptr<UintCircBuff1MByte>> const & circBuffVec, std::string& rcAddr): 
eudaq::Producer("MyDataSender", rcAddr), 
m_circBuffVec(circBuffVec),
m_killThread(false){
	SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
}

void STEUDAQDataSender::monitorBuffer(){
	std::cout << "Started monitoring buffer!" << std::endl;
	bool waitedGracePeriod = false;
	while(true){
		std::cout << "Monitoring ... buffer has size: " << m_circBuffVec.size() << std::endl;
		//std::map<int, std:vector<uint32_t>> dataPackages;
		
		for(auto& buffer: m_circBuffVec) {
			uint32_t element;
			int currentTriggerNo = -1;
			std::vector<uint32_t> data;
			while(buffer->pop(element)) {
				//std::cout << "Popped element: " << element << ", ";
	      		if( TRIGGER_WORD_MACRO(element) ){
					//in case we get a new trigger, we need to send the old data to eudaq, but only if it's not the first trigger
					if(currentTriggerNo!=-1){
						eudaq::RawDataEvent event("EXAMPLE", m_runNo, currentTriggerNo-1);
						event.AddBlock(0, data);
						SendEvent(event);
					}
					uint32_t nextElement;
					buffer->pop(nextElement);
		  			currentTriggerNo = TRIGGER_NUMBER_MACRO2(element, nextElement)%TLU_TRIGGER_AMOUNT;
					std::cout << "Found trigger number: " << currentTriggerNo << std::endl;
				} else {
					std::cout << "Found other data" << std::endl;
					data.emplace_back(element);
				}
			}
			std::cout << std::endl;
		}
		//	std::this_thread::yield();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if(m_killThread && !waitedGracePeriod){
			waitedGracePeriod = true;
			std::this_thread::sleep_for(std::chrono::seconds(10));
		} else if (m_killThread && waitedGracePeriod) {
			std::cout << "KILLING THREAD - YEAH!!!!!" << std::endl;
			SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
			return;
		}
	}
}

void STEUDAQDataSender::OnStartRun (unsigned param){
	m_killThread = false;
	m_runNo = param;
	SetConnectionState(eudaq::ConnectionState::STATE_RUNNING, "Running!");
	auto workerThread = startThread();
	workerThread.detach();
}

void STEUDAQDataSender::OnStopRun (){
	m_killThread = true;
}

std::thread STEUDAQDataSender::startThread() {
	return std::thread([=]{ monitorBuffer(); });
}
