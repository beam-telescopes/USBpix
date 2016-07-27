//File: RodVmeAddresses.h

#ifndef SCTPIXELROD_RODVMEADDRESSES_H 
#define SCTPIXELROD_RODVMEADDRESSES_H

/*!
 * RodVmeAddresses.h is a wrapper around the testBench C code header file
 * vmeAddressMap.h. This wrapper adapts things to appear more like C++ and to
 * declare things const so the compiler can catch attempts to change them in the
 * code.
 *
 * @author Tom Meyer (meyer@iastate.edu) - originator
 */

namespace SctPixelRod {

#include "dsp_types.h"
#include "vmeAddressMap.h"

// Give some shorter aliases to the HPI registers
const unsigned long HPIC = HPI_CONTROL_REG_REL_ADDR;
const unsigned long HPIA = HPI_ADDRESS_REG_REL_ADDR;
const unsigned long HPID_AUTO = HPI_DATA_REG_WITH_AUTOINC_REL_ADDR;
const unsigned long HPID_NOAUTO = HPI_DATA_REG_WITHOUT_AUTOINC_REL_ADDR;

//This MUST be a multiple of 0x40 (256 decimal) because of an obscure feature
//in the VME requirement for block transfers.
const long MAX_HPID_WORD_ELEMENTS = 0x10000; // 32b words - 16 bits max for VME block xfers

// Flash Registers
const unsigned long FLASH_ADDR_WRITEDATA_REG = 0x00c00010;
const unsigned long FLASH_CONTROL_REG = 0x00c0000c;

//FPGA Program/Reset Manager - 32bit size registers - valid only low 8 bits! 
const unsigned long FPGA_CONTROL_REG_REL_ADDR[8] = {
  FPGA_CONTROL_REG_0_REL_ADDR, // fpga cnfg control reg
  FPGA_CONTROL_REG_1_REL_ADDR, // fpga rst control reg
  FPGA_CONTROL_REG_2_REL_ADDR, // vme dsp rst control reg
  FPGA_CONTROL_REG_3_REL_ADDR, // flash control reg
  FPGA_CONTROL_REG_4_REL_ADDR, // flash address(23:0)+data(31:24) reg
  FPGA_CONTROL_REG_5_REL_ADDR, // 
  FPGA_CONTROL_REG_6_REL_ADDR, // 
  FPGA_CONTROL_REG_7_REL_ADDR  // 
};

// FPGA Registers
const unsigned long FPGA_STATUS_REG_REL_ADDR[8] = {
  FPGA_STATUS_REG_0_REL_ADDR, // fpga cnfg status reg
  FPGA_STATUS_REG_1_REL_ADDR, // fpga reset status reg
  FPGA_STATUS_REG_2_REL_ADDR, // vme dsp reset status reg
  FPGA_STATUS_REG_3_REL_ADDR, // fpga init status reg
  FPGA_STATUS_REG_4_REL_ADDR, // flash status reg
  FPGA_STATUS_REG_5_REL_ADDR, // halt status
  FPGA_STATUS_REG_6_REL_ADDR, // ROD Serial Number
  FPGA_STATUS_REG_7_REL_ADDR  // flash read data reg
}; 


// define ROD reset status values 
#define FPGA_RESET_COMPLETE   0x1F
#define ALLDSP_RESET_COMPLETE 0x3E

// define DSP locations
#define MAGIC_LOCATION 0x80000000
#define MEMORYMAP_REG  0x80000004

// Define flash memory addresses, values, and bits
const unsigned long FPGA_FLASH_0_BOTTOM = 0xE00000;
const unsigned long FPGA_FLASH_1_BOTTOM = 0xE80000;
const unsigned long FPGA_FLASH_2_BOTTOM = 0xF00000;
const unsigned long FPGA_FLASH_REL_ADDR_REVE = 0xE00000;
const unsigned long FLASH_MEMORY_SIZE = 0x80000;      // 512 kB
const unsigned long FLASH_MEMORY_SIZE_REVE = 0x200000;      // 2 MB
const unsigned long FLASH_SECTOR_SIZE = 0x1000;       // 4 kB
const unsigned long FLASH_SECTOR_SIZE_REVE = 0x10000;       // 64 kB
const double FLASH_TIMEOUT = 5.0;                     // in seconds
const unsigned long CHIP_ERASE_TIME_MS = 100;
const unsigned long SECTOR_ERASE_TIME_MS = 25*2;   // 25ms times 2 for safety
const unsigned long READ_HANDSHAKE_BIT = 0;
const unsigned long WRITE_COMMAND_HANDSHAKE_BIT = 1;
const unsigned long WRITE_DATA_HANDSHAKE_BIT = 2;
const unsigned long NUMBER_OR_SECTORS = 128;
const unsigned long NUMBER_OF_SECTORS_REVE = 32;

} //  End namespace SctPixelRod

#endif // SCTPIXELROD_RODVMEADDRESSES_H
