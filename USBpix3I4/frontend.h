#ifndef _FRONTEND_H
#define _FRONTEND_H

#include <string>
#include <array>
#include <vector>
#include <map>
#include <functional>
#include <cstdint>

#include "registerproxy.h"
#include "commandbuffer.h"
#include "commandgenerator.h"
#include "util.h"

#include "dllexport.h"

struct PixRegister {
	int addr;
	int size;
	int px_strobes;

	enum {
		LATCH = 1 << 0,
		MSB = 1 << 2,
		LATCH_MSB = LATCH | MSB
	} type;
};

class DllExport Frontend {
	public:
		enum Flavour {
			FEI4A,
			FEI4B
		};

		static const int ROWS = 336;
		static const int COLUMNS = 80;

		static const int MIN_GLOBAL_REG = 0;
		static const int MAX_GLOBAL_REG = 35;

		Frontend(int _address, Flavour _flavour = FEI4B) : address(_address), flavour(_flavour), cmd(_address) {
			loadRegisterList();
			pixel_registers = {{{{{{0}}}}}};
		}

		RegisterProxy operator[](const std::string &name);
		RegisterProxy operator[](int reg);

		std::vector<std::string>::const_iterator begin(void);
		std::vector<std::string>::const_iterator end(void);

		CommandBuffer readGlobalRegisters(const std::vector<std::string> &regs);
		CommandBuffer readGlobalRegisters(const std::vector<int> &regs);
		CommandBuffer readGlobalRegisters(void);

		CommandBuffer writeGlobalRegisters(const std::vector<std::string> &regs);
		CommandBuffer writeGlobalRegisters(const std::vector<int> &regs);
		CommandBuffer writeGlobalRegisters(void);

		void setPixelRegister(const std::string &name, const std::vector<int> &dcs, uint8_t value);
		void setPixelRegister(const std::string &name, uint8_t value);
		void setPixelRegister(const std::string &name, int x, int y, uint8_t value);
		void setPixelRegister(const std::string &name, const Matrix<uint8_t, ROWS, COLUMNS> &value);

		uint8_t getPixelRegister(const std::string &name, int row, int column);
		std::vector<uint16_t> getPixelRegisterDC(const std::string &name, int bit, int dc);

		CommandBuffer writePixelRegister(const std::string &name);
		CommandBuffer writePixelRegister(const std::string &name, const std::vector<int> &dcs, bool all_dcs = false);
		CommandBuffer readPixelRegister(const std::string &name, int bit, const std::vector<int> &dcs);
	
		CommandBuffer writePixelMask(const std::vector<std::string> &regs);
		CommandBuffer shiftPixelMask(const std::vector<std::string> &regs);

		void disableGlobalPulseRegisters(void);
		CommandBuffer loadConfig(const std::string &file);
		void saveConfig(const std::string &file);

		CommandBuffer flushWrites(void);
		Command runMode(void);
		Command confMode(void);
		Command globalPulse(int width);
		Command globalReset(void);

		const int address;
		const Flavour flavour;

	protected:
		uint64_t readRegister(const FERegister &reg) const;
		void writeRegister(const FERegister &reg, uint64_t val);
		uint64_t readRegisterNr(int reg) const;
		void writeRegisterNr(int reg, uint64_t val);

		uint64_t readCompoundRegister(const FERegister &reg) const;
		void writeCompoundRegister(const FERegister &reg, uint64_t val);

		void saveGlobalRegisters(std::array<uint16_t, 36> &regs);
		void restoreGlobalRegisters(std::array<uint16_t, 36> &regs);

		void loadRegisterList(void);
		std::vector<int> uniqueRegisterList(const std::vector<std::string> &regs);
		std::vector<uint8_t> getDC(const PixRegister &r, int bit, int dc);

		CommandGenerator cmd;

		std::array<uint16_t, 36> global_registers = std::array<uint16_t, 36> {{0}};
		std::map<std::string, FERegister> registers;
		std::vector<std::string> register_list;

		std::array<Matrix<uint8_t, ROWS, COLUMNS>, 7> pixel_registers;// = {{{{{{0}}}}}};
		std::map<std::string, PixRegister> pix_registers;

		std::vector<int> written_registers;
};

#endif
