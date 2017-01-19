#include "STEUDAQGen3RndDataSender.h"
#include <defines.h>

#include "eudaq/RawDataEvent.hh"

#include <thread>
#include <chrono>
#include <iostream>
#include <bitset>

#define TLU_TRIGGER_AMOUNT 32767

STEUDAQGen3RndDataSender::STEUDAQGen3RndDataSender(std::string prodName, std::vector<std::shared_ptr<UintCircBuff1MByte>> const & circBuffVec, std::string& rcAddr, int boardID): 
eudaq::Producer(std::move(prodName), rcAddr), 
mt(rd()), 
xDist(40,20),
yDist(167,60),
totDist(5,2),
tot2Dist(3,1),
lv1Dist(6,2),
offX(0,5),
offY(0,10),
uniformInt(1,9),
m_circBuffVec(circBuffVec),
m_killThread(false)
{
	SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
}

static const uint32_t HIGHEST32BIT = 1 << 31;

void STEUDAQGen3RndDataSender::monitorBuffer(){
	std::cout << "Started monitoring buffer!" << std::endl;
	bool waitedGracePeriod = false;

	eudaq::RawDataEvent bore(eudaq::RawDataEvent::BORE("USBPIX_GEN3", m_runNo));
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
					data.push_back(element);
					if(currentTriggerNo != -1) {
						//eudaq::RawDataEvent event("USBPIX_GEN3", m_runNo, currentTriggerNo-1+TLU_TRIGGER_AMOUNT*triggerRollover+triggerRollover);
						eudaq::RawDataEvent event("USBPIX_GEN3", m_runNo, currentTriggerNo);
						auto lv1 = lv1Dist(mt);
						if(lv1 < 0) lv1 = 0;
						else if(lv1 > 15) lv1 = 15;

						size_t ix = 0;

						for(; ix <= lv1; ++ix ){
							uint32_t DHChan1 = ( 1 << 24 ) | ( data_header << 16 );
							uint32_t DHChan2 = ( 2 << 24 ) | ( data_header << 16 );
							uint32_t DHChan3 = ( 4 << 24 ) | ( data_header << 16 );
							data.push_back(DHChan1);
							data.push_back(DHChan2);
							data.push_back(DHChan3);
						} 

						auto xCoord = xDist(mt);
						auto yCoord = yDist(mt);
						auto tot1_1 = totDist(mt);
						auto tot1_2 = totDist(mt);
						auto tot1_3 = totDist(mt);
						auto tot2_1 = tot2Dist(mt);
						auto tot2_2 = tot2Dist(mt);
						auto tot2_3 = tot2Dist(mt);

						auto uInt1 = uniformInt(mt);
						auto uInt2 = uniformInt(mt);
						auto uInt3 = uniformInt(mt);

						if(xCoord < 0) xCoord = 0;
						else if(xCoord > 79) xCoord = 79;
						if(yCoord < 0) yCoord = 0;
						else if(yCoord > 335) yCoord = 335;

						uint32_t tot2_1_val = 0xF;
						uint32_t tot2_2_val = 0xF;
						uint32_t tot2_3_val = 0xF;

						if(uInt1 < 4 && yCoord < 335 ) tot2_1_val = static_cast<uint32_t>(tot2_1);
						if(uInt2 < 7 && yCoord < 335 ) tot2_2_val = static_cast<uint32_t>(tot2_2);
						if(uInt3 < 9 && yCoord < 335 ) tot2_3_val = static_cast<uint32_t>(tot2_3);

						uint32_t DRChan1 = ( 1 << 24 ) | ( static_cast<uint32_t>(xCoord) << 17 ) | ( static_cast<uint32_t>(yCoord) << 8 )
											| (static_cast<uint32_t>(tot1_1) << 4 ) | tot2_1_val;
						uint32_t DRChan2 = ( 2 << 24 ) | ( static_cast<uint32_t>(xCoord) << 17 ) | ( static_cast<uint32_t>(yCoord) << 8 )
											| (static_cast<uint32_t>(tot1_2) << 4 ) | tot2_2_val;
						uint32_t DRChan3 = ( 4 << 24 ) | ( static_cast<uint32_t>(xCoord) << 17 ) | ( static_cast<uint32_t>(yCoord) << 8 )
											| (static_cast<uint32_t>(tot1_3) << 4 ) | tot2_3_val;
						
						data.push_back(DRChan1);
						data.push_back(DRChan2);
						data.push_back(DRChan3);

						for(; ix <= 15; ++ix ){
							uint32_t DHChan1 = ( 1 << 24 ) | ( data_header << 16 );
							uint32_t DHChan2 = ( 2 << 24 ) | ( data_header << 16 );
							uint32_t DHChan3 = ( 4 << 24 ) | ( data_header << 16 );
							data.push_back(DHChan1);
							data.push_back(DHChan2);
							data.push_back(DHChan3);
						} 
/*						
						for(auto entry: data) {
							std::cout << "Data element: " << std::bitset<32>(entry) << std::endl;
						}
*/
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

void STEUDAQGen3RndDataSender::OnStartRun (unsigned param){
	m_killThread = false;
	m_runNo = param;
	SetConnectionState(eudaq::ConnectionState::STATE_RUNNING, "Running!");
	auto workerThread = startThread();
	workerThread.detach();
}

void STEUDAQGen3RndDataSender::OnStopRun (){
	m_killThread = true;
	SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
}

void STEUDAQGen3RndDataSender::OnConfigure(const eudaq::Configuration& config){
	SetConnectionState(eudaq::ConnectionState::STATE_CONF, "Configured (dy default)");
}

std::thread STEUDAQGen3RndDataSender::startThread() {
	return std::thread([=]{ monitorBuffer(); });
}
