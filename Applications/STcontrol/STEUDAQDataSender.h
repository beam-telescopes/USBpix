#ifndef STEUDAQDataSender_h 
#define STEUDAQDataSender_h

#include "eudaq/DataSender.hh"
#include "eudaq/Producer.hh"

#include <vector>
#include <thread>
#include <memory>
#include <atomic>

#include "Utility/CircularFifo.h"

using UintCircBuff1MByte = CircularFifo<uint32_t, 1000000/sizeof(uint32_t)>;
#define TLU_TRIGGER_AMOUNT	0x8000

class STEUDAQDataSender: public eudaq::Producer {

public:
	STEUDAQDataSender(std::vector<std::shared_ptr<UintCircBuff1MByte>> const & circBuffVec, std::string& rcAddr);
	void OnStartRun(unsigned);
	void OnStopRun();

private:
	std::thread startThread();
	void monitorBuffer();
	std::vector<std::shared_ptr<UintCircBuff1MByte>> const & m_circBuffVec;
	std::atomic<bool> m_killThread;
	std::atomic<unsigned> m_runNo;
};
#endif
