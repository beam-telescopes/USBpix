#ifndef _DEVICE_H
#define _DEVICE_H

#include <map>
#include <string>

#include "interface.h"
#include "registerproxy.h"

class FwDevice {
	public:
		FwDevice(int _base, USB &_usb) : base(_base), usb(_usb) {}

		RegisterProxy operator[](const std::string &name);

		void printRegisters(void) const;
		void reset(void);

	protected:
		void checkVersion(unsigned int v);

		uint64_t readRegister(const Register &reg) const;
		void writeRegister(const Register &reg, uint64_t val);

		std::map<std::string, Register> registers;

		const int base;
		USB &usb;
};

#endif
