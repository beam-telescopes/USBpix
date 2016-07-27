#ifndef _INTERFACE_H
#define _INTERFACE_H

#include <vector>
#include <array>
#include <cstdint>

class SiUSBDevice;

class USB {
	public:
		USB(SiUSBDevice *dev_in){dev=dev_in;};
		~USB(void){};

		std::vector<uint8_t> read(int addr, int length);
		void write(int addr, const std::vector<uint8_t> &data);

	protected:
		SiUSBDevice *dev;
};

#endif
