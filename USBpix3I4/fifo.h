#ifndef _FIFO_H
#define _FIFO_H

#include "device.h"

class Fifo : public FwDevice {
	public:
		Fifo(int _base, int _fifo_base, USB &_usb) : FwDevice(_base, _usb), fifo_base(_fifo_base) {
			registers = decltype(registers){ // workaround for MSVC12
				{"RESET", {0, 8, 0, Register::WRITE}},
				{"VERSION", {0, 8, 0, Register::READ}},
				{"ALMOST_FULL_THRESHOLD", {1, 8, 0, Register::READ_WRITE}},
				{"ALMOST_EMPTY_THRESHOLD", {2, 8, 0, Register::READ_WRITE}},
				{"READ_ERROR_COUNTER", {3, 8, 0, Register::READ}},
				{"FIFO_SIZE", {4, 32, 0, Register::READ}}
			};

			checkVersion(2);
		}

		int fifoSize(void);
		std::vector<uint32_t> getData(void);

	protected:
		const int fifo_base;
};

inline int Fifo::fifoSize(void) {
	return (*this)["FIFO_SIZE"] / 4;
}

inline std::vector<uint32_t> Fifo::getData(void) {
	int size = fifoSize();
	std::vector<uint32_t> result;

	if(size > 0) {
		auto data = usb.read(fifo_base, size * 4);
		result.resize(size);

		for(int i = 0; i < size; i++) {
			result[i] = data[i * 4] | (data[i * 4 + 1] << 8) | (data[i * 4 + 2] << 16) | (data[i * 4 + 3] << 24);
		}
	}

	return result;
}

#endif
