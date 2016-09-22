#ifndef STEUDAQDataSender_h 
#define STEUDAQDataSender_h

#include "eudaq/DataSender.hh"
#include "eudaq/Producer.hh"
#include <vector>
#include <thread>
#include <memory>
#include "Utility/CircularFifo.h"

using UintCircBuff1MByte = CircularFifo<uint32_t, 1000000/sizeof(uint32_t)>;

class STEUDAQDataSender: public eudaq::Producer {

public:
	STEUDAQDataSender(std::vector<std::shared_ptr<UintCircBuff1MByte>> const & circBuffVec, std::string& rcAddr);
	std::thread startThread();

private:
	void monitorBuffer();
	std::vector<std::shared_ptr<UintCircBuff1MByte>> const & m_circBuffVec;
};
#endif
