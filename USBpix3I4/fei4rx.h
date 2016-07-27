#ifndef _FEI4RX_H
#define _FEI4RX_H

#include "device.h"

class FEI4rx : public FwDevice {
	public:
		FEI4rx(int _base, USB &_usb) : FwDevice(_base, _usb) {
			registers = decltype(registers){ // workaround for MSVC12
				{"RESET", {0, 8, 0, Register::WRITE}},
				{"RX_RESET", {1, 8, 0, Register::WRITE}},
				{"VERSION", {0, 8, 0, Register::READ}},
				{"READY", {2, 1, 0, Register::READ}},
				{"INVERT_RX", {2, 1, 1, Register::READ_WRITE}},
				{"FIFO_SIZE", {3, 16, 0, Register::READ}},
				{"DECODER_ERROR_COUNTER", {5, 8, 0, Register::READ}},
				{"LOST_DATA_COUNTER", {6, 8, 0, Register::READ}}
			};

			checkVersion(2);
		}

		void rxReset(void);
};

inline void FEI4rx::rxReset(void) {
	(*this)["RX_RESET"] = 0;
}

#endif
