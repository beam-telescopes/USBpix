#include <stdexcept>
#include <algorithm>
#include <iostream>
//USB board classes
#include "SiLibUSB.h"

#include "interface.h"

std::vector<uint8_t> USB::read(int addr, int length) {
	if(!dev->HandlePresent())
		throw std::runtime_error("Device is NULL in USB::read.");

	std::vector<uint8_t> result(length);
	if(!dev->ReadBlock(addr, &result[0], length))
		throw std::runtime_error("USB::read failed");

	return result;

}

void USB::write(int addr, const std::vector<uint8_t> &data) {
	if(!dev->HandlePresent())
		throw std::runtime_error("Device is NULL in USB::write.");

	if(!dev->WriteBlock(addr, const_cast<uint8_t*>(&data[0]), data.size()))
		throw std::runtime_error("USB::write failed");
}
