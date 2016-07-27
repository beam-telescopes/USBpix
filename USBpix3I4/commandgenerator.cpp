#include "commandgenerator.h"

const uint16_t slow = 0x168; // 0b101101000;

const uint8_t rd_register = 0x1;  // 0b0001;
const uint8_t wr_register = 0x2;  // 0b0010;
const uint8_t wr_frontend = 0x4;  // 0b0100;
const uint8_t global_reset = 0x8; // 0b1000;
const uint8_t global_pulse = 0x9; // 0b1001;
const uint8_t run_mode = 0xA; // 0b1010;

const uint8_t r_conf_mode = 0x07; //0b000111;
const uint8_t r_run_mode = 0x38; //0b111000;

void assemble(uint8_t *buffer, uint16_t f1, uint16_t f2, uint16_t f3, uint16_t f4) {
	uint32_t val = (f1 << (24 - 9)) | (f2 << (24 - 4 - 9)) | (f3 << (24 - 4 - 4 - 9)) | (f4 << (24 - 6 - 4 - 4 - 9));

	buffer[0] = (val >> 16) & 0xFF;
	buffer[1] = (val >> 8) & 0xFF;
	buffer[2] = (val >> 0) & 0xFF;
}

void assemble(uint8_t *buffer, uint64_t f1, uint64_t f2, uint64_t f3, uint64_t f4, uint64_t f5) {
	uint64_t val = (f1 << (40 - 9)) | (f2 << (40 - 4 - 9)) | (f3 << (40 - 4 - 4 - 9)) | (f4 << (40 - 6 - 4 - 4 - 9)) | (f5 << (40 - 16 - 6 - 4 - 4 - 9));

	buffer[0] = (val >> 32) & 0xFF;
	buffer[1] = (val >> 24) & 0xFF;
	buffer[2] = (val >> 16) & 0xFF;
	buffer[3] = (val >> 8) & 0xFF;
	buffer[4] = (val >> 0) & 0xFF;
}

Command CommandGenerator::rdRegister(int reg) {
	Command cmd;
	cmd.cmd.resize(3);
	cmd.size = 24;

	assemble(&cmd.cmd[0], slow, rd_register, address, reg);
	return cmd;
}

Command CommandGenerator::wrRegister(int reg, uint16_t val) {
	Command cmd;
	cmd.cmd.resize(5);
	cmd.size = 40;

	assemble(&cmd.cmd[0], slow, wr_register, address, reg, val);
	return cmd;
}

Command CommandGenerator::wrFrontEnd(const std::vector<uint8_t> &reg) {
	Command cmd;
	cmd.cmd.resize(87);
	cmd.size = 696;

	assemble(&cmd.cmd[0], slow, wr_frontend, address, 0);

	cmd.cmd[2] |= reg[0] >> 7;

	for(int i = 0; i < 83; i++) {
		cmd.cmd[i + 3] = (reg[i] << 1) | (reg[i + 1] >> 7);
	}

	cmd.cmd[83 + 3] = reg[83] << 1;

	return cmd;
}

Command CommandGenerator::globalReset(void) {
	Command cmd;
	cmd.cmd.resize(3);
	cmd.size = 18;

	assemble(&cmd.cmd[0], slow, global_reset, address, 0);
	return cmd;
}

Command CommandGenerator::globalPulse(int width) {
	Command cmd;
	cmd.cmd.resize(3);
	cmd.size = 24;

	assemble(&cmd.cmd[0], slow, global_pulse, address, width);
	return cmd;
}

Command CommandGenerator::runMode(void) {
	Command cmd;
	cmd.cmd.resize(3);
	cmd.size = 24;

	assemble(&cmd.cmd[0], slow, run_mode, address, r_run_mode);
	return cmd;
}

Command CommandGenerator::confMode(void) {
	Command cmd;
	cmd.cmd.resize(3);
	cmd.size = 24;

	assemble(&cmd.cmd[0], slow, run_mode, address, r_conf_mode);
	return cmd;
}

Command CommandGenerator::lv1(void) {
	return {6, {0xE8}}; //0b11101000
}

Command CommandGenerator::bcr(void) {
	return {10, {0xB0, 0x80}}; //0b10110000 0b10000000
}

Command CommandGenerator::ecr(void) {
	return {10, {0xB1, 0x00}}; //0b10110001 0b00000000
}

Command CommandGenerator::cal(void) {
	return {10, {0xB2, 0x00}}; //0b10110010 0b00000000
}

Command CommandGenerator::zeros(int length) {
	Command cmd;
	cmd.size = length;

	int l = length / 8;

	if(length % 8) {
		l++;
	}

	cmd.cmd.resize(l);

	return cmd;
}
