#include "STEUDAQGen2DataSender.h"
#include <defines.h>

#include "eudaq/RawDataEvent.hh"

#include <thread>
#include <chrono>
#include <iostream>
#include <bitset>

STEUDAQGen2DataSender::STEUDAQGen2DataSender(std::string prodName, std::vector<std::shared_ptr<UintCircBuff1MByte>> const & circBuffVec, std::string& rcAddr, int boardID): 
eudaq::Producer(std::move(prodName), rcAddr), 
m_circBuffVec(circBuffVec),
m_killThread(false){
	SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
}

void STEUDAQGen2DataSender::monitorBuffer(){
	std::cout << "Started monitoring buffer!" << std::endl;
	bool waitedGracePeriod = false;

	eudaq::RawDataEvent bore(eudaq::RawDataEvent::BORE("EXAMPLE", m_runNo));
	SendEvent(bore);	
	std::cout << "Sent BORE" << std::endl;

	std::map<int,std::vector<uint32_t>> dataMap;
	std::map<int,int> triggerMap;

	triggerMap[0] = -1;

	while(true){
		std::cout << "Monitoring ... buffer has size: " << m_circBuffVec.size() << std::endl;

		for(auto& buffer: m_circBuffVec) {
			uint32_t element;
			auto& data = dataMap[0];
			auto currentTriggerNo =  triggerMap.at(0);

			while(buffer->pop(element)) {
				std::cout << "Popped element: " << std::bitset<32>(element) << std::endl;
	      		if( TRIGGER_WORD_MACRO(element) ){
					//in case we get a new trigger, we need to send the old data to eudaq, but only if it's not the first trigger
					if(currentTriggerNo!=-1){
						eudaq::RawDataEvent event("EXAMPLE", m_runNo, currentTriggerNo-1);
						event.AddBlock(0, data);
						SendEvent(event);
						data.clear();
						std::cout << "Sent trigger: " << currentTriggerNo-1 << std::endl;
					}
					uint32_t nextElement;
					auto nextWorked = buffer->pop(nextElement);
					if(nextWorked) {
		  				currentTriggerNo = TRIGGER_NUMBER_MACRO2(element, nextElement)%TLU_TRIGGER_AMOUNT;
						std::cout << "Found trigger number: " << currentTriggerNo-1 << std::endl;
					} else {
						std::cout << "PANIC PANIC PANIC!!!!" << std::endl;
						std::cout << "PANIC PANIC PANIC!!!!" << std::endl;
						std::cout << "PANIC PANIC PANIC!!!!" << std::endl;
						std::cout << "PANIC PANIC PANIC!!!!" << std::endl;
						std::cout << "PANIC PANIC PANIC!!!!" << std::endl;
					}
				} else {
					//std::cout << "Found other data" << std::endl;
					data.emplace_back(element);
				}
			}
			std::cout << "Next trigger to be sent should be: " << currentTriggerNo-1 << std::endl;
			triggerMap[0] = currentTriggerNo;
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

void STEUDAQGen2DataSender::OnStartRun (unsigned param){
	m_killThread = false;
	m_runNo = param;
	SetConnectionState(eudaq::ConnectionState::STATE_RUNNING, "Running!");
	auto workerThread = startThread();
	workerThread.detach();
}

void STEUDAQGen2DataSender::OnStopRun (){
	m_killThread = true;
}

std::thread STEUDAQGen2DataSender::startThread() {
	return std::thread([=]{ monitorBuffer(); });
}
