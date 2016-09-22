#include "STEUDAQDataSender.h"
#include <thread>
#include <chrono>
#include <iostream>

STEUDAQDataSender::STEUDAQDataSender(std::vector<std::shared_ptr<UintCircBuff1MByte>> const & circBuffVec, std::string& rcAddr): 
eudaq::Producer("MyDataSender", rcAddr), m_circBuffVec(circBuffVec){
	SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
}

void STEUDAQDataSender::monitorBuffer(){
	std::cout << "Started monitoring buffer!" << std::endl;
	while(true){
		std::cout << "Monitoring ... buffer has size: " << m_circBuffVec.size() << std::endl;
		for(auto& buffer: m_circBuffVec) {
			uint32_t element;
			while(buffer->pop(element)) {
				std::cout << "Popped element: " << element << ", ";
			}
			std::cout << std::endl;
		}
		//	std::this_thread::yield();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

std::thread STEUDAQDataSender::startThread() {
	return std::thread([=]{ monitorBuffer(); });
}
