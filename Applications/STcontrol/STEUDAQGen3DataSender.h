#ifndef STEUDAQGen3DataSender_h 
#define STEUDAQGen3DataSender_h

#include "eudaq/DataSender.hh"
#include "eudaq/Producer.hh"

#include <vector>
#include <thread>
#include <memory>
#include <atomic>

#include "Utility/CircularFifo.h"

#define TLU_TRIGGER_AMOUNT	0x8000

class STEUDAQGen3DataSender: public eudaq::Producer {

public:
	STEUDAQGen3DataSender(std::vector<std::shared_ptr<UintCircBuff1MByte>> const & circBuffVec, std::string& rcAddr);
	void OnStartRun(unsigned);
	void OnStopRun();
	void OnConfigure(const eudaq::Configuration&);

private:
	std::thread startThread();
	void monitorBuffer();
	void SetStateConf();
	std::vector<std::shared_ptr<UintCircBuff1MByte>> const & m_circBuffVec;
	std::atomic<bool> m_killThread;
	std::atomic<unsigned> m_runNo;
};
#endif
