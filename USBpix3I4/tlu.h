#ifndef _TLU_H
#define _TLU_H

#include "device.h"

class TLU : public FwDevice {
	public:
		enum {
			EXTERNAL = 0,
			NO_HANDSHAKE = 1,
			SIMPLE_HANDSHAKE = 2,
			DATA_HANDSHAKE = 3
		};

		TLU(int _base, USB &_usb) : FwDevice(_base, _usb) {
			registers = decltype(registers){ // workaround for MSVC12
				{"RESET", {0, 8, 0, Register::WRITE}},
				{"VERSION", {0, 8, 0, Register::READ}},
				{"TRIGGER_MODE", {1, 2, 0, Register::READ_WRITE}},
				{"TRIGGER_DATA_MSB_FIRST", {1, 1, 2, Register::READ_WRITE}},
				{"TRIGGER_DATA_DELAY", {1, 4, 4, Register::READ_WRITE}},
				{"TRIGGER_CLOCK_CYCLES", {2, 5, 0, Register::READ_WRITE}},
				{"EN_TLU_RESET", {2, 1, 5, Register::READ_WRITE}},
				{"EN_TLU_VETO", {2, 1, 6, Register::READ_WRITE}},
				{"EN_WRITE_TIMESTAMP", {2, 1, 7, Register::READ_WRITE}},
				{"TRIGGER_LOW_TIMEOUT", {3, 8, 0, Register::READ_WRITE}},
				{"CURRENT_TLU_TRIGGER_NUMBER", {4, 32, 0, Register::READ}},
				{"TRIGGER_COUNTER", {8, 32, 0, Register::READ_WRITE}},
				{"LOST_DATA_COUNTER", {12, 8, 0, Register::READ}},
				{"TRIGGER_SELECT", {13, 8, 0, Register::READ_WRITE}},
				{"TRIGGER_VETO_SELECT", {14, 8, 0, Register::READ_WRITE}},
				{"TRIGGER_INVERT", {15, 8, 0, Register::READ_WRITE}}
			};

			checkVersion(1);
		}
};

#endif
