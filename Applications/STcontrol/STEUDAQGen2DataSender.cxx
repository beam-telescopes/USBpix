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

	std::map<int,std::unique_ptr<std::vector<uint32_t>>> dataMap;
	std::map<int,int> triggerMap;
	std::map<int, Gen2BrdData> brdData;

	while(true){
		std::cout << "Monitoring ... buffer has size: " << m_circBuffVec.size() << std::endl;
		bool initialized = false;
		std::vector<eudaq::RawDataEvent> eudaqEvtVec;

		for(size_t bufferIdx = 0;  bufferIdx < m_circBuffVec.size(); ++bufferIdx) {
			std::cout << "Entered loop for buffer: " << bufferIdx << std::endl;

			if(!initialized) {
				for(size_t ix = 0;  ix < m_circBuffVec.size(); ++ix) {
					triggerMap[ix] = -1;
					dataMap[ix] = std::unique_ptr<std::vector<uint32_t>>(new std::vector<uint32_t>());
				}
				initialized = true;
			}

			auto& buffer = m_circBuffVec[bufferIdx];
			uint32_t element;
			auto& data = dataMap[bufferIdx];
			auto currentTriggerNo =  triggerMap[bufferIdx];

			while(buffer->pop(element)) {
				//std::cout << "Popped element: " << std::bitset<32>(element) << " from buffer: " << bufferIdx << std::endl;
	      		if( TRIGGER_WORD_MACRO(element) ){
					//in case we get a new trigger, we need to attach the old data to the eudaq event, but only if it's not the first trigger
					if(currentTriggerNo!=-1){

						auto& dataEntry = brdData[currentTriggerNo-1];
						if(!dataEntry.sizeSet()) {
							dataEntry.setSize(m_circBuffVec.size());
						}
						dataEntry.pushData(bufferIdx, std::move(data));
						data = std::unique_ptr<std::vector<uint32_t>>(new std::vector<uint32_t>());
						//std::cout << "Pushed data for idx: " << bufferIdx << " at trigger: " << currentTriggerNo-1 << std::endl;
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
					//data->emplace_back(element)
					//data->emplace_back(nextElement)
				} else {
					//std::cout << "Found other data" << std::endl;
					data->emplace_back(element);
				}
			}
			//std::cout << "Next trigger to be sent should be: " << currentTriggerNo-1 << std::endl;
			triggerMap[bufferIdx] = currentTriggerNo;
		}

		std::cout << "Checking triggers! " << std::endl; 
		for(auto it = brdData.begin(); it != brdData.end(); /* no increment */) {
			if(it->second.isDone()) {
				//std::cout << "Trigger " << it->first << " is done!" << std::endl;
				eudaq::RawDataEvent event("USBPIX_GEN2", 0, it->first);
				auto storedData = it->second.releaseData();
				for(size_t ix = 0; ix < storedData.size(); ++ix){
					event.AddBlock(ix, *storedData[ix].get());
					//std::cout << "Adding block for index: " << ix << " of size: " << storedData[ix]->size() << std::endl;
				}
				event.SetTag("board", boardID);
				SendEvent(event);
				brdData.erase(it++);
			} else {
				std::cout << "Trigger " << it->first << " is NOT done!" << std::endl;
				break;
				++it;
			}

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
	SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
}

std::thread STEUDAQGen2DataSender::startThread() {
	return std::thread([=]{ monitorBuffer(); });
}

void STEUDAQGen2DataSender::OnConfigure(const eudaq::Configuration& config){
	SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
}