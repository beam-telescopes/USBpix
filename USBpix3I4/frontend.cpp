
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iterator>

#include <iostream>

#include "frontend.h"

#define UFDEBUG false

using namespace std;

RegisterProxy Frontend::operator[](const std::string &name) {
	return RegisterProxy(std::bind(&Frontend::readRegister, this, registers.at(name)), std::bind(&Frontend::writeRegister, this, registers.at(name), std::placeholders::_1));
}

RegisterProxy Frontend::operator[](int reg) {
	return RegisterProxy(std::bind(&Frontend::readRegisterNr, this, reg), std::bind(&Frontend::writeRegisterNr, this, reg, std::placeholders::_1));
}

uint64_t Frontend::readCompoundRegister(const FERegister &reg) const {
	uint64_t value = 0;

	for(int i = 0; i * 16 < reg.offset + reg.size; i++) {
		value |= global_registers[reg.addr + i] << (i * 16);
	}

	return value;
}

void Frontend::writeCompoundRegister(const FERegister &reg, uint64_t val) {
	for(int i = 0; i * 16 < reg.offset + reg.size; i++) {
		global_registers[reg.addr + i] = selectBits(val, i * 16, 16);
		written_registers.push_back(reg.addr + i);
	}
}

uint64_t Frontend::readRegister(const FERegister &reg) const {
	uint64_t val = selectBits(readCompoundRegister(reg), reg.offset, reg.size);

	if(reg.type & FERegister::MSB_LAST) {
		val = reverseBits(val, reg.size);
	}

	return val;
}

void Frontend::writeRegister(const FERegister &reg, uint64_t val) {
	//if(!(reg.type & FERegister::WRITE)) {
	//	throw std::runtime_error("Write access to read-only register");
	//}

	val = selectBits(val, 0, reg.size);

	if(reg.type & FERegister::MSB_LAST) {
		val = reverseBits(val, reg.size);
	}

	uint64_t value = readCompoundRegister(reg);
	value = clearBits(value, reg.offset, reg.size);
	value |= val << reg.offset;

	writeCompoundRegister(reg, value);
}

uint64_t Frontend::readRegisterNr(int reg) const {
	return global_registers.at(reg);
}

void Frontend::writeRegisterNr(int reg, uint64_t val) {
	global_registers.at(reg) = val;
	written_registers.push_back(reg);
}

void Frontend::loadRegisterList(void) {
	registers.clear();

	if(flavour == FEI4A) {
		registers = decltype(registers){
			//{"spare_1", {1, 16, 0, FERegister::READ}},
			{"Reg2Spare", {2, 11, 0, FERegister::READ}},
			{"Conf_AddrEnable", {2, 1, 11, FERegister::READ_WRITE}},
			{"TrigCnt", {2, 4, 12, FERegister::READ_WRITE}},
			{"ErrMask0", {3, 16, 0, FERegister::READ_WRITE}}, //checkme
			{"ErrMask1", {4, 16, 0, FERegister::READ_WRITE}},
			{"Vthin", {5, 8, 0, FERegister::READ_WRITE_MSB}},
			{"PrmpVbpRight", {5, 8, 8, FERegister::READ_WRITE_MSB}},
			{"PrmpVbp", {6, 8, 0, FERegister::READ_WRITE_MSB}},
			{"DisVbn_CPPM", {6, 8, 8, FERegister::READ_WRITE_MSB}},
			{"DisVbn", {7, 8, 0, FERegister::READ_WRITE_MSB}},
			{"TdacVbp", {7, 8, 8, FERegister::READ_WRITE_MSB}},
			{"Amp2VbpFol", {8, 8, 0, FERegister::READ_WRITE_MSB}},
			{"Amp2Vbn", {8, 8, 8, FERegister::READ_WRITE_MSB}},
			{"Amp2Vbp", {9, 8, 0, FERegister::READ_WRITE_MSB}},
			{"PrmpVbpTop", {9, 8, 8, FERegister::READ_WRITE_MSB}},
			{"Amp2Vbpf", {10, 8, 0, FERegister::READ_WRITE_MSB}},
			{"FdacVbn", {10, 8, 8, FERegister::READ_WRITE_MSB}},
			{"PrmpVbpLeft", {11, 8, 0, FERegister::READ_WRITE_MSB}},
			{"PrmpVbnFol", {11, 8, 8, FERegister::READ_WRITE_MSB}},
			{"PrmpVbnLcc", {12, 8, 0, FERegister::READ_WRITE_MSB}},
			{"PrmpVbpf", {12, 8, 8, FERegister::READ_WRITE_MSB}},
			{"Reg13Spare", {13, 1, 0, FERegister::READ}},
			{"PxStrobes", {13, 13, 1, FERegister::READ_WRITE_MSB}},
			{"S0", {13, 1, 14, FERegister::READ_WRITE}},
			{"S1", {13, 1, 15, FERegister::READ_WRITE}},
			{"BonnDac", {14, 8, 0, FERegister::READ_WRITE}},
			{"LVDSDrvIref", {14, 8, 8, FERegister::READ_WRITE}},
			{"LVDSDrvVos", {15, 8, 0, FERegister::READ_WRITE}},
			{"PllIbias", {15, 8, 8, FERegister::READ_WRITE_MSB}},
			{"PllIcp", {16, 8, 0, FERegister::READ_WRITE_MSB}},
			{"TempSensBias", {16, 8, 8, FERegister::READ_WRITE_MSB}},
			{"PlsrIdacRamp", {17, 8, 0, FERegister::READ_WRITE_MSB}},
			{"Reg17Spare", {17, 8, 8, FERegister::READ}},
			{"PlsrVgOPamp", {18, 8, 0, FERegister::READ_WRITE_MSB}},
			{"Reg18Spare", {18, 8, 8, FERegister::READ}},
			{"Reg19Spare", {19, 8, 0, FERegister::READ}},
			{"PlsrDacBias", {19, 8, 8, FERegister::READ_WRITE_MSB}},
			{"Vthin_AltFine", {20, 8, 0, FERegister::READ_WRITE_MSB}},
			{"Vthin_AltCoarse", {20, 8, 8, FERegister::READ_WRITE_MSB}},
			{"PlsrDAC", {21, 10, 0, FERegister::READ_WRITE_MSB}},
			{"DIGHITIN_Sel", {21, 1, 10, FERegister::READ_WRITE}},
			{"DINJ_Override", {21, 1, 11, FERegister::READ_WRITE}},
			{"HITLD_In", {21, 1, 12, FERegister::READ_WRITE}},
			{"Reg21Spare", {21, 3, 13, FERegister::READ}},
			{"Reg22Spare2", {22, 2, 0, FERegister::READ}},
			{"Colpr_Addr", {22, 6, 2, FERegister::READ_WRITE_MSB}},
			{"Colpr_Mode", {22, 2, 8, FERegister::READ_WRITE_MSB}},
			{"Reg22Spare1", {22, 6, 10, FERegister::READ}},
			{"DisableColumnCnfg0", {23, 16, 0, FERegister::READ_WRITE_MSB}}, //checkme
			{"DisableColumnCnfg1", {24, 16, 0, FERegister::READ_WRITE_MSB}},
			{"DisableColumnCnfg2", {25, 8, 0, FERegister::READ_WRITE_MSB}},
			{"TrigLat", {25, 8, 8, FERegister::READ_WRITE}},
			{"HitDiscCnfg", {26, 2, 0, FERegister::READ_WRITE}},
			{"StopModeCnfg", {26, 1, 2, FERegister::READ_WRITE}},
			{"CMDcnt", {26, 14, 3, FERegister::READ_WRITE}},
			{"SR_Clock", {27, 1, 1, FERegister::READ_WRITE}},
			{"Latch_en", {27, 1, 2, FERegister::READ_WRITE}},
			{"SR_clr", {27, 1, 3, FERegister::READ_WRITE}},
			{"CalEn", {27, 1, 4, FERegister::READ_WRITE}},
			{"GateHitOr", {27, 1, 5, FERegister::READ_WRITE}},
			{"Reg27Spare", {27, 5, 6, FERegister::READ}},
			{"ReadSkipped", {27, 1, 11, FERegister::READ_WRITE}},
			{"ReadErrorReq", {27, 1, 12, FERegister::READ_WRITE}},
			{"Stop_Clk", {27, 1, 13, FERegister::READ_WRITE}},
			{"Efuse_sense", {27, 1, 14, FERegister::READ_WRITE}},
			{"EN_PLL", {27, 1, 15, FERegister::READ_WRITE}},
			{"EN320M", {28, 1, 0, FERegister::READ_WRITE}},
			{"EN160M", {28, 1, 1, FERegister::READ_WRITE}},
			{"CLK0", {28, 3, 2, FERegister::READ_WRITE_MSB}}, //checkme
			{"CLK1", {28, 3, 5, FERegister::READ_WRITE_MSB}},
			{"EN80M", {28, 1, 8, FERegister::READ_WRITE}},
			{"EN40M", {28, 1, 9, FERegister::READ_WRITE}},
			{"Reg28Spare", {28, 5, 10, FERegister::READ}},
			{"LVDSDrvSet06", {28, 1, 15, FERegister::READ_WRITE}},
			{"LVDSDrvSet12", {29, 1, 0, FERegister::READ_WRITE}},
			{"LVDSDrvSet30", {29, 1, 1, FERegister::READ_WRITE}},
			{"LVDSDrvEn", {29, 1, 2, FERegister::READ_WRITE}},
			{"Reg29Spare2", {29, 1, 3, FERegister::READ}},
			{"EmptyRecord", {29, 8, 4, FERegister::READ_WRITE}},
			{"Clk2OutCnfg", {29, 1, 12, FERegister::READ_WRITE}},
			{"no8b10b", {29, 1, 13, FERegister::READ_WRITE}},
			{"Reg29Spare1", {29, 2, 14, FERegister::READ}},
			//{"Reg30Spare", {30, 16, 0, FERegister::READ}},
			{"Reg31Spare", {31, 4, 0, FERegister::READ}},
			{"ExtAnaCalSW", {31, 1, 4, FERegister::READ_WRITE}},
			{"ExtDigCalSW", {31, 1, 5, FERegister::READ_WRITE}},
			{"PlsrDelay", {31, 6, 6, FERegister::READ_WRITE_MSB}},
			{"PlsrPwr", {31, 1, 12, FERegister::READ_WRITE}},
			{"PlsrRiseUpTau", {31, 3, 13, FERegister::READ_WRITE}},
			{"SELB0", {32, 16, 0, FERegister::READ_WRITE}}, //checkme
			{"SELB1", {33, 16, 0, FERegister::READ_WRITE}},
			{"SELB2", {34, 8, 8, FERegister::READ_WRITE}},
			//{"spare_34", {34, 4, 8, FERegister::READ}},
			{"EfuseCref", {34, 4, 0, FERegister::READ_WRITE_MSB}},
			{"Chip_SN", {35, 16, 0, FERegister::READ_WRITE}}
		};
	} else {
		registers = decltype(registers){
			//{"spare_0", {0, 16, 0, FERegister::READ}},
			{"Eventlimit", {1, 8, 0, FERegister::READ_WRITE_MSB}},
			{"SmallHitErase", {1, 1, 8, FERegister::READ_WRITE}},
			//{"spare_1", {1, 7, 9, FERegister::READ}},
			{"Reg2Spare", {2, 11, 0, FERegister::READ}},
			{"Conf_AddrEnable", {2, 1, 11, FERegister::READ_WRITE}},
			{"TrigCnt", {2, 4, 12, FERegister::READ_WRITE}},
			{"ErrMask0", {3, 16, 0, FERegister::READ_WRITE}}, //checkme
			{"ErrMask1", {4, 16, 0, FERegister::READ_WRITE}},
			{"BufVgOpAmp", {5, 8, 0, FERegister::READ_WRITE_MSB}},
			{"PrmpVbpRight", {5, 8, 8, FERegister::READ_WRITE_MSB}},
			{"PrmpVbp", {6, 8, 0, FERegister::READ_WRITE_MSB}},
			{"Reg6Spare", {6, 8, 8, FERegister::READ}},
			{"DisVbn", {7, 8, 0, FERegister::READ_WRITE_MSB}},
			{"TdacVbp", {7, 8, 8, FERegister::READ_WRITE_MSB}},
			{"Amp2VbpFol", {8, 8, 0, FERegister::READ_WRITE_MSB}},
			{"Amp2Vbn", {8, 8, 8, FERegister::READ_WRITE_MSB}},
			{"Amp2Vbp", {9, 8, 0, FERegister::READ_WRITE_MSB}},
			{"Reg9Spare", {9, 8, 8, FERegister::READ}},
			{"Amp2Vbpf", {10, 8, 0, FERegister::READ_WRITE_MSB}},
			{"FdacVbn", {10, 8, 8, FERegister::READ_WRITE_MSB}},
			{"PrmpVbpLeft", {11, 8, 0, FERegister::READ_WRITE_MSB}},
			{"PrmpVbnFol", {11, 8, 8, FERegister::READ_WRITE_MSB}},
			{"PrmpVbnLcc", {12, 8, 0, FERegister::READ_WRITE_MSB}},
			{"PrmpVbpf", {12, 8, 8, FERegister::READ_WRITE_MSB}},
			{"Reg13Spare", {13, 1, 0, FERegister::READ}},
			{"PxStrobes", {13, 13, 1, FERegister::READ_WRITE_MSB}},
			{"S0", {13, 1, 14, FERegister::READ_WRITE}},
			{"S1", {13, 1, 15, FERegister::READ_WRITE}},
			{"GADCOpAmp", {14, 8, 0, FERegister::READ_WRITE_MSB}},
			{"LVDSDrvIref", {14, 8, 8, FERegister::READ_WRITE_MSB}},
			{"LVDSDrvVos", {15, 8, 0, FERegister::READ_WRITE_MSB}},
			{"PllIbias", {15, 8, 8, FERegister::READ_WRITE_MSB}},
			{"PllIcp", {16, 8, 0, FERegister::READ_WRITE_MSB}},
			{"TempSensBias", {16, 8, 8, FERegister::READ_WRITE_MSB}},
			{"PlsrIdacRamp", {17, 8, 0, FERegister::READ_WRITE_MSB}},
			{"Reg17Spare", {17, 8, 8, FERegister::READ}},
			{"PlsrVgOPamp", {18, 8, 0, FERegister::READ_WRITE_MSB}},
			{"VrefDigTune", {18, 8, 8, FERegister::READ_WRITE_MSB}},
			{"VrefAnTune", {19, 8, 0, FERegister::READ_WRITE_MSB}},
			{"PlsrDacBias", {19, 8, 8, FERegister::READ_WRITE_MSB}},
			{"Vthin_AltFine", {20, 8, 0, FERegister::READ_WRITE_MSB}},
			{"Vthin_AltCoarse", {20, 8, 8, FERegister::READ_WRITE_MSB}},
			{"PlsrDAC", {21, 10, 0, FERegister::READ_WRITE_MSB}},
			{"DIGHITIN_Sel", {21, 1, 10, FERegister::READ_WRITE}},
			{"DINJ_Override", {21, 1, 11, FERegister::READ_WRITE}},
			{"HITLD_In", {21, 1, 12, FERegister::READ_WRITE}},
			{"Reg21Spare", {21, 3, 13, FERegister::READ}},
			{"Reg22Spare2", {22, 2, 0, FERegister::READ}},
			{"Colpr_Addr", {22, 6, 2, FERegister::READ_WRITE_MSB}},
			{"Colpr_Mode", {22, 2, 8, FERegister::READ_WRITE_MSB}},
			{"Reg22Spare1", {22, 6, 10, FERegister::READ}},
			{"DisableColumnCnfg0", {23, 16, 0, FERegister::READ_WRITE_MSB}}, //checkme
			{"DisableColumnCnfg1", {24, 16, 0, FERegister::READ_WRITE_MSB}},
			{"DisableColumnCnfg2", {25, 8, 0, FERegister::READ_WRITE_MSB}},
			{"TrigLat", {25, 8, 8, FERegister::READ_WRITE}},
			{"HitDiscCnfg", {26, 2, 0, FERegister::READ_WRITE}},
			{"StopModeCnfg", {26, 1, 2, FERegister::READ_WRITE}},
			{"CMDcnt", {26, 14, 3, FERegister::READ_WRITE}},
			{"SR_Clock", {27, 1, 1, FERegister::READ_WRITE}},
			{"Latch_en", {27, 1, 2, FERegister::READ_WRITE}},
			{"SR_clr", {27, 1, 3, FERegister::READ_WRITE}},
			{"CalEn", {27, 1, 4, FERegister::READ_WRITE}},
			{"GateHitOr", {27, 1, 5, FERegister::READ_WRITE}},
			{"Reg27Spare2", {27, 3, 6, FERegister::READ}},
			{"ShiftReadBack", {27, 1, 9, FERegister::READ_WRITE}},
			{"GADC_Enable", {27, 1, 10, FERegister::READ_WRITE}},
			{"Reg27Spare1", {27, 1, 11, FERegister::READ}},
			{"ReadErrorReq", {27, 1, 12, FERegister::READ_WRITE}},
			{"Stop_Clk", {27, 1, 13, FERegister::READ_WRITE}},
			{"Efuse_sense", {27, 1, 14, FERegister::READ_WRITE}},
			{"EN_PLL", {27, 1, 15, FERegister::READ_WRITE}},
			{"EN320M", {28, 1, 0, FERegister::READ_WRITE}},
			{"EN160M", {28, 1, 1, FERegister::READ_WRITE}},
			{"CLK0", {28, 3, 2, FERegister::READ_WRITE_MSB}}, //checkme
			{"CLK1", {28, 3, 5, FERegister::READ_WRITE_MSB}},
			{"EN80M", {28, 1, 8, FERegister::READ_WRITE}},
			{"EN40M", {28, 1, 9, FERegister::READ_WRITE}},
			{"Reg28Spare", {28, 5, 10, FERegister::READ}},
			{"LVDSDrvSet06", {28, 1, 15, FERegister::READ_WRITE}},
			{"LVDSDrvSet12", {29, 1, 0, FERegister::READ_WRITE}},
			{"LVDSDrvSet30", {29, 1, 1, FERegister::READ_WRITE}},
			{"LVDSDrvEn", {29, 1, 2, FERegister::READ_WRITE}},
			{"Reg29Spare2", {29, 1, 3, FERegister::READ}},
			{"EmptyRecord", {29, 8, 4, FERegister::READ_WRITE}},
			{"Clk2OutCnfg", {29, 1, 12, FERegister::READ_WRITE}},
			{"no8b10b", {29, 1, 13, FERegister::READ_WRITE}},
			{"Reg29Spare1", {29, 2, 14, FERegister::READ}},
			{"Reg30Spare", {30, 12, 0, FERegister::READ}},
			{"IleakRange", {30, 1, 12, FERegister::READ_WRITE}},
			{"TempSensDisable", {30, 1, 13, FERegister::READ_WRITE}},
			{"TempSensDiodeSel", {30, 2, 14, FERegister::READ_WRITE_MSB}},
			{"GADCSel", {31, 3, 0, FERegister::READ_WRITE}},
			{"Reg31Spare", {31, 1, 3, FERegister::READ}},
			{"ExtAnaCalSW", {31, 1, 4, FERegister::READ_WRITE}},
			{"ExtDigCalSW", {31, 1, 5, FERegister::READ_WRITE}},
			{"PlsrDelay", {31, 6, 6, FERegister::READ_WRITE_MSB}},
			{"PlsrPwr", {31, 1, 12, FERegister::READ_WRITE}},
			{"PlsrRiseUpTau", {31, 3, 13, FERegister::READ_WRITE}},
			{"SELB0", {32, 16, 0, FERegister::READ_WRITE}}, //checkme
			{"SELB1", {33, 16, 0, FERegister::READ_WRITE}},
			{"SELB2", {34, 8, 8, FERegister::READ_WRITE}},
			{"Reg34Spare1", {34, 3, 5, FERegister::READ}},
			{"PrmpVbpMsnEn", {34, 1, 4, FERegister::READ_WRITE}},
			{"Reg34Spare2", {34, 4, 0, FERegister::READ}},
			{"Chip_SN", {35, 16, 0, FERegister::READ_WRITE}}
		};
	}

	pix_registers.clear();

	pix_registers = decltype(pix_registers){
		{"ENABLE", {0, 1, 1 << 0, PixRegister::LATCH}},
		{"TDAC", {1, 5, 1 << 1, PixRegister::LATCH_MSB}},
		{"CAP0", {2, 1, 1 << 6, PixRegister::LATCH}},
		{"CAP1", {3, 1, 1 << 7, PixRegister::LATCH}},
		{"ILEAK", {4, 1, 1 << 8, PixRegister::LATCH}},
		{"FDAC", {5, 4, 1 << 9, PixRegister::LATCH}},
		{"EnableDigInj", {6, 1, 0, PixRegister::LATCH}}
	};

	for(auto &i : registers) {
		register_list.push_back(i.first);
	}

	if(UFDEBUG) cout << "address " << address << endl;
}

std::vector<int> Frontend::uniqueRegisterList(const std::vector<std::string> &regs) {
	std::vector<int> r;

	for(auto const &i : regs) {
		const FERegister &j = registers.at(i);

		for(int k = 0; k * 16 < j.size; k++) {
			r.push_back(j.addr + k);
		}
	}

	std::sort(r.begin(), r.end());
	auto last = std::unique(r.begin(), r.end());
	r.erase(last, r.end());

	return r;
}

CommandBuffer Frontend::readGlobalRegisters(const std::vector<std::string> &regs) {
	std::vector<int> r = uniqueRegisterList(regs);

	return readGlobalRegisters(r);
}

CommandBuffer Frontend::readGlobalRegisters(const std::vector<int> &regs) {
	CommandBuffer c;

	for(auto i : regs) {
		c += cmd.rdRegister(i);
	}

	return c;
}

CommandBuffer Frontend::readGlobalRegisters(void) {
	CommandBuffer c;

	for(int i=MIN_GLOBAL_REG; i<=MAX_GLOBAL_REG; i++) {
		c += cmd.rdRegister(i);
	}

	return c;
}

CommandBuffer Frontend::writeGlobalRegisters(const std::vector<std::string> &regs) {
	std::vector<int> r = uniqueRegisterList(regs);

	return writeGlobalRegisters(r);
}

CommandBuffer Frontend::writeGlobalRegisters(const std::vector<int> &regs) {
	CommandBuffer c;

	for(auto i : regs) {
		c += cmd.wrRegister(i, global_registers[i]);
	}

	return c;
}

CommandBuffer Frontend::writeGlobalRegisters(void) {
	CommandBuffer c;

	for(int i=MIN_GLOBAL_REG; i<=MAX_GLOBAL_REG; i++) {
		c += cmd.wrRegister(i, global_registers[i]);
	}

	return c;
}

CommandBuffer Frontend::flushWrites(void) {
	CommandBuffer c;

	if(UFDEBUG) cout << "flushWrites" << endl;

	std::sort(written_registers.begin(), written_registers.end());
	auto last = std::unique(written_registers.begin(), written_registers.end());
	written_registers.erase(last, written_registers.end());

	for(auto i : written_registers) {
		c += cmd.wrRegister(i, global_registers[i]);
		if(UFDEBUG) cout << dec << i << ' ' << hex << global_registers[i] << endl;
	}

	written_registers.clear();

	return c;
}

void Frontend::disableGlobalPulseRegisters(void) {
	(*this)["SR_clr"] = 0;
	(*this)["CalEn"] = 0;
	(*this)["ReadErrorReq"] = 0;
	(*this)["Stop_Clk"] = 0;
	(*this)["SR_Clock"] = 0;
	(*this)["Efuse_sense"] = 0;
	(*this)["Latch_en"] = 0;

	(*this)["DIGHITIN_Sel"] = 0;

	if(flavour == FEI4A) {
		(*this)["ReadSkipped"] = 0;
	} else {
		(*this)["ShiftReadBack"] = 0;
		(*this)["GADC_Enable"] = 0;
	}
}

void Frontend::setPixelRegister(const std::string &name, const std::vector<int> &dcs, uint8_t value) {
	const PixRegister &r = pix_registers.at(name);

	for(auto dc : dcs) {
		for(int i = 0; i < ROWS; i++) {
			pixel_registers[r.addr][i][2 * dc] = value;
			pixel_registers[r.addr][i][2 * dc + 1] = value;
		}
	}
}

void Frontend::setPixelRegister(const std::string &name, uint8_t value) {
	const PixRegister &r = pix_registers.at(name);

	for(int dc = 0; dc < 40; dc++) {
		for(int i = 0; i < ROWS; i++) {
			pixel_registers[r.addr][i][2 * dc] = value;
			pixel_registers[r.addr][i][2 * dc + 1] = value;
		}
	}
}

void Frontend::setPixelRegister(const std::string &name, int x, int y, uint8_t value) {
	const PixRegister &r = pix_registers.at(name);

	pixel_registers[r.addr][y][x] = value;
}

void Frontend::setPixelRegister(const std::string &name, const Matrix<uint8_t, ROWS, COLUMNS> &value) {
	const PixRegister &r = pix_registers.at(name);

	pixel_registers[r.addr] = value;
}

CommandBuffer Frontend::writePixelRegister(const std::string &name) {
	return writePixelRegister(name, {
		 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
		10,11,12,13,14,15,16,17,18,19,
		20,21,22,23,24,25,26,27,28,29,
		30,31,32,33,34,35,36,37,38,39,
	});
}

CommandBuffer Frontend::writePixelRegister(const std::string &name, const std::vector<int> &dcs, bool all_dcs) {
	const PixRegister &r = pix_registers.at(name);
	bool enable_latch = r.type & PixRegister::LATCH;

	std::array<uint16_t, 36> greg_copy;
	saveGlobalRegisters(greg_copy);

	CommandBuffer c;

	disableGlobalPulseRegisters();

	(*this)["S0"] = 0;
	(*this)["S1"] = 0;
	(*this)["HITLD_In"] = 0;

	(*this)["GateHitOr"] = 0;
	(*this)["Latch_en"] = enable_latch;

	(*this)["Colpr_Mode"] = all_dcs ? 3 : 0;

	(*this)["Vthin_AltFine"] = 255;
	(*this)["Vthin_AltCoarse"] = 255;

	c += flushWrites();

	for(int i = 0; i < r.size; i++) {
		(*this)["PxStrobes"] = r.px_strobes << i;

		c += flushWrites();

		for(auto dc : dcs) {
			(*this)["Colpr_Addr"] = dc;
			c += flushWrites();

			if(r.type & PixRegister::MSB) {
				c += cmd.wrFrontEnd(getDC(r, r.size - i - 1, dc));
			} else {
				c += cmd.wrFrontEnd(getDC(r, i, dc));
			}

			if(enable_latch) {
				c += cmd.globalPulse(0);
			}
		}
	}

	restoreGlobalRegisters(greg_copy);
	c += flushWrites();

	return c;
}

CommandBuffer Frontend::readPixelRegister(const std::string &name, int bit, const std::vector<int> &dcs) {
	const PixRegister &r = pix_registers.at(name);
	bool enable_latch = r.type & PixRegister::LATCH;

	std::array<uint16_t, 36> greg_copy;
	saveGlobalRegisters(greg_copy);

	CommandBuffer c;

	disableGlobalPulseRegisters();

	(*this)["S0"] = 0;
	(*this)["S1"] = 0;
	(*this)["HITLD_In"] = 0;

	(*this)["GateHitOr"] = 0;
	(*this)["Colpr_Mode"] = 0;
	(*this)["Colpr_Addr"] = 0;
	(*this)["PxStrobes"] = 0;

	c += flushWrites();

	if(r.type & PixRegister::MSB) { //FIXME
		bit = r.size - bit - 1;
	}

	(*this)["PxStrobes"] = r.px_strobes << bit;

	c += flushWrites();

	for(auto dc : dcs) {
		(*this)["Colpr_Addr"] = dc;
		c += flushWrites();

		if(enable_latch) {
			(*this)["S0"] = 1;
			(*this)["S1"] = 1;
			(*this)["SR_Clock"] = 1;
			c += flushWrites();
			c += cmd.globalPulse(0);
		}
		(*this)["S0"] = 0;
		(*this)["S1"] = 0;
		(*this)["SR_Clock"] = 0;
		c += flushWrites();

		if(flavour == FEI4B) {
			(*this)["ShiftReadBack"] = 1;
			c += flushWrites();
		}

		if(r.type & PixRegister::MSB) {
			c += cmd.wrFrontEnd(getDC(r, r.size - bit - 1, dc));
		} else {
			c += cmd.wrFrontEnd(getDC(r, bit, dc));
		}

		if(flavour == FEI4B) {
			(*this)["ShiftReadBack"] = 0;
			c += flushWrites();
		}
	}

	restoreGlobalRegisters(greg_copy);
	c += flushWrites();

	return c;
}

CommandBuffer Frontend::loadConfig(const std::string &file) {
	std::ifstream f(file);
	std::string line;

	while(std::getline(f, line)) {
		std::istringstream buffer(line);
		std::vector<std::string> ret{std::istream_iterator<std::string>(buffer), std::istream_iterator<std::string>()};

		if(ret.size() >= 2 && ret[0] != "#") {
			(*this)[ret[0]] = std::stoi(ret[1], 0 , 0);
		}
	}

	return flushWrites();
}

void Frontend::saveConfig(const std::string &file) {
	std::ofstream f(file);

	for(const auto &i : registers) {
		f << i.first <<  ' ' << readRegister(i.second) << std::endl;
	}
}

uint8_t Frontend::getPixelRegister(const std::string &name, int row, int column) {
	const PixRegister &r = pix_registers.at(name);

	return pixel_registers[r.addr][row][column];
}

std::vector<uint16_t> Frontend::getPixelRegisterDC(const std::string &name, int bit, int dc) {
	const PixRegister &r = pix_registers.at(name);

	std::vector<uint16_t> reg;
	reg.resize(42);

	for(int i = 0; i < ROWS; i++) {
		int j = ROWS - i - 1;

		reg[j / 16] |= ((pixel_registers[r.addr][i][2 * dc + 1] >> bit) & 1) << (i % 16);
		reg[j / 16 + 21] |= ((pixel_registers[r.addr][j][2 * dc] >> bit) & 1) << (i % 16);
	}

	return reg;
}

std::vector<uint8_t> Frontend::getDC(const PixRegister &r, int bit, int dc) {
	std::vector<uint8_t> reg;
	reg.resize(84);

	for(int i = 0; i < ROWS; i++) {
		int j = ROWS - i - 1;

		reg[j / 8] |= ((pixel_registers[r.addr][i][2 * dc + 1] >> bit) & 1) << (i % 8);
		reg[j / 8 + 42] |= ((pixel_registers[r.addr][j][2 * dc] >> bit) & 1) << (i % 8);
	}

	return reg;
}

void Frontend::saveGlobalRegisters(std::array<uint16_t, 36> &regs) {
	regs = global_registers;
}

void Frontend::restoreGlobalRegisters(std::array<uint16_t, 36> &regs) {
	for(int i = 0; i < 36; i++) {
		if(global_registers[i] != regs[i]) {
			written_registers.push_back(i);
		}
	}

	global_registers = regs;
}

Command Frontend::confMode(void) {
	return cmd.confMode();
}

Command Frontend::runMode(void) {
	return cmd.runMode();
}

Command Frontend::globalPulse(int width) {
	return cmd.globalPulse(width);
}

Command Frontend::globalReset(void) {
	return cmd.globalReset();
}

std::vector<std::string>::const_iterator Frontend::begin(void) {
	return register_list.cbegin();
}

std::vector<std::string>::const_iterator Frontend::end(void) {
	return register_list.cend();
}

CommandBuffer writePixelMask(const std::vector<std::string> &/*regs*/) {
	CommandBuffer c;

	return c;
}

CommandBuffer shiftPixelMask(const std::vector<std::string> &/*regs*/) {
	CommandBuffer c;

	return c;
}

