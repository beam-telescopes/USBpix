#ifndef _COMMANDBUFFER_H
#define _COMMANDBUFFER_H

#include <vector>
#include <list>
#include <cstdint>

#include "dllexport.h"

struct DllExport Command {
	int size;
	std::vector<uint8_t> cmd;

	Command operator+(const Command &command) const;
};

class DllExport CommandBuffer {
	public:
		CommandBuffer(void) {}
		CommandBuffer(CommandBuffer &&buf);

		std::vector<uint8_t> getBuffer(unsigned int size = 0);
		void clear(void);
		bool empty(void);

		void operator+=(const Command &command);
		void operator+=(const CommandBuffer &buf);

	protected:
		std::list<Command> commands;
};

#endif
