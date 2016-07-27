#include <stdexcept>
#include <iostream>

#include "device.h"
#include "util.h"

uint64_t FwDevice::readRegister(const Register &reg) const {
	if(!(reg.type & Register::READ)) {
		throw std::runtime_error("Read access to write-only register");
	}

	int size = reg.size / 8;
	if(reg.size % 8) {
		size++;
	}

	auto result = usb.read(reg.addr + base, size);

	int i = 0;
	uint64_t value = 0;
	for(auto j : result) {
		value |= j << (i * 8);
		i++;
	}

	return selectBits(value, reg.offset, reg.size);
}

void FwDevice::writeRegister(const Register &reg, uint64_t val) {
	if(!(reg.type & Register::WRITE)) {
		throw std::runtime_error("Write access to read-only register");
	}

	val = selectBits(val, 0, reg.size);
	uint64_t value = val;

	int size = reg.size / 8;
	if(reg.size % 8) {
		size++;

		auto data = usb.read(reg.addr + base, size);

		int i = 0;
		value = 0;
		for(auto j : data) {
			value |= j << (i * 8);
			i++;
		}

		value = clearBits(value, reg.offset, reg.size);
		value |= val << reg.offset;
	}

	std::vector<uint8_t> data;

	for(int i = 0; i < size; i++) {
		data.push_back(value & 0xFF);
		value >>= 8;
	}

	usb.write(reg.addr + base, data);
}

void FwDevice::checkVersion(unsigned int v) {
	if((*this)["VERSION"] != v) {
		throw std::runtime_error("Version mismatch");
	}
}

void FwDevice::reset(void) {
	(*this)["RESET"] = 0;
}

RegisterProxy FwDevice::operator[](const std::string &name) {
	return RegisterProxy(std::bind(&FwDevice::readRegister, this, registers.at(name)), std::bind(&FwDevice::writeRegister, this, registers.at(name), std::placeholders::_1));
}

void FwDevice::printRegisters(void) const {
	for(auto const &i : registers) {
		std::cout << i.first << ": ";
		if(i.second.type & Register::READ) {
			std::cout << readRegister(i.second);
		}
		std::cout << std::endl;
	}
}
