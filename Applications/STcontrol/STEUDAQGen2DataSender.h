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

class Gen2BrdData {

  private:
  	size_t mNoBoards;
	size_t mNoOfSuccWrites = 0;
	bool mIsDone = false;
	std::vector<std::unique_ptr<std::vector<uint32_t>>> dataVec;

  public:
	Gen2BrdData(size_t noBoards = 0): mNoBoards(noBoards),  dataVec(noBoards) {};

	bool pushData(size_t idx, std::unique_ptr<std::vector<uint32_t>> data) {
		if(!dataVec[idx]) { 
			dataVec[idx] = std::move(data);
			if(++mNoOfSuccWrites == mNoBoards) mIsDone = true;
			return true;
		} else {
			return false;
		} 
	};

	bool sizeSet() const {
		return mNoBoards != 0;
	};

	void setSize(size_t s) {
		mNoBoards = s;
		dataVec = std::vector<std::unique_ptr<std::vector<uint32_t>>>(mNoBoards);
	};

	bool isDone() const {
		return mIsDone;
	};

	std::vector<std::unique_ptr<std::vector<uint32_t>>> releaseData() {
		return std::move(dataVec);
	};

};

class STEUDAQGen2DataSender: public STEUDAQDataSenderInterface, public eudaq::Producer  {

public:
	STEUDAQGen2DataSender(std::string prodName, std::vector<std::shared_ptr<UintCircBuff1MByte>> const & circBuffVec, std::string& rcAddr, int boardID);
	void OnStartRun(unsigned);
	void OnStopRun();
	void OnConfigure(const eudaq::Configuration&);
	
private:
	std::thread startThread();
	void monitorBuffer();
	std::vector<std::shared_ptr<UintCircBuff1MByte>> const & m_circBuffVec;
	std::atomic<bool> m_killThread;
	std::atomic<unsigned> m_runNo;
	int boardID;
};
#endif
