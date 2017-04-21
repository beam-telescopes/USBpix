#ifndef DEFINESI3_H
#define DEFINESI3_H

// ----------- CS FPGA - BLOCK RAM ------------------
#define CS_CONFIG_GLOBAL_WRITEMEM		0x0400
#define CS_CONFIG_GLOBAL_READMEM		0x0800
#define CS_CONFIG_PIXEL_WRITEMEM		0x1000
#define CS_CONFIG_PIXEL_READMEM			0x2000

// ----------- CS FPGA - REGISTER -------------------
#define CS_RESET						0 // 1-bit register, reset register/FSM
#define CS_XCK_PHASE					1 // 1-bit register, select phase of XCK: 1'b0: 0°; 1'b1: 180°
// SRAM
#define CS_SRAM_ADD_USB_0				2 // 21-bit register, SRAM address of USB interface
#define CS_SRAM_ADD_USB_1				3
#define CS_SRAM_ADD_USB_2				4
#define CS_SRAM_ADD_FSM_20_13			5 // 8-bit register, USB interrupt readout, MSB of SRAM address of readout FSM
#define CS_SRAM_FULL					6 // 1-bit register, SRAM full
#define CS_RESET_SRAM_ADD_FSM			7 // 1-bit register, set SRAM address of readout FSM to 21'b0 
#define CS_SRAM_READOUT_READY			8 // 1-bit register, SRAM readout ready
// Sync Signal
#define CS_START_SYNC_FSM				9 // 1-bit register, send sync signal
#define CS_SYNC_LENGTH					10 // 8-bit register, length of sync signal
// FE Configuration
#define CS_START_CONFIG_FSM				11 // 1_bit register, start FE configuration FSM
#define CS_CONFIG_FSM_DCNT_0			12 // 16-bit register, data length of pixel or global data in number of bits (DCNT)
#define CS_CONFIG_FSM_DCNT_1			13
// Strobe, LV1, uC Scan
#define CS_LENGTH_STRB_0				14 // 16-bit register, length of strobe signal
#define CS_LENGTH_STRB_1				15
#define CS_LENGTH_LV1					16 // 8-bit register, length of LV1 trigger
#define CS_DELAY_LV1					17 // 8-bit register, delay of LV1 trigger (after strobe signal or trigger)
#define CS_QUANTITY_STRB				18 // 8-bit register, set number of strobe signals, 0 means off
#define CS_LENGTH_TOTAL_0				19 // 16-bit register, set total length of signal cycle containing strobe and LV1 signal, 0 means off
#define CS_LENGTH_TOTAL_1				20
#define CS_CONFIGURATION_STEP			21 // 8-bit register, configuration number, effect on SRAM address of readout FSM in calib mode
#define CS_SCAN_LED						22 // 1-bit register, uC scan LED
// Measurement
#define CS_MEASUREMENT_START_STOP		23 // 1-bit register, start measurement
#define CS_MEASUREMENT_PAUSE_RESUME		24 // 1-bit register, pause measurement
// Readout Mode
#define CS_READOUT_MODE					25 // 2-bit register, select readout mode: 2'b01: calib mode; 2'b10: TOT mode; 2'b11: run mode, save raw data; 2'b00: fake injection mode, does not save any data
// Trigger Mode
#define CS_TRIGGER_MODE					26 // 8-bit register, select trigger mode: 8'b00000000: no external trigger, uC scan / strobe signal; 8'b00000001: external trigger (LEMO RX0) / TLU no handshake; 8'b00000010: USBpix self-trigger, use output of MonHit (Hitbus or whichever signal is selected by MONMUX); 8'b00000011: TLU simple handshake; 8'b00000100: TLU trigger data handshake
// Source Scan
#define CS_MINIMUM_TRIGGER_LENGTH		27 // 8-bit register, set minimum trigger lenght in numbers of BC, only triggers with a length of n+1 BC are accepted by 100% possibility, accepting a trigger with a length between n and n+1 BC is possible, 0 means off
#define CS_SAVE_UNKNOWN_WORD			28 // 1-bit register, when enabled readout FSM saves all data from FE
#define CS_DISABLE_EOE_WORD_COUNTER		29 // 1-bit register, disable EOE word counter, don't stop data taking when all EOE words (max. 16) of BCID window arrived, wait for L_TOTAL
#define CS_COUNTER_MODE					30 // 1-bit register, select 2'b00: triggers, 2'b01: hit words
#define CS_NUMBER_EVENTS_0				31 // 32-bit register, set number of LV1 triggers / hits / EOE to measure, see also CS_COUNTER_MODE, 0 means off
#define CS_NUMBER_EVENTS_1				32
#define CS_NUMBER_EVENTS_2				33
#define CS_NUMBER_EVENTS_3				34
#define CS_TRIGGER_COUNTER_0			35 // 32-bit register, counting triggers
#define CS_TRIGGER_COUNTER_1			36
#define CS_TRIGGER_COUNTER_2			37
#define CS_TRIGGER_COUNTER_3			38
#define CS_HIT_WORD_COUNTER_0			39 // 32-bit register, counting hit words
#define CS_HIT_WORD_COUNTER_1			40
#define CS_HIT_WORD_COUNTER_2			41
#define CS_HIT_WORD_COUNTER_3			42
#define CS_TRIGGER_RATE_0				43 // 16-bit register, trigger rate
#define CS_TRIGGER_RATE_1				44
#define CS_HIT_RATE_0					45 // 16-bit register, hit word rate
#define CS_HIT_RATE_1					46
// TLU
#define CS_ENABLE_RJ45					47 // 1-bit register, enable RJ45 port, if nothing is connected input wires are tied to high
#define CS_SAVE_COMPLETE_TRIGGER_NUMBER	48 // 1-bit register, save complete trigger number (31 bit instead of 23 bit)
#define CS_TLU_TRIGGER_DATA_MSB_FIRST	49 // 1-bit register, enable to flip bit order of trigger number
#define CS_TLU_TRIGGER_CLOCK_CYCLES		50 // 8-bit register, length of TLU trigger number in bits, max. 32-bit
#define CS_TLU_TRIGGER_DATA_DELAY		51 // 8-bit register, set length of additional wait cycles before saving TLU trigger data from shift register (depends on TLU trigger data length: 32 - CS_TLU_TRIGGER_CLOCK_CYCLES + additional delay = CS_TLU_TRIGGER_DATA_DELAY)
#define CS_TLU_TRIGGER_LOW_TIME_OUT		52 // 8-bit register, set time out for TLU trigger not going low; stops source scan if time out is over; set to a number smaller than CS_DELAY_LV1; 0 means off
// uC scan status
#define CS_SCAN_FEI3_ERROR_1			53 // 1-bit register, used for interrupt readout
#define CS_SCAN_FEI3_ERROR_2			54 // 1-bit register, used for interrupt readout
#define CS_SCAN_FEI3_ERROR_3			55 // 1-bit register, used for interrupt readout
#define CS_SCAN_FEI3_ERROR_4			56 // 1-bit register, used for interrupt readout
#define CS_SCAN_FEI3_WARNING			57 // 1-bit register, used for interrupt readout
// Noise Occupancy
#define CS_READ_NO_OC_0					-1 // 32-bit register
#define CS_READ_NO_OC_1					-1
#define CS_READ_NO_OC_2					-1
#define CS_READ_NO_OC_3					-1
#define CS_ENABLE_NO_OC					-1 // 1-bit register
#define CS_RESET_NO_OC					-1 // 1-bit register

// ----------- uC Commands -------------------
#define CMD_START_SCAN				1
#define CMD_STOP_SCAN				2
#define CMD_GET_SCAN_STATUS			3
#define CMD_GET_ERROR_STATUS		4

// ----------- uC Scan Status Flags ----------
#define ALL_FLAGS			0xFF
#define SCAN_BUSY			0x00 // scan is busy/running
#define SCAN_READY			0x01 // scan finished
#define SCAN_CANCELED		0x02 // scan externally cancelled
#define SCAN_ERROR			0x04 // timeout in uC is reached, scan aborted
#define SCAN_FEI3_WARNING	0x08 // FE-I3 Warning
#define SCAN_FEI3_ERROR_1	0x10 // FE-I3 Error 1
#define SCAN_FEI3_ERROR_2	0x20 // FE-I3 Error 2
#define SCAN_FEI3_ERROR_3	0x40 // FE-I3 Error 3
#define SCAN_FEI3_ERROR_4	0x80 // FE-I3 Error 4

// command implementaion, without parity, MSB = ReadPixel
// one-bit commands
#define FE_CMD_NULL					0x000000
#define FE_REF_RESET				0x000002
#define FE_SOFT_RESET				0x00000C // SoftReset1 + SoftReset2
#define FE_CLOCK_GLOBAL				0x000010
#define FE_WRITE_GLOBAL				0x000060 // WriteGlobal1 + WriteGlobal2
#define FE_READ_GLOBAL				0x000080
#define FE_CLOCK_PIXEL				0x000100
#define FE_WRITE_HITBUS				0x000200
#define FE_WRITE_SELECT				0x000400
#define FE_WRITE_MASK_B				0x000800
#define FE_WRITE_TDAC0				0x001000
#define FE_WRITE_TDAC1				0x002000
#define FE_WRITE_TDAC2				0x004000
#define FE_WRITE_TDAC3				0x008000
#define FE_WRITE_TDAC4				0x010000
#define FE_WRITE_TDAC5				0x020000
#define FE_WRITE_TDAC6				0x040000
#define FE_WRITE_FDAC0				0x080000
#define FE_WRITE_FDAC1				0x100000
#define FE_WRITE_FDAC2				0x200000
#define FE_WRITE_KILL				0x400000
#define FE_READ_PIXEL				0x800000
// three-bit commands
#define FE_RB_HITBUS				0x800300 // ReadPixel + ClockPixel + WriteHitbus
#define FE_RB_SELECT				0x800500 // ReadPixel + ClockPixel + WriteSelect
#define FE_RB_MASK_B				0x800900 // ReadPixel + ClockPixel + WriteMask
#define FE_RB_TDAC0					0x801100 // ReadPixel + ClockPixel + WriteTDAC0
#define FE_RB_TDAC1					0x802100 // ReadPixel + ClockPixel + WriteTDAC1
#define FE_RB_TDAC2					0x804100 // ReadPixel + ClockPixel + WriteTDAC2
#define FE_RB_TDAC3					0x808100 // ReadPixel + ClockPixel + WriteTDAC3
#define FE_RB_TDAC4					0x810100 // ReadPixel + ClockPixel + WriteTDAC4
#define FE_RB_TDAC5					0x820100 // ReadPixel + ClockPixel + WriteTDAC5
#define FE_RB_TDAC6					0x840100 // ReadPixel + ClockPixel + WriteTDAC6
#define FE_RB_FDAC0					0x880100 // ReadPixel + ClockPixel + WriteFDAC0
#define FE_RB_FDAC1					0x900100 // ReadPixel + ClockPixel + WriteFDAC1
#define FE_RB_FDAC2					0xA00100 // ReadPixel + ClockPixel + WriteFDAC2
#define FE_RB_KILL					0xC00100 // ReadPixel + ClockPixel + WriteKill

// command implementaion, without parity, MSB = CommandParity
// one-bit commands
#define FE_CMD_NULL_MSB_PARITY		0x000000
#define FE_REF_RESET_MSB_PARITY		0x400000
#define FE_SOFT_RESET_MSB_PARITY	0x300000 // SoftReset1 + SoftReset2
#define FE_CLOCK_GLOBAL_MSB_PARITY	0x080000
#define FE_WRITE_GLOBAL_MSB_PARITY	0x060000 // WriteGlobal1 + WriteGlobal2
#define FE_READ_GLOBAL_MSB_PARITY	0x010000
#define FE_CLOCK_PIXEL_MSB_PARITY	0x008000
#define FE_WRITE_HITBUS_MSB_PARITY	0x004000
#define FE_WRITE_SELECT_MSB_PARITY	0x002000
#define FE_WRITE_MASK_B_MSB_PARITY	0x001000
#define FE_WRITE_TDAC0_MSB_PARITY	0x000800
#define FE_WRITE_TDAC1_MSB_PARITY	0x000400
#define FE_WRITE_TDAC2_MSB_PARITY	0x000200
#define FE_WRITE_TDAC3_MSB_PARITY	0x000100
#define FE_WRITE_TDAC4_MSB_PARITY	0x000080
#define FE_WRITE_TDAC5_MSB_PARITY	0x000040
#define FE_WRITE_TDAC6_MSB_PARITY	0x000020
#define FE_WRITE_FDAC0_MSB_PARITY	0x000010
#define FE_WRITE_FDAC1_MSB_PARITY	0x000008
#define FE_WRITE_FDAC2_MSB_PARITY	0x000004
#define FE_WRITE_KILL_MSB_PARITY	0x000002
#define FE_READ_PIXEL_MSB_PARITY	0x000001
// three-bit commands
#define FE_RB_HITBUS_MSB_PARITY		0x00C001 // ReadPixel + ClockPixel + WriteHitbus
#define FE_RB_SELECT_MSB_PARITY		0x00A001 // ReadPixel + ClockPixel + WriteSelect
#define FE_RB_MASK_B_MSB_PARITY		0x009001 // ReadPixel + ClockPixel + WriteMask
#define FE_RB_TDAC0_MSB_PARITY		0x008801 // ReadPixel + ClockPixel + WriteTDAC0
#define FE_RB_TDAC1_MSB_PARITY		0x008401 // ReadPixel + ClockPixel + WriteTDAC1
#define FE_RB_TDAC2_MSB_PARITY		0x008201 // ReadPixel + ClockPixel + WriteTDAC2
#define FE_RB_TDAC3_MSB_PARITY		0x008101 // ReadPixel + ClockPixel + WriteTDAC3
#define FE_RB_TDAC4_MSB_PARITY		0x008081 // ReadPixel + ClockPixel + WriteTDAC4
#define FE_RB_TDAC5_MSB_PARITY		0x008041 // ReadPixel + ClockPixel + WriteTDAC5
#define FE_RB_TDAC6_MSB_PARITY		0x008021 // ReadPixel + ClockPixel + WriteTDAC6
#define FE_RB_FDAC0_MSB_PARITY		0x008011 // ReadPixel + ClockPixel + WriteFDAC0
#define FE_RB_FDAC1_MSB_PARITY		0x008001 // ReadPixel + ClockPixel + WriteFDAC1
#define FE_RB_FDAC2_MSB_PARITY		0x008001 // ReadPixel + ClockPixel + WriteFDAC2
#define FE_RB_KILL_MSB_PARITY		0x008001 // ReadPixel + ClockPixel + WriteKill

// scan commands
#define SHIFT_HITBUS				0x01
#define SHIFT_SELECT				0x02
#define SHIFT_MASK_B				0x04
#define SHIFT_KILL					0x08

// command register identifier
#define COMMAND						0
#define CHIPADDRESS					1

// empty 3-bit, to fill up 32-bit
#define EMPTY3BIT					2

// global register identifier
#define GLOBALPARITY			3
#define CHIP_LATENCY			4
#define SELFTRGDELAY			5
#define SELFTRGWIDTH			6
#define ENABLESELFTRG			7
#define ENABLEHITPARITY			8
#define SELECTDO				9
#define SELECTMONHIT			10
#define TSITSCENABLE			11
#define SELECTDATAPHASE			12
#define ENABLEEOEPARITY			13
#define HITBUSSCALER			14
#define MONLEAKADC				15
#define AREGTRIM				16
#define ENABLEAREGMEAS			17
#define AREGMEAS				18
#define ENABLEAREG				19
#define ENABLELVDSREFMEAS		20
#define DREGTRIM				21
#define ENABLEDREGMEAS			22
#define DREGMEAS				23
#define CAPMEASCIRCUIT			24
#define ENABLECAPTEST			25
#define ENABLEANALOGOUT			26
#define TESTPIXELMUX			27
#define ENABLEVCALMEAS			28
#define ENABLELEAKMEAS			29
#define ENABLEBUFFERBOOST		30
#define ENABLECOL8				31
#define TESTDACIVDD2			32
#define IVDD2DAC				33
#define IDDAC					34
#define TESTDACID				35
#define ENABLECOL7				36
#define TESTDACIP2				37
#define IP2DAC					38
#define IPDAC					39
#define TESTDACIP				40
#define ENABLECOL6				41
#define TESTDACITRIMTH			42
#define ITRIMTHDAC				43
#define IFDAC					44
#define TESTDACIF				45
#define ENABLECOL5				46
#define TESTDACITRIMIF			47
#define ITRIMIFDAC				48
#define VCALDAC					49
#define TESTDACVCAL				50
#define ENABLECOL4				51
#define HITINJECTCAPSEL			52
#define ENABLEEXTINJ			53
#define TESTANALOGREF			54
#define EOCMUXCONTROL			55
#define CEUCLOCKCONTROL			56
#define ENABLEDIGITALINJ		57
#define ENABLECOL3				58
#define TESTDACITH1				59
#define ITH1DAC					60
#define ITH2DAC					61
#define TESTDACITH2				62
#define ENABLECOL2				63
#define TESTDACIL				64
#define ILDAC					65
#define IL2DAC					66
#define TESTDACIL2				67
#define ENABLECOL1				68
#define THRMIN					69
#define THRDUB					70
#define READMODE				71
#define ENABLECOL0				72
#define HITBUSENABLE			73
#define GLOBALTDAC				74
#define ENABLETUNE				75
#define ENABLEBIASCOMP			76
#define ENABLEIPMONITOR			77

// pixel register identifiers
#define PIXEL2880		3
#define PIXEL2848		4
#define PIXEL2816		5
#define PIXEL2784		6
#define PIXEL2752		7
#define PIXEL2720		8
#define PIXEL2688		9
#define PIXEL2656		10
#define PIXEL2624		11
#define PIXEL2592		12
#define PIXEL2560		13
#define PIXEL2528		14
#define PIXEL2496		15
#define PIXEL2464		16
#define PIXEL2432		17
#define PIXEL2400		18
#define PIXEL2368		19
#define PIXEL2336		20
#define PIXEL2304		21
#define PIXEL2272		22
#define PIXEL2240		23
#define PIXEL2208		24
#define PIXEL2176		25
#define PIXEL2144		26
#define PIXEL2112		27
#define PIXEL2080		28
#define PIXEL2048		29
#define PIXEL2016		30
#define PIXEL1984		31
#define PIXEL1952		32
#define PIXEL1920		33
#define PIXEL1888		34
#define PIXEL1856		35
#define PIXEL1824		36
#define PIXEL1792		37
#define PIXEL1760		38
#define PIXEL1728		39
#define PIXEL1696		40
#define PIXEL1664		41
#define PIXEL1632		42
#define PIXEL1600		43
#define PIXEL1568		44
#define PIXEL1536		45
#define PIXEL1504		46
#define PIXEL1472		47
#define PIXEL1440		48
#define PIXEL1408		49
#define PIXEL1376		50
#define PIXEL1344		51
#define PIXEL1312		52
#define PIXEL1280		53
#define PIXEL1248		54
#define PIXEL1216		55
#define PIXEL1184		56
#define PIXEL1152		57
#define PIXEL1120		58
#define PIXEL1088		59
#define PIXEL1056		60
#define PIXEL1024		61
#define PIXEL992		62
#define PIXEL960		63
#define PIXEL928		64
#define PIXEL896		65
#define PIXEL864		66
#define PIXEL832		67
#define PIXEL800		68
#define PIXEL768		69
#define PIXEL736		70
#define PIXEL704		71
#define PIXEL672		72
#define PIXEL640		73
#define PIXEL608		74
#define PIXEL576		75
#define PIXEL544		76
#define PIXEL512		77
#define PIXEL480		78
#define PIXEL448		79
#define PIXEL416		80
#define PIXEL384		81
#define PIXEL352		82
#define PIXEL320		83
#define PIXEL288		84
#define PIXEL256		85
#define PIXEL224		86
#define PIXEL192		87
#define PIXEL160		88
#define PIXEL128		89
#define PIXEL96			90
#define PIXEL64			91
#define PIXEL32			92

// ADC Identifiers
#define	ADC_GAIN				0
#define	ADC_OFFSET				1
#define	TIA_GAIN				2
#define	N_DUMMY					3
#define	N_MEAN					4

// Command register identifiers
#define HITBUS					0
#define SELECT					1
#define MASK_B					2
#define TDAC0					3
#define TDAC1					4
#define TDAC2					5
#define TDAC3					6
#define TDAC4					7
#define TDAC5					8
#define TDAC6					9
#define FDAC0					10
#define FDAC1					11
#define FDAC2					12
#define KILL					13
// Logical Command register identfiers supported by higer level functions
#define PIX_TDAC				50
#define PIX_FDAC				51

//--- Select_xxx multiplexer selects
#define REGCLKIN				0
#define HITBUS_OUT				1
#define DATA_OUT				3
#define PIXEL_OUT				11
#define GLOBAL_OUT				15

#define COMMAND_REG_SIZE_WO_ADD	24
#define COMMAND_REG_SIZE		32
#define COMMAND_REG_BYTESIZE	4 // #byte command register

#define GLOBAL_REG_SIZE			231

#define GLOBAL_REG_ITEMS		78
#define GLOBAL_REG_BITSIZE		260
#define GLOBAL_REG_BYTESIZE		33 // #byte command + global register

#define PIXEL_REG_ITEMS			93
#define PIXEL_REG_BITSIZE		3032
#define PIXEL_REG_BYTESIZE		364 // #byte command + pixel register
#define PIXEL_REG_LATCHES		14

#define DCNT_READPIXEL			2 // solves mask shifting problem, normally DCNT=1 for ReadPixel + ClockPixel + load strobe command; see FE-I3 documentation of Kevin Einsweiler
#define DCNT_COMMAND			0
#define DCNT_GLOBAL				231
#define DCNT_PIXEL				2880

// ADC Configuration
#define ADC_REG_ITEMS			5
#define ADC_REG_BITSIZE			160
#define ADC_REG_BYTESIZE		5

// SRAM identifiers
#define SRAM_BYTESIZE			2097152
#define WORDSIZE				4 // in bytes
#define SRAM_WORDSIZE			SRAM_BYTESIZE/WORDSIZE

// Data Format
#define ISGLOBAL				0x01
#define ISADC					0x02
#define ISPIXEL					0x00

// masks for FE-I3 raw data processing
// EOE / hit word
#define HEADER_MASK				0xFE000000 // hit word & EOE word, 32-bit word, 7-bit
#define HEADER_26_MASK			0x02000000 // hit word & EOE word, 26-bit word, 1-bit
#define L1ID_MASK				0x00000F00 // EOE word
#define BCID_3_0_MASK			0x01E00000 // hit word & EOE word
#define BCID_7_4_MASK			0x000000F0 // EOE word
#define WARN_MASK				0x0000000F // EOE word
#define FLAG_MASK				0x001FE000 // EOE word
#define FLAG_NO_ERROR_MASK		0x001E0000 // EOE word, flag with no error
#define	FLAG_ERROR_1_MASK		0x001C2000 // EOE word, flag with error n=1
#define	FLAG_ERROR_2_MASK		0x001C4000 // EOE word, flag with error n=2
#define	FLAG_ERROR_3_MASK		0x001C8000 // EOE word, flag with error n=4
#define	FLAG_ERROR_4_MASK		0x001D0000 // EOE word, flag with error n=8
#define ROW_MASK				0x001FE000 // hit word, 0 - 159
#define COL_MASK				0x00001F00 // hit word, 0 - 17
#define TOT_MASK				0x000000FF // hit word, 8-bit, HitParity disabled
#define TOTPARITY_MASK			0x0000007F // hit word, 7-bit, HitParity enabled
#define EOEPARITY_MASK			0x00001000 // EOE word
#define HITPARITY_MASK			0x00000080 // hit word

#define FLAG_WO_STATUS			0xE0 // flag without error status
#define FLAG_NO_ERROR			0xF0 // flag with no error
#define FLAG_ERROR_1			0xE1 // flag with error n=1
#define FLAG_ERROR_2			0xE2 // flag with error n=2
#define FLAG_ERROR_3			0xE4 // flag with error n=4
#define FLAG_ERROR_4			0xE8 // flag with error n=8

// trigger word
#define TRIGGER_WORD_HEADER_MASK		0x80000000 // trigger header
#define TRIGGER_NUMBER_MASK				0x7FFFFF00 // trigger number
#define EXT_TRIGGER_MODE_MASK			0x00000007 // trigger number
#define TRIGGER_WORD_ERROR_MASK			0x000000F8 // trigger number
#define	L_TOTAL_TIME_OUT_MASK			0x00000004 // trigger number
#define EOE_WORD_WRONG_NUMBER_MASK		0x00000010 // trigger number
#define UNKNOWN_WORD_MASK				0x00000020 // trigger number
#define EOE_WORD_WARNING_MASK			0x00000040 // trigger number
#define EOE_WORD_ERROR_MASK				0x00000080 // trigger number

// macros for for FE-I3 data processing
// EOE / hit word
#define HEADER_MACRO(X)			((HEADER_MASK & X) >> 25)
#define FLAG_MACRO(X)			((FLAG_MASK & X) >> 13)
#define ROW_MACRO(X)			((ROW_MASK & X) >> 13)
#define COL_MACRO(X)			((COL_MASK & X) >> 8)
#define BCID_3_0_MACRO(X)		((BCID_3_0_MASK & X) >> 21)
#define L1ID_MACRO(X)			((L1ID_MASK & X) >> 8)
#define BCID_MACRO(X)			(((BCID_3_0_MASK & X) >> 21) | (BCID_7_4_MASK & X))
#define TOT_MACRO(X)			(TOT_MASK & X)
#define WARN_MACRO(X)			(WARN_MASK & X)
// trigger word
#define TRIGGER_WORD_HEADER_MACRO(X)	((TRIGGER_WORD_HEADER_MASK & X) >> 31)
#define TRIGGER_NUMBER_MACRO(X)			((TRIGGER_NUMBER_MASK & X) >> 8)
#define TRIGGER_MODE_MACRO(X)			(EXT_TRIGGER_MODE_MASK & X)
#define TRIGGER_WORD_ERROR_MACRO(X)		((TRIGGER_WORD_ERROR_MASK & X) >> 3)

#endif // DEFINESI3_H
