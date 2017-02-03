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
m_killThread(false),
boardID(boardID){
	SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
}

void STEUDAQGen2DataSender::monitorBuffer(){
	std::cout << "Started monitoring buffer!" << std::endl;
	bool waitedGracePeriod = false;

	eudaq::RawDataEvent bore(eudaq::RawDataEvent::BORE("USBPIX_GEN2", m_runNo));
	bore.SetTag("board", boardID);
	SendEvent(bore);	
	std::cout << "Sent BORE" << std::endl;

	std::map<int,std::vector<uint32_t>> dataMap;
	std::map<int,int> triggerMap;

	bool sendEvents = false;
	size_t finishedEvents = 0;

	while(true){
		std::cout << "Monitoring ... buffer has size: " << m_circBuffVec.size() << std::endl;

		bool initialized = false;

		std::vector<eudaq::RawDataEvent> eudaqEvtVec;

		for(size_t bufferIdx = 0;  bufferIdx < m_circBuffVec.size(); ++bufferIdx) {
			std::cout << "Entered loop for buffer: " << bufferIdx << std::endl;

			if(!initialized) {
				for(size_t ix = 0;  ix < m_circBuffVec.size(); ++ix) {
					triggerMap[ix] = -1;
				}
				initialized = true;
			}

			auto& buffer = m_circBuffVec[bufferIdx];
			uint32_t element;
			auto& data = dataMap[bufferIdx];
			auto currentTriggerNo =  triggerMap.at(bufferIdx);

			size_t internalTriggerCounter = 0;

			auto getEudaqEvt = [&]() -> eudaq::RawDataEvent&  {
				if(bufferIdx == 0) {
					eudaqEvtVec.emplace_back("USBPIX_GEN2", m_runNo, currentTriggerNo-1);
					eudaq::RawDataEvent& event = eudaqEvtVec.back();
					std::cout << "Added event for trigger: " << currentTriggerNo-1 << std::endl;
					event.SetTag("board", boardID);
					return event;
				} else {
					std::cout << "Retrieved event for trigger: " << currentTriggerNo-1 << " at local coordinate: " << internalTriggerCounter << "for buffer: " << bufferIdx << std::endl;
					if(bufferIdx+1 == m_circBuffVec.size()) ++finishedEvents;
					return eudaqEvtVec.at(internalTriggerCounter);
				}
			};

			while(buffer->pop(element)) {
				//std::cout << "Popped element: " << std::bitset<32>(element) << " from buffer: " << bufferIdx << std::endl;
	      		if( TRIGGER_WORD_MACRO(element) ){
					//in case we get a new trigger, we need to attach the old data to the eudaq event, but only if it's not the first trigger
					if(currentTriggerNo!=-1){
						eudaq::RawDataEvent& event = getEudaqEvt();
						++internalTriggerCounter;
						event.AddBlock(bufferIdx, data);
						data.clear();
						//std::cout << "Sent trigger: " << currentTriggerNo-1 << std::endl;
					}
					uint32_t nextElement;
					auto nextWorked = buffer->pop(nextElement);
					if(nextWorked) {
		  				currentTriggerNo = TRIGGER_NUMBER_MACRO2(element, nextElement)%TLU_TRIGGER_AMOUNT;
						//std::cout << "Found trigger number: " << currentTriggerNo-1 << std::endl;
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
			//std::cout << "Next trigger to be sent should be: " << currentTriggerNo-1 << std::endl;
			triggerMap[bufferIdx] = currentTriggerNo;
		}

		for(size_t counter = 0; counter < finishedEvents; counter++){
				SendEvent(eudaqEvtVec.at(counter));
				
		} 
		if( finishedEvents != 0 ) {
			eudaqEvtVec.erase(eudaqEvtVec.begin(), eudaqEvtVec.begin()+finishedEvents);
			finishedEvents = 0;
		}

		//	std::this_thread::yield();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if(m_killThread && !waitedGracePeriod){
			waitedGracePeriod = true;
			std::this_thread::sleep_for(std::chrono::seconds(10));
		} else if (m_killThread && waitedGracePeriod) {
			std::cout << "KILLING THREAD" << std::endl;
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
