#ifndef _CMDSEQ_H
#define _CMDSEQ_H

#include "device.h"

class CmdSeq : public FwDevice {
	public:
		enum {
			POSITIVE_EDGE = 0,
			NEGATIVE_EDGE = 1,
			MANCHESTER_IEEE = 2,
			MANCHESTER_THOMAS = 3
		};

		CmdSeq(int _base, USB &_usb) : FwDevice(_base, _usb) {
			registers = decltype(registers){ // workaround for MSVC12
				{"RESET", {0, 8, 0, Register::WRITE}},
				{"VERSION", {0, 8, 0, Register::READ}},
				{"START", {1, 8, 0, Register::WRITE}},
				{"READY", {1, 1, 0, Register::READ}},
				{"EN_EXT_TRIGGER", {2, 1, 0, Register::READ_WRITE}},
				{"OUTPUT_MODE", {2, 2, 1, Register::READ_WRITE}},
				{"CLOCK_GATE", {2, 1, 3, Register::READ_WRITE}},
				{"CMD_PULSE", {2, 1, 4, Register::READ_WRITE}},
				{"CMD_SIZE", {3, 16, 0, Register::READ_WRITE}},
				{"CMD_REPEAT", {5, 32, 0, Register::READ_WRITE}},
				{"START_SEQUENCE_LENGTH", {9, 16, 0, Register::READ_WRITE}},
				{"STOP_SEQUENCE_LENGTH", {11, 16, 0, Register::READ_WRITE}}
			};

			checkVersion(0);
		}

		std::vector<uint8_t> getData(int size = mem_size, int addr = 0);
		void setData(const std::vector<uint8_t> &data, int addr = 0);
		void start(void);

		static const int mem_offset = 16;
		static const int mem_size = 2048 - mem_offset;
};


inline void CmdSeq::start(void) {
	(*this)["START"] = 0;
}

inline std::vector<uint8_t> CmdSeq::getData(int size, int addr) {
	if(size + addr > mem_size) {
		throw std::runtime_error("CmdSeq: Request too large");
	}

	return usb.read(base + mem_offset + addr, size);
}

inline void CmdSeq::setData(const std::vector<uint8_t> &data, int addr) {
	if(data.size() + addr > mem_size) {
		throw std::runtime_error("CmdSeq: Request too large");
	}

	usb.write(base + mem_offset + addr, data);
}

#endif
