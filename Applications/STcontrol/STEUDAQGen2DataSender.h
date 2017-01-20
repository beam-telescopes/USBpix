#ifndef STEUDAQGen2DataSender_h 
#define STEUDAQGen2DataSender_h

#include "STEUDAQDataSenderInterface.h"

#include "eudaq/DataSender.hh"
#include "eudaq/Producer.hh"

#include <vector>
#include <thread>
#include <memory>
#include <atomic>

#include "Utility/CircularFifo.h"

#define TLU_TRIGGER_AMOUNT	0x8000

class STEUDAQGen2DataSender: public STEUDAQDataSenderInterface, public eudaq::Producer  {

public:
	STEUDAQGen2DataSender(std::string prodName, std::vector<std::shared_ptr<UintCircBuff1MByte>> const & circBuffVec, std::string& rcAddr, int boardID);
	void OnStartRun(unsigned);
	void OnStopRun();

private:
	std::thread startThread();
	void monitorBuffer();
	std::vector<std::shared_ptr<UintCircBuff1MByte>> const & m_circBuffVec;
	std::atomic<bool> m_killThread;
	std::atomic<unsigned> m_runNo;
	int boardID;
};
#endif
