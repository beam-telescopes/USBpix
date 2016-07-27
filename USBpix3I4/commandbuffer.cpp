#include "commandbuffer.h"

Command Command::operator+(const Command &command) const {
	Command c;

	c.size = size + command.size;
	c.cmd = cmd;

	std::size_t num_bytes = c.size / 8;
	if(c.size % 8) {
		num_bytes++;
	}

	c.cmd.resize(num_bytes);

	int index = size / 8;
	int offset = size % 8;

	c.cmd[index] |= command.cmd[0] >> offset;

	for(std::size_t i = 0; i < command.cmd.size() - 1; i++) {
		c.cmd[index + i + 1] = (command.cmd[i] << (8 - offset)) | (command.cmd[i + 1] >> offset);
	}

	if(command.cmd.size() + cmd.size() == num_bytes && offset != 0) { //fixme?
		c.cmd[index + command.cmd.size()] = command.cmd.back() << (8 - offset);
	}

	return c;
}

CommandBuffer::CommandBuffer(CommandBuffer &&buf) {
	commands = buf.commands;
}

std::vector<uint8_t> CommandBuffer::getBuffer(unsigned int size) {
	std::vector<uint8_t> res;

	while(!commands.empty() && (res.size() + commands.front().cmd.size() <= size || size == 0)) {
		const auto &i = commands.front();
		res.insert(res.end(), i.cmd.begin(), i.cmd.end());
		commands.pop_front();
	}

	return res;
}

void CommandBuffer::clear(void) {
	commands.clear();
}

bool CommandBuffer::empty(void) {
	return commands.empty();
}

void CommandBuffer::operator+=(const Command &cmd) {
	commands.emplace_back(cmd);
}

void CommandBuffer::operator+=(const CommandBuffer &buf) {
	commands.insert(commands.end(), buf.commands.begin(), buf.commands.end());
}
