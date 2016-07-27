/******************************************************************************
 *
 *  Title  : vmeAddressMap.h
 *  Version 0.0
 *
 *  Description: Addresses of ROD VME registers.
 *  Note:        All registers except rodStatusRegs are READ/WRITE by Host.
 *  Related files: ...
 *
 *  Author: Lukas Tomasek, tomasekl@fzu.cz
 *
 ******************************************************************************/

#ifndef VME_ADDRESS_MAP_H	 /* multiple inclusion protection */
#define VME_ADDRESS_MAP_H

/******************************************************************************
 *                               Definitions                                  *
 ******************************************************************************/

/*
 * VME - HPI interface:
 *
 * 	vme_d[15..0] -> hd[15..0]
 * 	vme_a[21] -> hctrl[0]
 * 	vme_a[22] -> hctrl[1]
 * 	vme_a[1] -> hwil
 * 	vme_rnw -> hrnw
 * 	vme_cs_n -> hcs_n
 * 	vme_ds_n -> hds1_n
 * 	vme_dsack0_n <- hrdy_n
 * 	vme_int_n <- hint_n
 * 	vme_be_n[1..0] -> hbe_n[1..0]
 */


#define BASE_ADDRESS(slotNumber)  (slotNumber<<24)

/* absoluteVMEaddress = baseAddress + relativeAddress */

/*
 * HPI Registers - 32bit size registers with 2*16bit access!!!
 */

#define HPI_CONTROL_REG_REL_ADDR				0x000000
#define HPI_ADDRESS_REG_REL_ADDR				0x200000
#define HPI_DATA_REG_WITH_AUTOINC_REL_ADDR		0x400000
#define HPI_DATA_REG_WITHOUT_AUTOINC_REL_ADDR	0x600000

/*
 * FPGA Program/Reset Manager - 32bit size registers - valid only low 8 bits!
 */

#define FPGA_CONTROL_REG_0_REL_ADDR		0xC00000	/* fpga cnfg control reg */
#define FPGA_CONTROL_REG_1_REL_ADDR		0xC00004	/* fpga rst control reg */
#define FPGA_CONTROL_REG_2_REL_ADDR		0xC00008	/* vme dsp rst control reg */
#define FPGA_CONTROL_REG_3_REL_ADDR		0xC0000C	/* flash control reg */
#define FPGA_CONTROL_REG_4_REL_ADDR		0xC00010	/* flash address(23:0)+data(31:24) reg */
#define FPGA_CONTROL_REG_5_REL_ADDR		0xC00014	/*   */
#define FPGA_CONTROL_REG_6_REL_ADDR		0xC00018	/*   */
#define FPGA_CONTROL_REG_7_REL_ADDR		0xC0001C	/*   */

#define FPGA_STATUS_REG_0_REL_ADDR		0xC00020	/* fpga cnfg status reg */
#define FPGA_STATUS_REG_1_REL_ADDR		0xC00024	/* fpga reset status reg */
#define FPGA_STATUS_REG_2_REL_ADDR		0xC00028    /* vme dsp reset status reg */
#define FPGA_STATUS_REG_3_REL_ADDR		0xC0002C	/* fpga init status reg */
#define FPGA_STATUS_REG_4_REL_ADDR		0xC00030	/* flash status reg */
#define FPGA_STATUS_REG_5_REL_ADDR		0xC00034	/* halt status */
#define FPGA_STATUS_REG_6_REL_ADDR		0xC00038	/* ROD serial number */
#define FPGA_STATUS_REG_7_REL_ADDR		0xC0003C	/* flash read data reg */


/******************************************************************************/
#endif /* VME_ADDRESS_MAP_H */
