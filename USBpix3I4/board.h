#ifndef _BOARD_H
#define _BOARD_H

#include <vector>
#include <array>

#include "cmdseq.h"
#include "fifo.h"
#include "fei4rx.h"
#include "tlu.h"
#include "gpio.h"
#include "commandbuffer.h"

#include "dllexport.h"

class SiUSBDevice;

class DllExport Board {
	public:
                Board(SiUSBDevice *dev, int type=10);
		~Board(void);

		void init(int output_mode = CmdSeq::MANCHESTER_THOMAS);
		void wait(void);
		void wait(int size, int repeat);
		void sendCommands(CommandBuffer &buffer);
		void sendCommands(CommandBuffer &buffer, int repeat);
		void sendCommands(const Command &c, int repeat = 1);
		void setCommand(const Command &c);

		void enableTrigger(int trigger_mode);
		void disableTrigger(void);
		uint32_t getTriggerCounter(void);

		void printRxStatus(void);

		std::vector<uint32_t> getData(void);

	protected:
		USB u;

		CmdSeq cmd = CmdSeq(0x0000, u);
		Fifo fifo = Fifo(0x8100, 0x80000000, u);
		TLU tlu = TLU(0x8200, u);
		GPIO gpio_power = GPIO(0x8900, u);
		//GPIO gpio_disable = GPIO(0x8A00, u);

		std::vector<FEI4rx> rx;
};

#endif
