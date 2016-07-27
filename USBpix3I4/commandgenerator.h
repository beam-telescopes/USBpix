#ifndef _COMMANDGENERATOR_H
#define _COMMANDGENERATOR_H

#include "commandbuffer.h"

class DllExport CommandGenerator {
	public:
		CommandGenerator(int _address) : address(_address) {}

		Command rdRegister(int reg);
		Command wrRegister(int reg, uint16_t val);
		Command wrFrontEnd(const std::vector<uint8_t> &reg);
		Command globalReset(void);
		Command globalPulse(int width);
		Command runMode(void);
		Command confMode(void);

		static Command lv1(void);
		static Command bcr(void);
		static Command ecr(void);
		static Command cal(void);

		static Command zeros(int length);

	protected:
		int address;
};

#endif
