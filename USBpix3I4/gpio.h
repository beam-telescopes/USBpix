#ifndef _GPIO_H
#define _GPIO_H

#include "device.h"

class GPIO : public FwDevice {
	public:
		GPIO(int _base, USB &_usb,  int _size = 8) : FwDevice(_base, _usb), size(_size) {
			int bytes = (size-1)/8 + 1;

			registers = decltype(registers){ // workaround for MSVC12
				{"RESET", {0, 8, 0, Register::WRITE}},
				{"VERSION", {0, 8, 0, Register::READ}},
				{"INPUT", {1, size, 0, Register::READ}},
				{"OUTPUT", {2 + bytes - 1, size, 0, Register::READ_WRITE}},
				{"OUTPUT_EN", {3 + 2 * (bytes - 1), size, 0, Register::READ_WRITE}}
			};

			checkVersion(0);
		}

	protected:
		const int size;
};

#endif
