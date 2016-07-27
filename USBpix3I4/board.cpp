#include "board.h"
#include "commandgenerator.h"
#include "frontend.h"
//USB board classes
#include "SiLibUSB.h"

#include <chrono>
#include <thread>

#include <iostream>

using namespace std;

Board::Board(SiUSBDevice *dev, int type) : u(dev) {
  // type=10: MMC3, =0: MIO3+SCA, =1: MIO3+BIC
	rx.emplace_back(0x8600, u); // workaround for MSVC
	if(type>0){
	  rx.emplace_back(0x8500, u);
	  rx.emplace_back(0x8400, u);
	  rx.emplace_back(0x8300, u);

	  if(type>9){ //MMC3
	    rx.emplace_back(0x9600, u);
	    rx.emplace_back(0x9500, u);
	    rx.emplace_back(0x9400, u);
	    rx.emplace_back(0x9300, u);
	  }
	}
}

void Board::init(int output_mode) {
	cmd["OUTPUT_MODE"] = output_mode;

	tlu["TRIGGER_LOW_TIMEOUT"] = 0xFF;
	tlu["TRIGGER_MODE"] = TLU::NO_HANDSHAKE;

	for(auto &i : rx) {
		i["INVERT_RX"] = 0;
	}

	for(auto &i : rx) {
		i.rxReset();
	}

	cout << "fifo size: " << fifo["FIFO_SIZE"] << endl;
	cout << "resetting fifo" << endl;
	fifo.reset();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	cout << "fifo size: " << fifo["FIFO_SIZE"] << endl;
}

Board::~Board(void) {
}

void Board::enableTrigger(int trigger_mode) {
	tlu["TRIGGER_COUNTER"] = 0;
	tlu["TRIGGER_MODE"] = trigger_mode;
	cmd["EN_EXT_TRIGGER"] = 1;
}

void Board::disableTrigger(void) {
	cmd["EN_EXT_TRIGGER"] = 0;
	tlu["TRIGGER_MODE"] = TLU::NO_HANDSHAKE;
}

uint32_t Board::getTriggerCounter(void) {
	return tlu["TRIGGER_COUNTER"];
}

void Board::wait(void) {
	wait(cmd["CMD_SIZE"], cmd["CMD_REPEAT"]);
}

void Board::wait(int size, int repeat) {
	std::this_thread::sleep_for(std::chrono::nanoseconds(size * repeat * 25ull));

	while(!cmd["READY"]) {
		//cout << '!' << flush;
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	//cout << ' ';
}

void Board::sendCommands(CommandBuffer &buffer) {
	while(!buffer.empty()) {
		auto buf = buffer.getBuffer(cmd.mem_size);
		int size = buf.size() * 8; //fixme
		//cout << "sending "<< buf.size() << " bytes of commands" << endl;

		cmd["CMD_REPEAT"] = 1;
		cmd["CMD_SIZE"] = size;
		cmd.setData(buf);
		cmd.start();

		wait(size, 1);
	}
}

void Board::sendCommands(CommandBuffer &buffer, int repeat) {
	auto buf = buffer.getBuffer(cmd.mem_size);
	int size = buf.size() * 8; //fixme

	cmd["CMD_REPEAT"] = repeat;
	cmd["CMD_SIZE"] = size;
	cmd.setData(buf);
	cmd.start();

	wait(size, repeat);
}

void Board::setCommand(const Command &c) {
	cmd["CMD_SIZE"] = c.size;
	cmd.setData(c.cmd);
}

void Board::sendCommands(const Command &c, int repeat) {
	cmd["CMD_REPEAT"] = repeat;
	cmd["CMD_SIZE"] = c.size;
	cmd.setData(c.cmd);
	cmd.start();

	wait(c.size, repeat);
}

std::vector<uint32_t> Board::getData(void) {
	return fifo.getData();
}

void Board::printRxStatus(void) {
	int j = 1;
	for(auto &i : rx) {
		cout << "RX" << j << " ready: " << i["READY"] << " error count: " << i["DECODER_ERROR_COUNTER"] << " lost data: " << i["LOST_DATA_COUNTER"] << endl;
		j++;
	}
}

