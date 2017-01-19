#ifndef STEUDAQGen3RndDataSender_h 
#define STEUDAQGen3RndDataSender_h

#include "STEUDAQDataSenderInterface.h"

#include "eudaq/DataSender.hh"
#include "eudaq/Producer.hh"

#include <vector>
#include <thread>
#include <memory>
#include <atomic>
#include <random>

#include "Utility/CircularFifo.h"

#define TLU_TRIGGER_AMOUNT 0x8000

class STEUDAQGen3RndDataSender: public STEUDAQDataSenderInterface, public eudaq::Producer  {

public:
	STEUDAQGen3RndDataSender(std::string prodName, std::vector<std::shared_ptr<UintCircBuff1MByte>> const & circBuffVec, std::string& rcAddr, int boardID);
	void OnStartRun(unsigned);
	void OnStopRun();
	void OnConfigure(const eudaq::Configuration&);

private:
    std::random_device rd;
    std::mt19937 mt;
	std::normal_distribution<double> xDist;
	std::normal_distribution<double> yDist;
	std::normal_distribution<double> totDist;
	std::normal_distribution<double> tot2Dist;
	std::normal_distribution<double> lv1Dist;
	std::normal_distribution<double> offX;
	std::normal_distribution<double> offY;
	std::uniform_int_distribution<> uniformInt;

	std::thread startThread();
	void monitorBuffer();
	void SetStateConf();
	std::vector<std::shared_ptr<UintCircBuff1MByte>> const & m_circBuffVec;
	std::atomic<bool> m_killThread;
	std::atomic<unsigned> m_runNo;


	const uint8_t header = 0xE8; 					//0b11101 000
	const uint8_t data_header = header | 0x1;		//0b11101 001
	const uint8_t address_record = header | 0x2;	//0b11101 010
	const uint8_t value_record = header | 0x4;		//0b11101 100
	const uint8_t service_record = header | 0x7;	//0b11101 111
};
#endif
