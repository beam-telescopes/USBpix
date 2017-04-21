#ifndef DATASTRUCTDEFINES
#define DATASTRUCTDEFINES

// ******************* commands ****************************
static ConfDataStruct triggerCnfRegItem[TRIGGER_REG_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,   0, false},
	{ 1,    COMMAND,         "COMMAND",              1,   5,  29, false}
};

static ConfDataStruct fastCnfRegItem[FAST_REG_ITEMS] = {
	{ 0,    IDLE,           "IDLE",                  0,   1,   0, false},
	{ 1,    COMMAND,        "COMMAND",               1,   9,   0, false}
};

static ConfDataStruct slowCnfRegItem[SLOW_REG_ITEMS] = {
	{ 0,    IDLE,           "IDLE",                  0,   1,   0, false},
	{ 1,    COMMAND,        "COMMAND",               1,   13,  0, false},
	{ 2,    CHIPADDRESS,    "CHIPADDRESS",          14,   4,   0, false},
	{ 3,    FIELD5,         "FIELD5",               18,   6,  56, false}
};


// Dummy reg used for FE-I4A and B missing regs
static ConfDataStruct gloCnfRegItem_DUMMY[CONFIG_REG_DUMMY_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMANDDUMMY,    "COMMANDDUMMY",         1,  13,      0, false}, // "Special name" needed to avoid automated regaddress setting
	{ 2,    CHIPADDRESSDUMMY,"CHIPADDRESSDUMMY",    14,   4,      0, false}, // "Special name" needed to avoid automated regaddress setting
	{ 3,    REGADDRESSDUMMY, "REGADDRESSDUMMY",     18,   6,      0, false}, // "Special name" needed to avoid automated regaddress setting
	{ 4,    DUMMY,			 "DUMMY",               24,  16,      0, false}
};

// **************** 42 global registers for FE-I4A ***********************
static ConfDataStruct gloCnfRegItem_FEI4A_2[CONFIG_REG_FEI4A_2_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,      2, false},
	{ 4,    TRIGCNT,         "TRIGCNT",             24,   4,      0, false},
	{ 5,    CONFADDRENABLE,  "CONFADDRENABLE",      28,   1,      1, false},
	{ 6,    CFGSPARE2,       "CFGSRARE2",           29,  11,      0, false}	
};

static ConfDataStruct gloCnfRegItem_FEI4A_3[CONFIG_REG_FEI4A_3_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,      3, false},
	{ 4,    ERRMASK0,        "ERRMASK0",            24,   16,     0, false}	
};

static ConfDataStruct gloCnfRegItem_FEI4A_4[CONFIG_REG_FEI4A_4_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,      4, false},
	{ 4,    ERRMASK1,        "ERRMASK1",            24,  16,      0, false}
	
};

static ConfDataStruct gloCnfRegItem_FEI4A_5[CONFIG_REG_FEI4A_5_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,      5, false},
	{ 4,    PRMPVBP_R,       "PRMPVBP_R",           24,   8,     43, true},
	{ 5,    VTHIN,           "VTHIN",               32,   8,    255, true}
};

static ConfDataStruct gloCnfRegItem_FEI4A_6[CONFIG_REG_FEI4A_6_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,      6, false},
	{ 4,    DISVBN_CPPM,     "DISVBN_CPPM",         24,   8,     62, true},
	{ 5,    PRMPVBP,         "PRMPVBP",             32,   8,     43, true}
};

static ConfDataStruct gloCnfRegItem_FEI4A_7[CONFIG_REG_FEI4A_7_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,      7, false},
	{ 4,    TDACVBP,         "TDACVBP",             24,   8,    255, true},
	{ 5,    DISVBN,          "DISVBN",              32,   8,     26, true}
};

static ConfDataStruct gloCnfRegItem_FEI4A_8[CONFIG_REG_FEI4A_8_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,      8, false},
	{ 4,    AMP2VBN,         "AMP2VBN",             24,   8,     79, true},
	{ 5,    AMP2VBPFOL,      "AMP2VBPFOL",          32,   8,     26, true}
	
};

static ConfDataStruct gloCnfRegItem_FEI4A_9[CONFIG_REG_FEI4A_9_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,      9, false},
	{ 4,    PRMPVBP_T,       "PRMPVBP_T",           24,   8,     64, true},
	{ 5,    AMP2VBP,         "AMP2VBP",             32,   8,     85, true}
};

static ConfDataStruct gloCnfRegItem_FEI4A_10[CONFIG_REG_FEI4A_10_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     10, false},
	{ 4,    FDACVBN,         "FDACVBN",             24,   8,     20, true},
	{ 5,    AMP2VBPFF,       "AMP2VBPFF",           32,   8,     13, true}
};

static ConfDataStruct gloCnfRegItem_FEI4A_11[CONFIG_REG_FEI4A_11_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     11, false},
	{ 4,    PRMPVBNFOL,      "PRMPVBNFOL",          24,   8,    106, true},
	{ 5,    PRMPVBP_L,       "PRMPVBP_L",           32,   8,     43, true}
};

static ConfDataStruct gloCnfRegItem_FEI4A_12[CONFIG_REG_FEI4A_12_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     12, false},
	{ 4,    PRMPVBPF,        "PRMPVBPF",            24,   8,    150, true},
	{ 5,    PRMPVBNLCC,      "PRMPVBNLCC",          32,   8,      0, true}
};

static ConfDataStruct gloCnfRegItem_FEI4A_13[CONFIG_REG_FEI4A_13_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     13, false},
	{ 4,    PXSTROBES1,      "PXSTROBES1",          24,   1,      0, false},
	{ 5,    PXSTROBES0,      "PXSTROBES0",          25,   1,      0, false},
	{ 6,    PXSTROBE0,       "PXSTROBE0",           26,   1,      0, false},
	{ 7,    PXSTROBE1,       "PXSTROBE1",           27,   1,      0, false},
	{ 8,    PXSTROBE2,       "PXSTROBE2",           28,   1,      0, false},
	{ 9,    PXSTROBE3,       "PXSTROBE3",           29,   1,      0, false},
	{10,    PXSTROBE4,       "PXSTROBE4",           30,   1,      0, false},
	{11,    PXSTROBE5,       "PXSTROBE5",           31,   1,      0, false},
	{12,    PXSTROBE6,       "PXSTROBE6",           32,   1,      0, false},
	{13,    PXSTROBE7,       "PXSTROBE7",           33,   1,      0, false},
	{14,    PXSTROBE8,       "PXSTROBE8",           34,   1,      0, false},
	{15,    PXSTROBE9,       "PXSTROBE9",           35,   1,      0, false},
	{16,    PXSTROBE10,      "PXSTROBE10",          36,   1,      0, false},
	{17,    PXSTROBE11,      "PXSTROBE11",          37,   1,      0, false},
	{18,    PXSTROBE12,      "PXSTROBE12",          38,   1,      0, false},
	{19,    REG13SPARES,     "REG13SPARES",         39,   1,      1, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_14[CONFIG_REG_FEI4A_14_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     14, false},
	{ 4,    LVDSDRVIREF,     "LVDSDRVIREF",         24,   8,     75, false},
	{ 5,    BONNDAC,         "BONNDAC",             32,   8,    237, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_15[CONFIG_REG_FEI4A_15_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     15, false},
	{ 4,    PLLIBIAS,        "PLLIBIAS",            24,   8,     88, true},
	{ 5,    LVDSDRVVOS,      "LVDSDRVVOS",          32,   8,    105, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_16[CONFIG_REG_FEI4A_16_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     16, false},
	{ 4,    TEMPDENSIBIAS,   "TEMPDENSIBIAS",       24,   8,      0, false},
	{ 5,    PLLICP,          "PLLICP",              32,   8,     28, true}
};

static ConfDataStruct gloCnfRegItem_FEI4A_17[CONFIG_REG_FEI4A_17_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     17, false},
	{ 4,    DAC8SPARE1,      "DAC8SPARE1",          24,   8,    255, true},
	{ 5,    PLSRLDACRAMP,    "PLSRLDACRAMP",        32,   8,     64, true}
};

static ConfDataStruct gloCnfRegItem_FEI4A_18[CONFIG_REG_FEI4A_18_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     18, false},
	{ 4,    DAC8SPARE2,      "DAC8SPARE2",          24,   8,      0, true},
	{ 5,    PLSRVGOAMP,      "PLSRVGOAMP",          32,   8,    255, true}
};

static ConfDataStruct gloCnfRegItem_FEI4A_19[CONFIG_REG_FEI4A_19_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     19, false},
	{ 4,    PLSRDACBIAS,     "PLSRDACBIAS",         24,   8,     96, true},
	{ 5,    DAC8SPARE5,      "DAC8SPARE5",          32,   8,    127, true}
};

static ConfDataStruct gloCnfRegItem_FEI4A_20[CONFIG_REG_FEI4A_20_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     20, false},
	{ 4,    VTHIN_ALTCOARSE, "VTHIN_ALTCOARSE",     24,   8,      0, true},
	{ 5,    VTHIN_ALTFINE,   "VTHIN_ALTFINE",       32,   8,     83, true}
};

static ConfDataStruct gloCnfRegItem_FEI4A_21[CONFIG_REG_FEI4A_21_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     21, false},
	{ 4,    REG21SPARES,     "REG21SPARES",         24,   3,      0, false},
	{ 5,    HITLD_IN,        "HITLD_IN",            27,   1,      0, false},
	{ 6,    DINJ_OVERRIDE,   "DINJ_OVERRIDE",       28,   1,      0, false},
	{ 7,    DIGHITIN_SEL,    "DIGHITIN_SEL",        29,   1,      0, false},
	{ 8,    PLSRDAC,         "PLSRDAC",             30,  10,    400, true}
};

static ConfDataStruct gloCnfRegItem_FEI4A_22[CONFIG_REG_FEI4A_22_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     22, false},
	{ 4,    REG22SPARES2,    "REG22SPARES2",        24,   6,      0, false},
	{ 5,    COLPR_MODE,      "COLPR_MODE",          30,   2,      0, true},
	{ 6,    COLPR_ADDR,      "COLPR_ADDR",          32,   6,      1, true},
	{ 7,    REG22SPARES1,    "REG22SPARES1",        38,   2,      0, false}

};

static ConfDataStruct gloCnfRegItem_FEI4A_23[CONFIG_REG_FEI4A_23_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     23, false},

	{ 4,    KILLDC15,        "KILLDC15",            24,   1,      0, false},
	{ 5,    KILLDC14,        "KILLDC14",            25,   1,      0, false},
	{ 6,    KILLDC13,        "KILLDC13",            26,   1,      0, false},
	{ 7,    KILLDC12,        "KILLDC12",            27,   1,      0, false},
	{ 8,    KILLDC11,        "KILLDC11",            28,   1,      0, false},
	{ 9,    KILLDC10,        "KILLDC10",            29,   1,      0, false},
	{10,    KILLDC9,         "KILLDC9",             30,   1,      0, false},
	{11,    KILLDC8,         "KILLDC8",             31,   1,      0, false},
	{12,    KILLDC7,         "KILLDC7",             32,   1,      0, false},
	{13,    KILLDC6,         "KILLDC6",             33,   1,      0, false},
	{14,    KILLDC5,         "KILLDC5",             34,   1,      0, false},
	{15,    KILLDC4,         "KILLDC4",             35,   1,      0, false},
	{16,    KILLDC3,         "KILLDC3",             36,   1,      0, false},
	{17,    KILLDC2,         "KILLDC2",             37,   1,      0, false},
	{18,    KILLDC1,         "KILLDC1",             38,   1,      0, false},
	{19,    KILLDC0,         "KILLDC0",             39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_24[CONFIG_REG_FEI4A_24_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     24, false},
	{ 4,    KILLDC31,        "KILLDC31",            24,   1,      0, false},
	{ 5,    KILLDC30,        "KILLDC30",            25,   1,      0, false},
	{ 6,    KILLDC29,        "KILLDC29",            26,   1,      0, false},
	{ 7,    KILLDC28,        "KILLDC28",            27,   1,      0, false},
	{ 8,    KILLDC27,        "KILLDC27",            28,   1,      0, false},
	{ 9,    KILLDC26,        "KILLDC26",            29,   1,      0, false},
	{10,    KILLDC25,        "KILLDC25",            30,   1,      0, false},
	{11,    KILLDC24,        "KILLDC24",            31,   1,      0, false},
	{12,    KILLDC23,        "KILLDC23",            32,   1,      0, false},
	{13,    KILLDC22,        "KILLDC22",            33,   1,      0, false},
	{14,    KILLDC21,        "KILLDC21",            34,   1,      0, false},
	{15,    KILLDC20,        "KILLDC20",            35,   1,      0, false},
	{16,    KILLDC19,        "KILLDC19",            36,   1,      0, false},
	{17,    KILLDC18,        "KILLDC18",            37,   1,      0, false},
	{18,    KILLDC17,        "KILLDC17",            38,   1,      0, false},
	{19,    KILLDC16,        "KILLDC16",            39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_25[CONFIG_REG_FEI4A_25_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     25, false},
	{ 4,    CHIP_LATENCY,    "CHIP_LATENCY",        24,   8,    210, false},
	{ 5,    KILLDC39,        "KILLDC39",            32,   1,      0, false},
	{ 6,    KILLDC38,        "KILLDC38",            33,   1,      0, false},
	{ 7,    KILLDC37,        "KILLDC37",            34,   1,      0, false},
	{ 8,    KILLDC36,        "KILLDC36",            35,   1,      0, false},
	{ 9,    KILLDC35,        "KILLDC35",            36,   1,      0, false},
	{10,    KILLDC34,        "KILLDC34",            37,   1,      0, false},
	{11,    KILLDC33,        "KILLDC33",            38,   1,      0, false},
	{12,    KILLDC32,        "KILLDC32",            39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_26[CONFIG_REG_FEI4A_26_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     26, false},
	{ 4,    CMDCNT0_12,      "CMDCNT0_12",          24,  13,     11, false},
	{ 5,    STOPMODECNFG,    "STOPMODECNFG",        37,   1,      0, false},
	{ 6,    HITDISCCNFG,     "HITDISCCNFG",         38,   2,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_27[CONFIG_REG_FEI4A_27_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     27, false},
	{ 4,    ENPLL,           "ENPLL",               24,   1,      1, false},
	{ 5,    EFUSE_SENSE,     "EFUSE_SENSE",         25,   1,      0, false},
	{ 6,    STOP_CLK,        "STOP_CLK",            26,   1,      0, false},
	{ 7,    RD_ERRORS,       "RD_ERRORS",           27,   1,      0, false},
	{ 8,    RD_SKIPPED,      "RD_SKIPPED",          28,   1,      0, false},
	{ 9,    REG27SPARES,     "REG27SPARES",         29,   5,      0, false},
	{10,    GATEHITOR,       "GATEHITOR",           34,   1,      0, false},
	{11,    DIG_INJ,         "DIG_INJ",             35,   1,      0, false},
	{12,    SR_CLR,          "SR_CLR",              36,   1,      0, false},
	{13,    LATCH_EN,        "LATCH_EN",            37,   1,      0, false},
	{14,    FE_CLK_PULSE,    "FE_CLK_PULSE",        38,   1,      0, false},
	{15,    CMDCNT13,        "CMDCNT13",            39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_28[CONFIG_REG_FEI4A_28_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     28, false},
	{ 4,    LVDSDRVSET06,    "LVDSDRVSET06",        24,   1,      1, false},
	{ 5,    REG28SPARES,     "REG28SPARES",         25,   5,      1, false},
	{ 6,    EN_40M,          "EN_40M",              30,   1,      0, false},
	{ 7,    EN_80M,          "EN_80M",              31,   1,      0, false},
	{ 8,    CLK1_S0,         "CLK1_S0",             32,   1,      0, false},
	{ 9,    CLK1_S1,         "CLK1_S1",             33,   1,      0, false},
	{10,    CLK1_S2,         "CLK1_S2",             34,   1,      0, false},
	{11,    CLK0_S0,         "CLK0_S0",             35,   1,      0, false},
	{12,    CLK0_S1,         "CLK0_S1",             36,   1,      0, false},
	{13,    CLK0_S2,         "CLK0_S2",             37,   1,      1, false},
	{14,    EN_160M,         "EN_160M",             38,   1,      1, false},
	{15,    EN_320M,         "EN_320M",             39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_29[CONFIG_REG_FEI4A_29_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     29, false},
	{ 4,    REG29SPARES,     "REG29SPARES",         24,   2,      0, false},
	{ 5,    DISABLE8B10B,    "DISABLE8B10B",        26,   1,      0, false},
	{ 6,    CLK2OUTCFG,      "CLK2OUTCFG",          27,   1,      0, false},
	{ 7,    EMPTYRECORD,     "EMPTYRECORD",         28,   8,      0, false},
	{ 8,    REG29SPARE2,     "REG29SPARE2",         36,   1,      0, false},
	{ 9,    LVDSDRVEN,       "LVDSDRVEN",           37,   1,      1, false},
	{10,    LVDSDRVSET30,    "LVDSDRVSET30",        38,   1,      1, false},
	{11,    LVDSDRVSET12,    "LVDSDRVSET12",        39,   1,      1, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_31[CONFIG_REG_FEI4A_31_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     31, false},
	{ 4,    RISEUPTAO,       "RISEUPTAO",           24,   3,      6, false},
	{ 5,    PULSERPWR,       "PULSERPWR",           27,   1,      1, false},
	{ 6,    PULSERDELAY,     "PULSERDELAY",         28,   6,      2,  true},
	{ 7,    EXTDIGCALSW,     "EXTDIGCALSW",         34,   1,      0, false},
	{ 8,    EXTANCALSW,      "EXTANCALSW",          35,   1,      1, false},
	{ 9,    REG31SPARES,     "REG31SPARES",         36,   4,      0, false}
};

// EFUSE Register
static ConfDataStruct gloCnfRegItem_FEI4A_32[CONFIG_REG_FEI4A_32_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     32, false},
	{ 4,    EFUSEDC0,        "EFUSEDC0",            24,   1,      0, false},
	{ 5,    EFUSEDC1,        "EFUSEDC1",            25,   1,      0, false},
	{ 6,    EFUSEDC2,        "EFUSEDC2",            26,   1,      0, false},
	{ 7,    EFUSEDC3,        "EFUSEDC3",            27,   1,      0, false},
	{ 8,    EFUSEDC4,        "EFUSEDC4",            28,   1,      0, false},
	{ 9,    EFUSEDC5,        "EFUSEDC5",            29,   1,      0, false},
	{10,    EFUSEDC6,        "EFUSEDC6",            30,   1,      0, false},
	{11,    EFUSEDC7,        "EFUSEDC7",            31,   1,      0, false},
	{12,    EFUSEDC8,        "EFUSEDC8",            32,   1,      0, false},
	{13,    EFUSEDC9,        "EFUSEDC9",            33,   1,      0, false},
	{14,    EFUSEDC10,       "EFUSEDC10",           34,   1,      0, false},
	{15,    EFUSEDC11,       "EFUSEDC11",           35,   1,      0, false},
	{16,    EFUSEDC12,       "EFUSEDC12",           36,   1,      0, false},
	{17,    EFUSEDC13,       "EFUSEDC13",           37,   1,      0, false},
	{18,    EFUSEDC14,       "EFUSEDC14",           38,   1,      0, false},
	{19,    EFUSEDC15,       "EFUSEDC15",           39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_33[CONFIG_REG_FEI4A_33_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     33, false},
	{ 4,    EFUSEDC16,       "EFUSEDC16",           24,   1,      0, false},
	{ 5,    EFUSEDC17,       "EFUSEDC17",           25,   1,      0, false},
	{ 6,    EFUSEDC18,       "EFUSEDC18",           26,   1,      0, false},
	{ 7,    EFUSEDC19,       "EFUSEDC19",           27,   1,      0, false},
	{ 8,    EFUSEDC20,       "EFUSEDC20",           28,   1,      0, false},
	{ 9,    EFUSEDC21,       "EFUSEDC21",           29,   1,      0, false},
	{10,    EFUSEDC22,       "EFUSEDC22",           30,   1,      0, false},
	{11,    EFUSEDC23,       "EFUSEDC23",           31,   1,      0, false},
	{12,    EFUSEDC24,       "EFUSEDC24",           32,   1,      0, false},
	{13,    EFUSEDC25,       "EFUSEDC25",           33,   1,      0, false},
	{14,    EFUSEDC26,       "EFUSEDC26",           34,   1,      0, false},
	{15,    EFUSEDC27,       "EFUSEDC27",           35,   1,      0, false},
	{16,    EFUSEDC28,       "EFUSEDC28",           36,   1,      0, false},
	{17,    EFUSEDC29,       "EFUSEDC29",           37,   1,      0, false},
	{18,    EFUSEDC30,       "EFUSEDC30",           38,   1,      0, false},
	{19,    EFUSEDC31,       "EFUSEDC31",           39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_34[CONFIG_REG_FEI4A_34_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     34, false},
	{ 4,    EFUSEDC32,       "EFUSEDC32",           24,   1,      0, false},
	{ 5,    EFUSEDC33,       "EFUSEDC33",           25,   1,      0, false},
	{ 6,    EFUSEDC34,       "EFUSEDC34",           26,   1,      0, false},
	{ 7,    EFUSEDC35,       "EFUSEDC35",	        27,   1,      0, false},
	{ 8,    EFUSEDC36,       "EFUSEDC36",           28,   1,      0, false},
	{ 9,    EFUSEDC37,       "EFUSEDC37",           29,   1,      0, false},
	{10,    EFUSEDC38,       "EFUSEDC38",           30,   1,      0, false},
	{11,    EFUSEDC39,       "EFUSEDC39",           31,   1,      0, false},
	{12,    EFUSEVREF,       "EFUSEVREF",           32,   4,      0, false},
	{13,    EFUSECREF,       "EFUSECREF",           36,   4,     14, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_35[CONFIG_REG_FEI4A_35_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     35, false},
	{ 4,    EFUSECHIPSERNUM, "EFUSECHIPSERNUM",     24,  16,      0, false}
};

// read-only register
static ConfDataStruct gloCnfRegItem_FEI4A_40[CONFIG_REG_FEI4A_40_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     40, false},
	{ 4,    REG40SPARES,     "REG40SPARES",         24,  16,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_41[CONFIG_REG_FEI4A_41_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     41, false},
	{ 4,    EOCHLSKIPPED,    "EOCHLSKIPPED",        24,   8,      0, false},
	{ 5,    REG41SPARES,     "REG41SPARES",         32,   8,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4A_42[CONFIG_REG_FEI4A_42_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    COMMAND,         "COMMAND",              1,  13,      0, false},
	{ 2,    CHIPADDRESS,     "CHIPADDRESS",         14,   4,      0, false},
	{ 3,    REGADDRESS,      "REGADDRESS",          18,   6,     42, false},
	{ 4,    READCMDERR,      "READCMDERR",          24,  16,      0, false}
};

// **************** 42 global registers for FE-I4B ***********************
static ConfDataStruct gloCnfRegItem_FEI4B_1[CONFIG_REG_FEI4B_1_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,      1, false},
	{ 4,    B_REG1SPARE,       "B_REG1SPARE",           24,   7,      0, false},
	{ 5,    B_SMALLHITERASE,   "B_SMALLHITERASE",       31,   1,      0, false},
	{ 6,    B_EVENTLIMIT,      "B_EVENTLIMIT",          32,   8,      0, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_2[CONFIG_REG_FEI4B_2_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,      2, false},
	{ 4,    B_TRIGCNT,         "B_TRIGCNT",             24,   4,      0, false},
	{ 5,    B_CONFADDRENABLE,  "B_CONFADDRENABLE",      28,   1,      1, false},
	{ 6,    B_CFGSPARE2,       "B_CFGSRARE2",           29,  11,      0, false}	
};

static ConfDataStruct gloCnfRegItem_FEI4B_3[CONFIG_REG_FEI4B_3_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,      3, false},
	{ 4,    B_ERRMASK0,        "B_ERRMASK0",            24,   16,     0, false}	
};

static ConfDataStruct gloCnfRegItem_FEI4B_4[CONFIG_REG_FEI4B_4_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,      4, false},
	{ 4,    B_ERRMASK1,        "B_ERRMASK1",            24,  16,      0, false}
	
};

static ConfDataStruct gloCnfRegItem_FEI4B_5[CONFIG_REG_FEI4B_5_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,      5, false},
	{ 4,    B_PRMPVBP_R,       "B_PRMPVBP_R",           24,   8,     43, true},
	{ 5,    B_BUFVGOPAMP,      "B_BUFVGOPAMP",          32,   8,    255, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_6[CONFIG_REG_FEI4B_6_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,      6, false},
	{ 4,    B_REG6SPARE,       "B_REG6SPARE",           24,   8,      0, false},
	{ 5,    B_PRMPVBP,         "B_PRMPVBP",             32,   8,     43, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_7[CONFIG_REG_FEI4B_7_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,      7, false},
	{ 4,    B_TDACVBP,         "B_TDACVBP",             24,   8,    255, true},
	{ 5,    B_DISVBN,          "B_DISVBN",              32,   8,     26, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_8[CONFIG_REG_FEI4B_8_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,      8, false},
	{ 4,    B_AMP2VBN,         "B_AMP2VBN",             24,   8,     79, true},
	{ 5,    B_AMP2VBPFOL,      "B_AMP2VBPFOL",          32,   8,     26, true}
	
};

static ConfDataStruct gloCnfRegItem_FEI4B_9[CONFIG_REG_FEI4B_9_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,      9, false},
	{ 4,    B_REG9SPARE,       "B_REG9SPARE",           24,   8,      0, false},
	{ 5,    B_AMP2VBP,         "B_AMP2VBP",             32,   8,     85, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_10[CONFIG_REG_FEI4B_10_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     10, false},
	{ 4,    B_FDACVBN,         "B_FDACVBN",             24,   8,     20, true},
	{ 5,    B_AMP2VBPFF,       "B_AMP2VBPFF",           32,   8,     13, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_11[CONFIG_REG_FEI4B_11_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     11, false},
	{ 4,    B_PRMPVBNFOL,      "B_PRMPVBNFOL",          24,   8,    106, true},
	{ 5,    B_PRMPVBP_L,       "B_PRMPVBP_L",           32,   8,     43, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_12[CONFIG_REG_FEI4B_12_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     12, false},
	{ 4,    B_PRMPVBPF,        "B_PRMPVBPF",            24,   8,    150, true},
	{ 5,    B_PRMPVBNLCC,      "B_PRMPVBNLCC",          32,   8,      0, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_13[CONFIG_REG_FEI4B_13_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     13, false},
	{ 4,    B_PXSTROBES1,      "B_PXSTROBES1",          24,   1,      0, false},
	{ 5,    B_PXSTROBES0,      "B_PXSTROBES0",          25,   1,      0, false},
	{ 6,    B_PXSTROBE0,       "B_PXSTROBE0",           26,   1,      0, false},
	{ 7,    B_PXSTROBE1,       "B_PXSTROBE1",           27,   1,      0, false},
	{ 8,    B_PXSTROBE2,       "B_PXSTROBE2",           28,   1,      0, false},
	{ 9,    B_PXSTROBE3,       "B_PXSTROBE3",           29,   1,      0, false},
	{10,    B_PXSTROBE4,       "B_PXSTROBE4",           30,   1,      0, false},
	{11,    B_PXSTROBE5,       "B_PXSTROBE5",           31,   1,      0, false},
	{12,    B_PXSTROBE6,       "B_PXSTROBE6",           32,   1,      0, false},
	{13,    B_PXSTROBE7,       "B_PXSTROBE7",           33,   1,      0, false},
	{14,    B_PXSTROBE8,       "B_PXSTROBE8",           34,   1,      0, false},
	{15,    B_PXSTROBE9,       "B_PXSTROBE9",           35,   1,      0, false},
	{16,    B_PXSTROBE10,      "B_PXSTROBE10",          36,   1,      0, false},
	{17,    B_PXSTROBE11,      "B_PXSTROBE11",          37,   1,      0, false},
	{18,    B_PXSTROBE12,      "B_PXSTROBE12",          38,   1,      0, false},
	{19,    B_REG13SPARES,     "B_REG13SPARES",         39,   1,      1, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_14[CONFIG_REG_FEI4B_14_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     14, false},
	{ 4,    B_LVDSDRVIREF,     "B_LVDSDRVIREF",         24,   8,     75, true},
	{ 5,    B_ADCOPAMP,        "B_ADCOPAMP",            32,   8,    100, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_15[CONFIG_REG_FEI4B_15_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     15, false},
	{ 4,    B_PLLIBIAS,        "B_PLLIBIAS",            24,   8,     88, true},
	{ 5,    B_LVDSDRVVOS,      "B_LVDSDRVVOS",          32,   8,    105, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_16[CONFIG_REG_FEI4B_16_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     16, false},
	{ 4,    B_TEMPDENSIBIAS,   "B_TEMPDENSIBIAS",       24,   8,      0, true},
	{ 5,    B_PLLICP,          "B_PLLICP",              32,   8,     28, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_17[CONFIG_REG_FEI4B_17_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     17, false},
	{ 4,    B_DAC8SPARE1,      "B_DAC8SPARE1",          24,   8,    255, true},
	{ 5,    B_PLSRLDACRAMP,    "B_PLSRLDACRAMP",        32,   8,     64, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_18[CONFIG_REG_FEI4B_18_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     18, false},
	{ 4,    B_VREFDIGTUNE,     "B_VREFDIGTUNE",         24,   8,      0, true},
	{ 5,    B_PLSRVGOAMP,      "B_PLSRVGOAMP",          32,   8,    255, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_19[CONFIG_REG_FEI4B_19_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     19, false},
	{ 4,    B_PLSRDACBIAS,     "B_PLSRDACBIAS",         24,   8,     96, true},
	{ 5,    B_VREFANTUNE,      "B_VREFANTUNE",          32,   8,      0, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_20[CONFIG_REG_FEI4B_20_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     20, false},
	{ 4,    B_VTHIN_ALTCOARSE, "B_VTHIN_ALTCOARSE",     24,   8,      0, true},
	{ 5,    B_VTHIN_ALTFINE,   "B_VTHIN_ALTFINE",       32,   8,     83, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_21[CONFIG_REG_FEI4B_21_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     21, false},
	{ 4,    B_REG21SPARES,     "B_REG21SPARES",         24,   3,      0, false},
	{ 5,    B_HITLD_IN,        "B_HITLD_IN",            27,   1,      0, false},
	{ 6,    B_DINJ_OVERRIDE,   "B_DINJ_OVERRIDE",       28,   1,      0, false},
	{ 7,    B_DIGHITIN_SEL,    "B_DIGHITIN_SEL",        29,   1,      0, false},
	{ 8,    B_PLSRDAC,         "B_PLSRDAC",             30,  10,      0, true}
};

static ConfDataStruct gloCnfRegItem_FEI4B_22[CONFIG_REG_FEI4B_22_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     22, false},
	{ 4,    B_REG22SPARES2,    "B_REG22SPARES2",        24,   6,      0, false},
	{ 5,    B_COLPR_MODE,      "B_COLPR_MODE",          30,   2,      0, true},
	{ 6,    B_COLPR_ADDR,      "B_COLPR_ADDR",          32,   6,      1, true},
	{ 7,    B_REG22SPARES1,    "B_REG22SPARES1",        38,   2,      0, false}

};

static ConfDataStruct gloCnfRegItem_FEI4B_23[CONFIG_REG_FEI4B_23_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     23, false},
	{ 4,    B_KILLDC15,        "B_KILLDC15",            24,   1,      0, false},
	{ 5,    B_KILLDC14,        "B_KILLDC14",            25,   1,      0, false},
	{ 6,    B_KILLDC13,        "B_KILLDC13",            26,   1,      0, false},
	{ 7,    B_KILLDC12,        "B_KILLDC12",            27,   1,      0, false},
	{ 8,    B_KILLDC11,        "B_KILLDC11",            28,   1,      0, false},
	{ 9,    B_KILLDC10,        "B_KILLDC10",            29,   1,      0, false},
	{10,    B_KILLDC9,         "B_KILLDC9",             30,   1,      0, false},
	{11,    B_KILLDC8,         "B_KILLDC8",             31,   1,      0, false},
	{12,    B_KILLDC7,         "B_KILLDC7",             32,   1,      0, false},
	{13,    B_KILLDC6,         "B_KILLDC6",             33,   1,      0, false},
	{14,    B_KILLDC5,         "B_KILLDC5",             34,   1,      0, false},
	{15,    B_KILLDC4,         "B_KILLDC4",             35,   1,      0, false},
	{16,    B_KILLDC3,         "B_KILLDC3",             36,   1,      0, false},
	{17,    B_KILLDC2,         "B_KILLDC2",             37,   1,      0, false},
	{18,    B_KILLDC1,         "B_KILLDC1",             38,   1,      0, false},
	{19,    B_KILLDC0,         "B_KILLDC0",             39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_24[CONFIG_REG_FEI4B_24_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     24, false},
	{ 4,    B_KILLDC31,        "B_KILLDC31",            24,   1,      0, false},
	{ 5,    B_KILLDC30,        "B_KILLDC30",            25,   1,      0, false},
	{ 6,    B_KILLDC29,        "B_KILLDC29",            26,   1,      0, false},
	{ 7,    B_KILLDC28,        "B_KILLDC28",            27,   1,      0, false},
	{ 8,    B_KILLDC27,        "B_KILLDC27",            28,   1,      0, false},
	{ 9,    B_KILLDC26,        "B_KILLDC26",            29,   1,      0, false},
	{10,    B_KILLDC25,        "B_KILLDC25",            30,   1,      0, false},
	{11,    B_KILLDC24,        "B_KILLDC24",            31,   1,      0, false},
	{12,    B_KILLDC23,        "B_KILLDC23",            32,   1,      0, false},
	{13,    B_KILLDC22,        "B_KILLDC22",            33,   1,      0, false},
	{14,    B_KILLDC21,        "B_KILLDC21",            34,   1,      0, false},
	{15,    B_KILLDC20,        "B_KILLDC20",            35,   1,      0, false},
	{16,    B_KILLDC19,        "B_KILLDC19",            36,   1,      0, false},
	{17,    B_KILLDC18,        "B_KILLDC18",            37,   1,      0, false},
	{18,    B_KILLDC17,        "B_KILLDC17",            38,   1,      0, false},
	{19,    B_KILLDC16,        "B_KILLDC16",            39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_25[CONFIG_REG_FEI4B_25_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     25, false},
	{ 4,    B_CHIP_LATENCY,    "B_CHIP_LATENCY",        24,   8,    210, false},
	{ 5,    B_KILLDC39,        "B_KILLDC39",            32,   1,      0, false},
	{ 6,    B_KILLDC38,        "B_KILLDC38",            33,   1,      0, false},
	{ 7,    B_KILLDC37,        "B_KILLDC37",            34,   1,      0, false},
	{ 8,    B_KILLDC36,        "B_KILLDC36",            35,   1,      0, false},
	{ 9,    B_KILLDC35,        "B_KILLDC35",            36,   1,      0, false},
	{10,    B_KILLDC34,        "B_KILLDC34",            37,   1,      0, false},
	{11,    B_KILLDC33,        "B_KILLDC33",            38,   1,      0, false},
	{12,    B_KILLDC32,        "B_KILLDC32",            39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_26[CONFIG_REG_FEI4B_26_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     26, false},
	{ 4,    B_CMDCNT0_12,      "B_CMDCNT0_12",          24,  13,     11, false},
	{ 5,    B_STOPMODECNFG,    "B_STOPMODECNFG",        37,   1,      0, false},
	{ 6,    B_HITDISCCNFG,     "B_HITDISCCNFG",         38,   2,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_27[CONFIG_REG_FEI4B_27_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     27, false},
	{ 4,    B_ENPLL,           "B_ENPLL",               24,   1,      1, false},
	{ 5,    B_EFUSE_SENSE,     "B_EFUSE_SENSE",         25,   1,      0, false},
	{ 6,    B_STOP_CLK,        "B_STOP_CLK",            26,   1,      0, false},
	{ 7,    B_RD_ERRORS,       "B_RD_ERRORS",           27,   1,      0, false},
	{ 8,    B_REG27SPARE1,     "B_REG27SPARE1",         28,   1,      0, false},
	{ 9,    B_ADC_EN_PULSE,    "B_ADC_EN_PULSE",        29,   1,      0, false},
	{10,    B_SR_RD_EN,        "B_SR_RD_EN",            30,   1,      0, false},
	{11,    B_REG27SPARES2,    "B_REG27SPARES2",        31,   3,      0, false},
	{12,    B_GATEHITOR,       "B_GATEHITOR",           34,   1,      0, false},
	{13,    B_DIG_INJ,         "B_DIG_INJ",             35,   1,      0, false},
	{14,    B_SR_CLR,          "B_SR_CLR",              36,   1,      0, false},
	{15,    B_LATCH_EN,        "B_LATCH_EN",            37,   1,      0, false},
	{16,    B_FE_CLK_PULSE,    "B_FE_CLK_PULSE",        38,   1,      0, false},
	{17,    B_CMDCNT13,        "B_CMDCNT13",            39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_28[CONFIG_REG_FEI4B_28_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     28, false},
	{ 4,    B_LVDSDRVSET06,    "B_LVDSDRVSET06",        24,   1,      1, false},
	{ 5,    B_REG28SPARES,     "B_REG28SPARES",         25,   5,      1, false},
	{ 6,    B_EN_40M,          "B_EN_40M",              30,   1,      0, false},
	{ 7,    B_EN_80M,          "B_EN_80M",              31,   1,      0, false},
	{ 8,    B_CLK1_S0,         "B_CLK1_S0",             32,   1,      0, false},
	{ 9,    B_CLK1_S1,         "B_CLK1_S1",             33,   1,      0, false},
	{10,    B_CLK1_S2,         "B_CLK1_S2",             34,   1,      0, false},
	{11,    B_CLK0_S0,         "B_CLK0_S0",             35,   1,      0, false},
	{12,    B_CLK0_S1,         "B_CLK0_S1",             36,   1,      0, false},
	{13,    B_CLK0_S2,         "B_CLK0_S2",             37,   1,      1, false},
	{14,    B_EN_160M,         "B_EN_160M",             38,   1,      1, false},
	{15,    B_EN_320M,         "B_EN_320M",             39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_29[CONFIG_REG_FEI4B_29_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     29, false},
	{ 4,    B_REG29SPARES,     "B_REG29SPARES",         24,   2,      0, false},
	{ 5,    B_DISABLE8B10B,    "B_DISABLE8B10B",        26,   1,      0, false},
	{ 6,    B_CLK2OUTCFG,      "B_CLK2OUTCFG",          27,   1,      0, false},
	{ 7,    B_EMPTYRECORD,     "B_EMPTYRECORD",         28,   8,      0, false},
	{ 8,    B_REG29SPARE2,     "B_REG29SPARE2",         36,   1,      0, false},
	{ 9,    B_LVDSDRVEN,       "B_LVDSDRVEN",           37,   1,      1, false},
	{10,    B_LVDSDRVSET30,    "B_LVDSDRVSET30",        38,   1,      1, false},
	{11,    B_LVDSDRVSET12,    "B_LVDSDRVSET12",        39,   1,      1, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_30[CONFIG_REG_FEI4B_30_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     30, false},
	{ 4,    B_TMPSENSED0,      "B_TMPSENSED0",          24,   1,      0, false},
	{ 5,    B_TMPSENSED1,      "B_TMPSENSED1",          25,   1,      0, false},
	{ 6,    B_TMPSENSEDISABLE, "B_TMPSENSEDISABLE",     26,   1,      0, false},
	{ 7,    B_ILEAKRANGE,      "B_ILEAKRANGE",          27,   1,      0, false},
	{ 8,    B_REG30SPARES,     "B_REG30SPARES",         28,  12,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_31[CONFIG_REG_FEI4B_31_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     31, false},
	{ 4,    B_RISEUPTAO,       "B_RISEUPTAO",           24,   3,      6, false},
	{ 5,    B_PULSERPWR,       "B_PULSERPWR",           27,   1,      1, false},
	{ 6,    B_PULSERDELAY,     "B_PULSERDELAY",         28,   6,      2, true},
	{ 7,    B_EXTDIGCALSW,     "B_EXTDIGCALSW",         34,   1,      0, false},
	{ 8,    B_EXTANCALSW,      "B_EXTANCALSW",          35,   1,      1, false},
	{ 9,    B_REG31SPARES,     "B_REG31SPARES",         36,   1,      0, false},
	{10,    B_ADCSELECT,       "B_ADCSELECT",           37,   3,      0, false}
};

// EFUSE Register
static ConfDataStruct gloCnfRegItem_FEI4B_32[CONFIG_REG_FEI4B_32_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     32, false},
	{ 4,    B_EFUSEDC0,        "B_EFUSEDC0",            24,   1,      0, false},
	{ 5,    B_EFUSEDC1,        "B_EFUSEDC1",            25,   1,      0, false},
	{ 6,    B_EFUSEDC2,        "B_EFUSEDC2",            26,   1,      0, false},
	{ 7,    B_EFUSEDC3,        "B_EFUSEDC3",            27,   1,      0, false},
	{ 8,    B_EFUSEDC4,        "B_EFUSEDC4",            28,   1,      0, false},
	{ 9,    B_EFUSEDC5,        "B_EFUSEDC5",            29,   1,      0, false},
	{10,    B_EFUSEDC6,        "B_EFUSEDC6",            30,   1,      0, false},
	{11,    B_EFUSEDC7,        "B_EFUSEDC7",            31,   1,      0, false},
	{12,    B_EFUSEDC8,        "B_EFUSEDC8",            32,   1,      0, false},
	{13,    B_EFUSEDC9,        "B_EFUSEDC9",            33,   1,      0, false},
	{14,    B_EFUSEDC10,       "B_EFUSEDC10",           34,   1,      0, false},
	{15,    B_EFUSEDC11,       "B_EFUSEDC11",           35,   1,      0, false},
	{16,    B_EFUSEDC12,       "B_EFUSEDC12",           36,   1,      0, false},
	{17,    B_EFUSEDC13,       "B_EFUSEDC13",           37,   1,      0, false},
	{18,    B_EFUSEDC14,       "B_EFUSEDC14",           38,   1,      0, false},
	{19,    B_EFUSEDC15,       "B_EFUSEDC15",           39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_33[CONFIG_REG_FEI4B_33_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     33, false},
	{ 4,    B_EFUSEDC16,       "B_EFUSEDC16",           24,   1,      0, false},
	{ 5,    B_EFUSEDC17,       "B_EFUSEDC17",           25,   1,      0, false},
	{ 6,    B_EFUSEDC18,       "B_EFUSEDC18",           26,   1,      0, false},
	{ 7,    B_EFUSEDC19,       "B_EFUSEDC19",           27,   1,      0, false},
	{ 8,    B_EFUSEDC20,       "B_EFUSEDC20",           28,   1,      0, false},
	{ 9,    B_EFUSEDC21,       "B_EFUSEDC21",           29,   1,      0, false},
	{10,    B_EFUSEDC22,       "B_EFUSEDC22",           30,   1,      0, false},
	{11,    B_EFUSEDC23,       "B_EFUSEDC23",           31,   1,      0, false},
	{12,    B_EFUSEDC24,       "B_EFUSEDC24",           32,   1,      0, false},
	{13,    B_EFUSEDC25,       "B_EFUSEDC25",           33,   1,      0, false},
	{14,    B_EFUSEDC26,       "B_EFUSEDC26",           34,   1,      0, false},
	{15,    B_EFUSEDC27,       "B_EFUSEDC27",           35,   1,      0, false},
	{16,    B_EFUSEDC28,       "B_EFUSEDC28",           36,   1,      0, false},
	{17,    B_EFUSEDC29,       "B_EFUSEDC29",           37,   1,      0, false},
	{18,    B_EFUSEDC30,       "B_EFUSEDC30",           38,   1,      0, false},
	{19,    B_EFUSEDC31,       "B_EFUSEDC31",           39,   1,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_34[CONFIG_REG_FEI4B_34_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     34, false},
	{ 4,    B_EFUSEDC32,       "B_EFUSEDC32",           24,   1,      0, false},
	{ 5,    B_EFUSEDC33,       "B_EFUSEDC33",           25,   1,      0, false},
	{ 6,    B_EFUSEDC34,       "B_EFUSEDC34",           26,   1,      0, false},
	{ 7,    B_EFUSEDC35,       "B_EFUSEDC35",	        27,   1,      0, false},
	{ 8,    B_EFUSEDC36,       "B_EFUSEDC36",           28,   1,      0, false},
	{ 9,    B_EFUSEDC37,       "B_EFUSEDC37",           29,   1,      0, false},
	{10,    B_EFUSEDC38,       "B_EFUSEDC38",           30,   1,      0, false},
	{11,    B_EFUSEDC39,       "B_EFUSEDC39",           31,   1,      0, false},
	{12,    B_REG34SPARES1,    "B_REG34SPARES1",        32,   3,      0, false},
	{13,    B_PRMPVBPMSNEN,    "B_PRMPVBPMSNEN",        35,   1,      0, false},
	{14,    B_REG34SPARES2,    "B_REG34SPARES2",        36,   4,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_35[CONFIG_REG_FEI4B_35_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     35, false},
	{ 4,    B_EFUSECHIPSERNUM, "B_EFUSECHIPSERNUM",     24,  16,      0, false}
};

// read-only register
static ConfDataStruct gloCnfRegItem_FEI4B_40[CONFIG_REG_FEI4B_40_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     40, false},
	{ 4,    B_REG40SPARES,     "B_REG40SPARES",         24,   2,      0, false},
	{ 5,    B_GADCOUT,         "B_GADCOUT",             26,  10,      0,  false},
	{ 6,    B_GADCSTATUS,      "B_GADCSTATUS",         36,   1,      0, false},
	{ 7,    B_GADCSELECTRB,     "B_GADCSELECTRB",        37,   3,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_41[CONFIG_REG_FEI4B_41_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     41, false},
	{ 4,    B_EOCHLSKIPPED,    "B_EOCHLSKIPPED",        24,   8,      0, false},
	{ 5,    B_REG41SPARES,     "B_REG41SPARES",         32,   8,      0, false}
};

static ConfDataStruct gloCnfRegItem_FEI4B_42[CONFIG_REG_FEI4B_42_ITEMS] = {
	{ 0,    B_IDLE,            "B_IDLE",                 0,   1,      0, false},
	{ 1,    B_COMMAND,         "B_COMMAND",              1,  13,      0, false},
	{ 2,    B_CHIPADDRESS,     "B_CHIPADDRESS",         14,   4,      0, false},
	{ 3,    B_REGADDRESS,      "B_REGADDRESS",          18,   6,     42, false},
	{ 4,    B_READCMDERR,      "B_READCMDERR",          24,  16,      0, false}
};

// *************** global shift registers for FE-I4A (bypass mode) ************************
static ConfDataStruct shiftCnfRegItemAB_A[SHIFT_REG_AB_A_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    PXSTROBES1,      "PXSTROBES1",           1,   1,      0, false},
	{ 2,    PXSTROBES0,      "PXSTROBES0",           2,   1,      0, false},
	{ 3,    PXSTROBE0,       "PXSTROBE0",            3,   1,      0, false},
	{ 4,    PXSTROBE1,       "PXSTROBE1",            4,   1,      0, false},
	{ 5,    PXSTROBE2,       "PXSTROBE2",            5,   1,      0, false},
	{ 6,    PXSTROBE3,       "PXSTROBE3",            6,   1,      0, false},
	{ 7,    PXSTROBE4,       "PXSTROBE4",            7,   1,      0, false},
	{ 8,    PXSTROBE5,       "PXSTROBE5",            8,   1,      0, false},
	{ 9,    PXSTROBE6,       "PXSTROBE6",            9,   1,      0, false},
	{10,    PXSTROBE7,       "PXSTROBE7",           10,   1,      0, false},
	{11,    PXSTROBE8,       "PXSTROBE8",           11,   1,      0, false},
	{12,    PXSTROBE9,       "PXSTROBE9",           12,   1,      0, false},
	{13,    PXSTROBE10,      "PXSTROBE10",          13,   1,      0, false},
	{15,    PXSTROBE11,      "PXSTROBE11",          14,   1,      0, false},
	{16,    PXSTROBE12,      "PXSTROBE12",          15,   1,      0, false},
	{17,    REG13SPARES,     "REG13SPARES",         16,   1,      0, false},
	{18,    PRMPVBPF,        "PRMPVBPF",            17,   8,     16, false},
	{19,    PRMPVBNLCC,      "PRMPVBNLCC",          25,   8,      0, false},
	{20,    PRMPVBNFOL,      "PRMPVBNFOL",          33,   8,     64, false},
	{21,    PRMPVBP_L,       "PRMPVBP_L",           41,   8,     66, false},
	{22,    FDACVBN,         "FDACVBN",             49,   8,     66, false},
	{23,    AMP2VBPFF,       "AMP2VBPFF",           57,   8,    175, false},
	{24,    PRMPVBP_T,       "PRMPVBP_T",           65,   8,     66, false},
	{25,    AMP2VBP,         "AMP2VBP",             73,   8,     64, false},
	{26,    AMP2VBN,         "AMP2VBN",             81,   8,     69, false},
	{27,    AMP2VBPFOL,      "AMP2VBPFOL",          89,   8,     26, false},
	{28,    TDACVBP,         "TDACVBP",             97,   8,      0, false},
	{29,    DISVBN,          "DISVBN",             105,   8,      0, false},
	{30,    DISVBN_CPPM,     "DISVBN_CPPM",        113,   8,     64, false},
	{31,    PRMPVBP,         "PRMPVBP",            121,   8,     32, false},
	{32,    PRMPVBP_R,       "PRMPVBP_R",          129,   8,     50, false},
	{33,    VTHIN,           "VTHIN",              137,   8,     64, false},
	{34,    DIGHITIN_SEL,    "DIGHITIN_SEL",       145,   1,      0, false},
	{35,    DINJ_OVERRIDE,   "DINJ_OVERRIDE",      146,   1,      0, false},
	{36,    HITLD_IN,        "HITLD_IN",           147,   1,      0, false},
	{37,    REG21SPARES,     "REG21SPARES",        148,   3,     64, false},
	{38,    REG22SPARES1,    "REG22SPARES1",       151,   2,      0, false},
	{39,    COLPR_ADDR,      "COLPR_ADDR",         153,   6,      0, false},
	{40,    COLPR_MODE,      "COLPR_MODE",         159,   2,      0, false},
	{41,    REG22SPARES2,    "REG22SPARES2",       161,   6,      0, false},
	{42,    PLSRDAC,         "PLSRDAC",            167,  10,      0, false},
	{43,    VTHIN_ALTCOARSE, "VTHIN_ALTCOARSE",    177,   8,      0, false},
	{44,    VTHIN_ALTFINE,   "VTHIN_ALTFINE",      185,   8,      0, false},
	{45,    DAC8SPARE5,      "DAC8SPARE5",         193,   8,      0, false},
	{46,    PLSRDACBIAS,     "PLSRDACBIAS",        201,   8,      0, false},
	{47,    PLSRVGOAMP,      "PLSRVGOAMP",         209,   8,      0, false},
	{48,    DAC8SPARE2,      "DAC8SPARE2",         217,   8,      0, false},
	{49,    DAC8SPARE1,      "DAC8SPARE1",         225,   8,      0, false},
	{50,    PLSRLDACRAMP,    "PLSRLDACRAMP",       233,   8,      0, false},
	{51,    TEMPDENSIBIAS,   "TEMPDENSIBIAS",      241,   8,      0, false},
	{52,    PLLICP,          "PLLICP",             249,   8,      0, false},
	{53,    PLLIBIAS,        "PLLIBIAS",           257,   8,      0, false},
	{54,    LVDSDRVVOS,      "LVDSDRVVOS",         265,   8,      0, false},
	{55,    LVDSDRVIREF,     "LVDSDRVIREF",        273,   8,      0, false},
	{56,    BONNDAC,         "BONNDAC",            281,   8,      0, false}
};

static ConfDataStruct shiftCnfRegItemC_A[SHIFT_REG_C_A_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    RISEUPTAO,       "RISEUPTAO",            1,   3,      0, false},
	{ 2,    PULSERPWR,       "PULSERPWR",            4,   1,      0, false},
	{ 3,    PULSERDELAY,     "PULSERDELAY",          5,   6,      0, false},
	{ 4,    EXTDIGCALSW,     "EXTDIGCALSW",         11,   1,      0, false},
	{ 5,    EXTANCALSW,      "EXTANCALSW",          12,   1,      0, false},
	{ 6,    REG31SPARES,     "REG31SPARES",         13,   4,      0, false},
	{ 7,    UNDEFINED,       "UNDEFINED",           17,  16,      0, false},
	{ 8,    REG29SPARES,     "REG29SPARES",         33,   2,      0, false},
	{ 9,    DISABLE8B10B,    "DISABLE8B10B",        35,   1,      0, false},
	{10,    CLK2OUTCFG,      "CLK2OUTCFG",          36,   1,      0, false},
	{11,    EMPTYRECORD,     "EMPTYRECORD",         37,   8,      0, false},
	{12,    REG29SPARE2,     "REG29SPARE2",         45,   1,      0, false},
	{13,    LVDSDRVEN,       "LVDSDRVEN",           46,   1,      0, false},
	{14,    LVDSDRVSET30,    "LVDSDRVSET30",        47,   1,      0, false},
	{15,    LVDSDRVSET12,    "LVDSDRVSET12",        48,   1,      0, false},
	{16,    LVDSDRVSET06,    "LVDSDRVSET06",        49,   1,      0, false},
	{17,    REG28SPARES,     "REG28SPARES",         50,   5,      0, false},
	{18,    EN_40M,          "EN_40M",              55,   1,      0, false},
	{19,    EN_80M,          "EN_80M",              56,   1,      0, false},
	{20,    CLK1_S0,         "CLK1_S0",             57,   1,      0, false},
	{21,    CLK1_S1,         "CLK1_S1",             58,   1,      0, false},
	{22,    CLK1_S2,         "CLK1_S2",             59,   1,      0, false},
	{23,    CLK0_S0,         "CLK0_S0",             60,   1,      0, false},
	{24,    CLK0_S1,         "CLK0_S1",             61,   1,      0, false},
	{25,    CLK0_S2,         "CLK0_S2",             62,   1,      0, false},
	{26,    EN_160M,         "EN_160M",             63,   1,      0, false},
	{27,    EN_320M,         "EN_320M",             64,   1,      0, false},
	{28,    ENPLL,           "ENPLL",               65,   1,      0, false},
	{29,    FE_CLK_PULSE,    "FE_CLK_PULSE",        66,   1,      0, false},
	{30,    LATCH_EN,        "LATCH_EN",            67,   1,      0, false},
	{31,    SR_CLR,          "SR_CLR",              68,   1,      0, false},
	{32,    DIG_INJ,         "DIG_INJ",             69,   1,      0, false},	
	{33,    REG27SPARES,     "REG27SPARES",         70,   5,      0, false},
	{34,    GATEHITOR,       "GATEHITOR",           75,   1,      0, false},
	{35,    RD_SKIPPED,      "RD_SKIPPED",          76,   1,      0, false},
	{36,    RD_ERRORS,       "RD_ERRORS",           77,   1,      0, false},
	{37,    STOP_CLK,        "STOP_CLK",            78,   1,      0, false},
	{38,    EFUSE_SENSE,     "EFUSE_SENSE",         79,   1,      0, false},
	{39,    CMDCNT13,        "CMDCNT13",            80,   1,      0, false},
	{40,    CMDCNT0_12,      "CMDCNT0_12",          81,  13,      0, false},
	{41,    STOPMODECNFG,    "STOPMODECNFG",        94,   1,      0, false},
	{42,    HITDISCCNFG,     "HITDISCCNFG",         95,   2,      0, false},
	{43,    CHIP_LATENCY,    "CHIP_LATENCY",        97,   8,      0, false},
	{44,    KILLDC39,        "KILLDC39",           105,   1,      0, false},
	{45,    KILLDC38,        "KILLDC38",           106,   1,      0, false},
	{46,    KILLDC37,        "KILLDC37",           107,   1,      0, false},
	{47,    KILLDC36,        "KILLDC36",           108,   1,      0, false},
	{48,    KILLDC35,        "KILLDC35",           109,   1,      0, false},
	{49,    KILLDC34,        "KILLDC34",           110,   1,      0, false},
	{50,    KILLDC33,        "KILLDC33",           111,   1,      0, false},
	{51,    KILLDC32,        "KILLDC32",           112,   1,      0, false},
	{52,    KILLDC31,        "KILLDC31",           113,   1,      0, false},
	{53,    KILLDC30,        "KILLDC30",           114,   1,      0, false},
	{54,    KILLDC29,        "KILLDC29",           115,   1,      0, false},
	{55,    KILLDC28,        "KILLDC28",           116,   1,      0, false},
	{56,    KILLDC27,        "KILLDC27",           117,   1,      0, false},
	{57,    KILLDC26,        "KILLDC26",           118,   1,      0, false},
	{58,    KILLDC25,        "KILLDC25",           119,   1,      0, false},
	{59,    KILLDC24,        "KILLDC24",           120,   1,      0, false},
	{60,    KILLDC23,        "KILLDC23",           121,   1,      0, false},
	{61,    KILLDC22,        "KILLDC22",           122,   1,      0, false},
	{62,    KILLDC21,        "KILLDC21",           123,   1,      0, false},
	{63,    KILLDC20,        "KILLDC20",           124,   1,      0, false},
	{64,    KILLDC19,        "KILLDC19",           125,   1,      0, false},
	{65,    KILLDC18,        "KILLDC18",           126,   1,      0, false},
	{66,    KILLDC17,        "KILLDC17",           127,   1,      0, false},
	{67,    KILLDC16,        "KILLDC16",           128,   1,      0, false},
	{68,    KILLDC15,        "KILLDC15",           129,   1,      0, false},
	{69,    KILLDC14,        "KILLDC14",           130,   1,      0, false},
	{70,    KILLDC13,        "KILLDC13",           131,   1,      0, false},
	{71,    KILLDC12,        "KILLDC12",           132,   1,      0, false},
	{72,    KILLDC11,        "KILLDC11",           133,   1,      0, false},
	{73,    KILLDC10,        "KILLDC10",           134,   1,      0, false},
	{74,    KILLDC9,         "KILLDC9",            135,   1,      0, false},
	{75,    KILLDC8,         "KILLDC8",            136,   1,      0, false},
	{76,    KILLDC7,         "KILLDC7",            137,   1,      0, false},
	{77,    KILLDC6,         "KILLDC6",            138,   1,      0, false},
	{78,    KILLDC5,         "KILLDC5",            139,   1,      0, false},
	{79,    KILLDC4,         "KILLDC4",            140,   1,      0, false},
	{80,    KILLDC3,         "KILLDC3",            141,   1,      0, false},
	{81,    KILLDC2,         "KILLDC2",            142,   1,      0, false},
	{82,    KILLDC1,         "KILLDC1",            143,   1,      0, false},
	{83,    KILLDC0,         "KILLDC0",            144,   1,      0, false}
};

// *************** global shift registers for FE-I4A (bypass mode) ************************
static ConfDataStruct shiftCnfRegItemAB_B[SHIFT_REG_AB_B_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    PXSTROBES1,      "PXSTROBES1",           1,   1,      0, false},
	{ 2,    PXSTROBES0,      "PXSTROBES0",           2,   1,      0, false},
	{ 3,    PXSTROBE0,       "PXSTROBE0",            3,   1,      0, false},
	{ 4,    PXSTROBE1,       "PXSTROBE1",            4,   1,      0, false},
	{ 5,    PXSTROBE2,       "PXSTROBE2",            5,   1,      0, false},
	{ 6,    PXSTROBE3,       "PXSTROBE3",            6,   1,      0, false},
	{ 7,    PXSTROBE4,       "PXSTROBE4",            7,   1,      0, false},
	{ 8,    PXSTROBE5,       "PXSTROBE5",            8,   1,      0, false},
	{ 9,    PXSTROBE6,       "PXSTROBE6",            9,   1,      0, false},
	{10,    PXSTROBE7,       "PXSTROBE7",           10,   1,      0, false},
	{11,    PXSTROBE8,       "PXSTROBE8",           11,   1,      0, false},
	{12,    PXSTROBE9,       "PXSTROBE9",           12,   1,      0, false},
	{13,    PXSTROBE10,      "PXSTROBE10",          13,   1,      0, false},
	{15,    PXSTROBE11,      "PXSTROBE11",          14,   1,      0, false},
	{16,    PXSTROBE12,      "PXSTROBE12",          15,   1,      0, false},
	{17,    REG13SPARES,     "REG13SPARES",         16,   1,      0, false},
	{18,    PRMPVBPF,        "PRMPVBPF",            17,   8,     16, false},
	{19,    PRMPVBNLCC,      "PRMPVBNLCC",          25,   8,      0, false},
	{20,    PRMPVBNFOL,      "PRMPVBNFOL",          33,   8,     64, false},
	{21,    PRMPVBP_L,       "PRMPVBP_L",           41,   8,     66, false},
	{22,    FDACVBN,         "FDACVBN",             49,   8,     66, false},
	{23,    AMP2VBPFF,       "AMP2VBPFF",           57,   8,    175, false},
	{24,    PRMPVBP_T,       "PRMPVBP_T",           65,   8,     66, false},
	{25,    AMP2VBP,         "AMP2VBP",             73,   8,     64, false},
	{26,    AMP2VBN,         "AMP2VBN",             81,   8,     69, false},
	{27,    AMP2VBPFOL,      "AMP2VBPFOL",          89,   8,     26, false},
	{28,    TDACVBP,         "TDACVBP",             97,   8,      0, false},
	{29,    DISVBN,          "DISVBN",             105,   8,      0, false},
	{30,    DISVBN_CPPM,     "DISVBN_CPPM",        113,   8,     64, false},
	{31,    PRMPVBP,         "PRMPVBP",            121,   8,     32, false},
	{32,    PRMPVBP_R,       "PRMPVBP_R",          129,   8,     50, false},
	{33,    VTHIN,           "VTHIN",              137,   8,     64, false},
	{34,    DIGHITIN_SEL,    "DIGHITIN_SEL",       145,   1,      0, false},
	{35,    DINJ_OVERRIDE,   "DINJ_OVERRIDE",      146,   1,      0, false},
	{36,    HITLD_IN,        "HITLD_IN",           147,   1,      0, false},
	{37,    REG21SPARES,     "REG21SPARES",        148,   3,     64, false},
	{38,    REG22SPARES1,    "REG22SPARES1",       151,   2,      0, false},
	{39,    COLPR_ADDR,      "COLPR_ADDR",         153,   6,      0, false},
	{40,    COLPR_MODE,      "COLPR_MODE",         159,   2,      0, false},
	{41,    REG22SPARES2,    "REG22SPARES2",       161,   6,      0, false},
	{42,    PLSRDAC,         "PLSRDAC",            167,  10,      0, false},
	{43,    VTHIN_ALTCOARSE, "VTHIN_ALTCOARSE",    177,   8,      0, false},
	{44,    VTHIN_ALTFINE,   "VTHIN_ALTFINE",      185,   8,      0, false},
	{45,    DAC8SPARE5,      "DAC8SPARE5",         193,   8,      0, false},
	{46,    PLSRDACBIAS,     "PLSRDACBIAS",        201,   8,      0, false},
	{47,    PLSRVGOAMP,      "PLSRVGOAMP",         209,   8,      0, false},
	{48,    DAC8SPARE2,      "DAC8SPARE2",         217,   8,      0, false},
	{49,    DAC8SPARE1,      "DAC8SPARE1",         225,   8,      0, false},
	{50,    PLSRLDACRAMP,    "PLSRLDACRAMP",       233,   8,      0, false},
	{51,    TEMPDENSIBIAS,   "TEMPDENSIBIAS",      241,   8,      0, false},
	{52,    PLLICP,          "PLLICP",             249,   8,      0, false},
	{53,    PLLIBIAS,        "PLLIBIAS",           257,   8,      0, false},
	{54,    LVDSDRVVOS,      "LVDSDRVVOS",         265,   8,      0, false},
	{55,    LVDSDRVIREF,     "LVDSDRVIREF",        273,   8,      0, false},
	{56,    BONNDAC,         "BONNDAC",            281,   8,      0, false}
};

/* unused!
static ConfDataStruct shiftCnfRegItemC_B[SHIFT_REG_C_B_ITEMS] = {
	{ 0,    IDLE,            "IDLE",                 0,   1,      0, false},
	{ 1,    RISEUPTAO,       "RISEUPTAO",            1,   3,      0, false},
	{ 2,    PULSERPWR,       "PULSERPWR",            4,   1,      0, false},
	{ 3,    PULSERDELAY,     "PULSERDELAY",          5,   6,      0, false},
	{ 4,    EXTDIGCALSW,     "EXTDIGCALSW",         11,   1,      0, false},
	{ 5,    EXTANCALSW,      "EXTANCALSW",          12,   1,      0, false},
	{ 6,    REG31SPARES,     "REG31SPARES",         13,   4,      0, false},
	{ 7,    UNDEFINED,       "UNDEFINED",           17,  16,      0, false},
	{ 8,    REG29SPARES,     "REG29SPARES",         33,   2,      0, false},
	{ 9,    DISABLE8B10B,    "DISABLE8B10B",        35,   1,      0, false},
	{10,    CLK2OUTCFG,      "CLK2OUTCFG",          36,   1,      0, false},
	{11,    EMPTYRECORD,     "EMPTYRECORD",         37,   8,      0, false},
	{12,    REG29SPARE2,     "REG29SPARE2",         45,   1,      0, false},
	{13,    LVDSDRVEN,       "LVDSDRVEN",           46,   1,      0, false},
	{14,    LVDSDRVSET30,    "LVDSDRVSET30",        47,   1,      0, false},
	{15,    LVDSDRVSET12,    "LVDSDRVSET12",        48,   1,      0, false},
	{16,    LVDSDRVSET06,    "LVDSDRVSET06",        49,   1,      0, false},
	{17,    REG28SPARES,     "REG28SPARES",         50,   5,      0, false},
	{18,    EN_40M,          "EN_40M",              55,   1,      0, false},
	{19,    EN_80M,          "EN_80M",              56,   1,      0, false},
	{20,    CLK1_S0,         "CLK1_S0",             57,   1,      0, false},
	{21,    CLK1_S1,         "CLK1_S1",             58,   1,      0, false},
	{22,    CLK1_S2,         "CLK1_S2",             59,   1,      0, false},
	{23,    CLK0_S0,         "CLK0_S0",             60,   1,      0, false},
	{24,    CLK0_S1,         "CLK0_S1",             61,   1,      0, false},
	{25,    CLK0_S2,         "CLK0_S2",             62,   1,      0, false},
	{26,    EN_160M,         "EN_160M",             63,   1,      0, false},
	{27,    EN_320M,         "EN_320M",             64,   1,      0, false},
	{28,    ENPLL,           "ENPLL",               65,   1,      0, false},
	{29,    FE_CLK_PULSE,    "FE_CLK_PULSE",        66,   1,      0, false},
	{30,    LATCH_EN,        "LATCH_EN",            67,   1,      0, false},
	{31,    SR_CLR,          "SR_CLR",              68,   1,      0, false},
	{32,    DIG_INJ,         "DIG_INJ",             69,   1,      0, false},	
	{33,    REG27SPARES,     "REG27SPARES",         70,   5,      0, false},
	{34,    GATEHITOR,       "GATEHITOR",           75,   1,      0, false},
	{35,    RD_SKIPPED,      "RD_SKIPPED",          76,   1,      0, false},
	{36,    RD_ERRORS,       "RD_ERRORS",           77,   1,      0, false},
	{37,    STOP_CLK,        "STOP_CLK",            78,   1,      0, false},
	{38,    EFUSE_SENSE,     "EFUSE_SENSE",         79,   1,      0, false},
	{39,    CMDCNT13,        "CMDCNT13",            80,   1,      0, false},
	{40,    CMDCNT0_12,      "CMDCNT0_12",          81,  13,      0, false},
	{41,    STOPMODECNFG,    "STOPMODECNFG",        94,   1,      0, false},
	{42,    HITDISCCNFG,     "HITDISCCNFG",         95,   2,      0, false},
	{43,    CHIP_LATENCY,    "CHIP_LATENCY",        97,   8,      0, false},
	{44,    KILLDC39,        "KILLDC39",           105,   1,      0, false},
	{45,    KILLDC38,        "KILLDC38",           106,   1,      0, false},
	{46,    KILLDC37,        "KILLDC37",           107,   1,      0, false},
	{47,    KILLDC36,        "KILLDC36",           108,   1,      0, false},
	{48,    KILLDC35,        "KILLDC35",           109,   1,      0, false},
	{49,    KILLDC34,        "KILLDC34",           110,   1,      0, false},
	{50,    KILLDC33,        "KILLDC33",           111,   1,      0, false},
	{51,    KILLDC32,        "KILLDC32",           112,   1,      0, false},
	{52,    KILLDC31,        "KILLDC31",           113,   1,      0, false},
	{53,    KILLDC30,        "KILLDC30",           114,   1,      0, false},
	{54,    KILLDC29,        "KILLDC29",           115,   1,      0, false},
	{55,    KILLDC28,        "KILLDC28",           116,   1,      0, false},
	{56,    KILLDC27,        "KILLDC27",           117,   1,      0, false},
	{57,    KILLDC26,        "KILLDC26",           118,   1,      0, false},
	{58,    KILLDC25,        "KILLDC25",           119,   1,      0, false},
	{59,    KILLDC24,        "KILLDC24",           120,   1,      0, false},
	{60,    KILLDC23,        "KILLDC23",           121,   1,      0, false},
	{61,    KILLDC22,        "KILLDC22",           122,   1,      0, false},
	{62,    KILLDC21,        "KILLDC21",           123,   1,      0, false},
	{63,    KILLDC20,        "KILLDC20",           124,   1,      0, false},
	{64,    KILLDC19,        "KILLDC19",           125,   1,      0, false},
	{65,    KILLDC18,        "KILLDC18",           126,   1,      0, false},
	{66,    KILLDC17,        "KILLDC17",           127,   1,      0, false},
	{67,    KILLDC16,        "KILLDC16",           128,   1,      0, false},
	{68,    KILLDC15,        "KILLDC15",           129,   1,      0, false},
	{69,    KILLDC14,        "KILLDC14",           130,   1,      0, false},
	{70,    KILLDC13,        "KILLDC13",           131,   1,      0, false},
	{71,    KILLDC12,        "KILLDC12",           132,   1,      0, false},
	{72,    KILLDC11,        "KILLDC11",           133,   1,      0, false},
	{73,    KILLDC10,        "KILLDC10",           134,   1,      0, false},
	{74,    KILLDC9,         "KILLDC9",            135,   1,      0, false},
	{75,    KILLDC8,         "KILLDC8",            136,   1,      0, false},
	{76,    KILLDC7,         "KILLDC7",            137,   1,      0, false},
	{77,    KILLDC6,         "KILLDC6",            138,   1,      0, false},
	{78,    KILLDC5,         "KILLDC5",            139,   1,      0, false},
	{79,    KILLDC4,         "KILLDC4",            140,   1,      0, false},
	{80,    KILLDC3,         "KILLDC3",            141,   1,      0, false},
	{81,    KILLDC2,         "KILLDC2",            142,   1,      0, false},
	{82,    KILLDC1,         "KILLDC1",            143,   1,      0, false},
	{83,    KILLDC0,         "KILLDC0",            144,   1,      0, false}
};
*/

// ******************* pixel shift register *************************
static ConfDataStruct pixCnfRegItem[PIXEL_REG_ITEMS] = {
	{ 0,    IDLE,              "IDLE",                 0,   1,   0, false},
	{ 1,    COMMAND,           "COMMAND",              1,  13,   0, false},
	{ 2,    CHIPADDRESS,       "CHIPADDRESS",         14,   4,  31, false},
	{ 3,    REGADDRESS,        "REGADDRESS",          18,   6,   0, false},

	{ 4,    PIXEL26880,			"PIXEL672",				  24, 32, 0, false},
	{ 5,    PIXEL26848,			"PIXEL640",				  56, 32, 0, false},
	{ 6,    PIXEL26816,			"PIXEL608",				  88, 32, 0, false},
	{ 7,    PIXEL26784,			"PIXEL576",				 120, 32, 0, false},
	{ 8,    PIXEL26752,			"PIXEL544",				 152, 32, 0, false},
	{ 9,    PIXEL26720,			"PIXEL512",				 184, 32, 0, false},
	{10,    PIXEL26688,			"PIXEL480",				 216, 32, 0, false},
	{11,    PIXEL26656,			"PIXEL448",				 248, 32, 0, false},
	{12,    PIXEL26624,			"PIXEL416",				 280, 32, 0, false},
	{13,    PIXEL26592,			"PIXEL384",				 312, 32, 0, false},
	{14,    PIXEL26560,			"PIXEL352",				 344, 32, 0, false},
	{15,    PIXEL26528,			"PIXEL320",				 376, 32, 0, false},
	{16,    PIXEL26496,			"PIXEL288",				 408, 32, 0, false},
	{17,    PIXEL26464,			"PIXEL256",				 440, 32, 0, false},
	{18,    PIXEL26432,			"PIXEL224",				 472, 32, 0, false},
	{19,    PIXEL26400,			"PIXEL192",				 504, 32, 0, false},
	{20,    PIXEL26368,			"PIXEL160",				 536, 32, 0, false},
	{21,    PIXEL26336,			"PIXEL128",				 568, 32, 0, false},
	{22,    PIXEL26304,			"PIXEL96",				 600, 32, 0, false},
	{23,    PIXEL26272,			"PIXEL64",				 632, 32, 0, false},
	{24,    PIXEL26240,			"PIXEL32",				 664, 32, 0, false}
};								// can stay...

// for readback of SR content to BlockRAM
static ConfDataStruct pixCnfRegRBItem[PIXEL_REG_RB_ITEMS] = {
	{ 0,    PIXEL26880,			"PIXEL672",				   0, 32, 0, false},
	{ 1,    PIXEL26848,			"PIXEL640",				  32, 32, 0, false},
	{ 2,    PIXEL26816,			"PIXEL608",				  64, 32, 0, false},
	{ 3,    PIXEL26784,			"PIXEL576",				  96, 32, 0, false},
	{ 4,    PIXEL26752,			"PIXEL544",				 128, 32, 0, false},
	{ 5,    PIXEL26720,			"PIXEL512",				 160, 32, 0, false},
	{ 6,    PIXEL26688,			"PIXEL480",				 192, 32, 0, false},
	{ 7,    PIXEL26656,			"PIXEL448",				 224, 32, 0, false},
	{ 8,    PIXEL26624,			"PIXEL416",				 256, 32, 0, false},
	{ 9,    PIXEL26592,			"PIXEL384",				 288, 32, 0, false},
	{10,    PIXEL26560,			"PIXEL352",				 320, 32, 0, false},
	{11,    PIXEL26528,			"PIXEL320",				 352, 32, 0, false},
	{12,    PIXEL26496,			"PIXEL288",				 384, 32, 0, false},
	{13,    PIXEL26464,			"PIXEL256",				 416, 32, 0, false},
	{14,    PIXEL26432,			"PIXEL224",				 448, 32, 0, false},
	{15,    PIXEL26400,			"PIXEL192",				 480, 32, 0, false},
	{16,    PIXEL26368,			"PIXEL160",				 512, 32, 0, false},
	{17,    PIXEL26336,			"PIXEL128",				 544, 32, 0, false},
	{18,    PIXEL26304,			"PIXEL96",				 576, 32, 0, false},
	{19,    PIXEL26272,			"PIXEL64",				 608, 32, 0, false},
	{20,    PIXEL26240,			"PIXEL32",				 640, 32, 0, false}
};	






// ScanChain
// ******************* Scan chain *************************
// Data output block 
static ConfDataStruct scanChainDOBds[SC_DOB_ITEMS] = {				//FIXME
	{ 0,    SCDOB0,             "SCDOB0",         0,   32,   0, false},
	{ 1,    SCDOB1,             "SCDOB1",        32,   32,   0, false},
	{ 2,    SCDOB2,             "SCDOB2",        64,    8,   0, false}
};
// Command decoder
static ConfDataStruct scanChainCMDds[SC_CMD_ITEMS] = {				//FIXME
	{ 0,    SCCMD0,             "SCCMD0",         0,    32,   0, false},
	{ 1,    SCCMD1,             "SCCMD1",        32,    32,   0, false},
	{ 2,    SCCMD2,             "SCCMD2",        64,    32,   0, false},
	{ 3,    SCCMD3,             "SCCMD3",        96,    32,   0, false},
	{ 4,    SCCMD4,             "SCCMD4",       128,    32,   0, false},
	{ 5,    SCCMD5,             "SCCMD5",       160,    32,   0, false},
	{ 6,    SCCMD6,             "SCCMD6",       192,    32,   0, false},
	{ 7,    SCCMD7,             "SCCMD7",       224,    32,   0, false},
	{ 8,    SCCMD8,             "SCCMD8",       256,     6,   0, false}

};
// End of chip logic
static ConfDataStruct scanChainECL_FEI4A_ds[SC_ECL_A_ITEMS] = {				//FIXME
	{	0	,	 SCECL0	,	 "	SCECL0	"	,	0, 	32,	0, false},
	{	1	,	 SCECL1	,	 "	SCECL1	"	,	32, 	32,	0, false},
	{	2	,	 SCECL2	,	 "	SCECL2	"	,	64, 	32,	0, false},
	{	3	,	 SCECL3	,	 "	SCECL3	"	,	96, 	32,	0, false},
	{	4	,	 SCECL4	,	 "	SCECL4	"	,	128, 	32,	0, false},
	{	5	,	 SCECL5	,	 "	SCECL5	"	,	160, 	32,	0, false},
	{	6	,	 SCECL6	,	 "	SCECL6	"	,	192, 	32,	0, false},
	{	7	,	 SCECL7	,	 "	SCECL7	"	,	224, 	32,	0, false},
	{	8	,	 SCECL8	,	 "	SCECL8	"	,	256, 	32,	0, false},
	{	9	,	 SCECL9	,	 "	SCECL9	"	,	288, 	32,	0, false},
	{	10	,	 SCECL10,	 "	SCECL10	"	,	320, 	32,	0, false},
	{	11	,	 SCECL11,	 "	SCECL11	"	,	352, 	32,	0, false},
	{	12	,	 SCECL12,	 "	SCECL12	"	,	384, 	32,	0, false},
	{	13	,	 SCECL13,	 "	SCECL13	"	,	416, 	32,	0, false},
	{	14	,	 SCECL14,	 "	SCECL14	"	,	448, 	32,	0, false},
	{	15	,	 SCECL15,	 "	SCECL15	"	,	480, 	32,	0, false},
	{	16	,	 SCECL16,	 "	SCECL16	"	,	512, 	32,	0, false},
	{	17	,	 SCECL17,	 "	SCECL17	"	,	544, 	32,	0, false},
	{	18	,	 SCECL18,	 "	SCECL18	"	,	576, 	32,	0, false},
	{	19	,	 SCECL19,	 "	SCECL19	"	,	608, 	32,	0, false},
	{	20	,	 SCECL20,	 "	SCECL20	"	,	640, 	32,	0, false},
	{	21	,	 SCECL21,	 "	SCECL21	"	,	672, 	32,	0, false},
	{	22	,	 SCECL22,	 "	SCECL22	"	,	704, 	32,	0, false},
	{	23	,	 SCECL23,	 "	SCECL23	"	,	736, 	32,	0, false},
	{	24	,	 SCECL24,	 "	SCECL24	"	,	768, 	32,	0, false},
	{	25	,	 SCECL25,	 "	SCECL25	"	,	800, 	32,	0, false},
	{	26	,	 SCECL26,	 "	SCECL26	"	,	832, 	32,	0, false},
	{	27	,	 SCECL27,	 "	SCECL27	"	,	864, 	32,	0, false},
	{	28	,	 SCECL28,	 "	SCECL28	"	,	896, 	32,	0, false},
	{	29	,	 SCECL29,	 "	SCECL29	"	,	928, 	32,	0, false},
	{	30	,	 SCECL30,	 "	SCECL30	"	,	960, 	32,	0, false},
	{	31	,	 SCECL31,	 "	SCECL31	"	,	992, 	32,	0, false},
	{	32	,	 SCECL32,	 "	SCECL32	"	,	1024, 	32,	0, false},
	{	33	,	 SCECL33,	 "	SCECL33	"	,	1056, 	32,	0, false},
	{	34	,	 SCECL34,	 "	SCECL34	"	,	1088, 	32,	0, false},
	{	35	,	 SCECL35,	 "	SCECL35	"	,	1120, 	32,	0, false},
	{	36	,	 SCECL36,	 "	SCECL36	"	,	1152, 	32,	0, false},
	{	37	,	 SCECL37,	 "	SCECL37	"	,	1184, 	32,	0, false},
	{	38	,	 SCECL38,	 "	SCECL38	"	,	1216, 	32,	0, false},
	{	39	,	 SCECL39,	 "	SCECL39	"	,	1248, 	32,	0, false},
	{	40	,	 SCECL40,	 "	SCECL40	"	,	1280, 	32,	0, false},
	{	41	,	 SCECL41,	 "	SCECL41	"	,	1312, 	32,	0, false},
	{	42	,	 SCECL42,	 "	SCECL42	"	,	1344, 	32,	0, false},
	{	43	,	 SCECL43,	 "	SCECL43	"	,	1376, 	32,	0, false},
	{	44	,	 SCECL44,	 "	SCECL44	"	,	1408, 	32,	0, false},
	{	45	,	 SCECL45,	 "	SCECL45	"	,	1440, 	32,	0, false},
	{	46	,	 SCECL46,	 "	SCECL46	"	,	1472, 	32,	0, false},
	{	47	,	 SCECL47,	 "	SCECL47	"	,	1504, 	32,	0, false},
	{	48	,	 SCECL48,	 "	SCECL48	"	,	1536, 	32,	0, false},
	{	49	,	 SCECL49,	 "	SCECL49	"	,	1568, 	32,	0, false},
	{	50	,	 SCECL50,	 "	SCECL50	"	,	1600, 	32,	0, false},
	{	51	,	 SCECL51,	 "	SCECL51	"	,	1632, 	32,	0, false},
	{	52	,	 SCECL52,	 "	SCECL52	"	,	1664, 	32,	0, false},
	{	53	,	 SCECL53,	 "	SCECL53	"	,	1696, 	32,	0, false},
	{	54	,	 SCECL54,	 "	SCECL54	"	,	1728, 	32,	0, false},
	{	55	,	 SCECL55,	 "	SCECL55	"	,	1760, 	32,	0, false},
	{	56	,	 SCECL56,	 "	SCECL56	"	,	1792, 	32,	0, false},
	{	57	,	 SCECL57,	 "	SCECL57	"	,	1824, 	32,	0, false},
	{	58	,	 SCECL58,	 "	SCECL58	"	,	1856, 	32,	0, false},
	{	59	,	 SCECL59,	 "	SCECL59	"	,	1888, 	32,	0, false},
	{	60	,	 SCECL60,	 "	SCECL60	"	,	1920, 	32,	0, false},
	{	61	,	 SCECL61,	 "	SCECL61	"	,	1952, 	32,	0, false},
	{	62	,	 SCECL62,	 "	SCECL62	"	,	1984, 	32,	0, false},
	{	63	,	 SCECL63,	 "	SCECL63	"	,	2016, 	32,	0, false},
	{	64	,	 SCECL64,	 "	SCECL64	"	,	2048, 	32,	0, false},
	{	65	,	 SCECL65,	 "	SCECL65	"	,	2080, 	32,	0, false},
	{	66	,	 SCECL66,	 "	SCECL66	"	,	2112, 	32,	0, false},
	{	67	,	 SCECL67,	 "	SCECL67	"	,	2144, 	32,	0, false},
	{	68	,	 SCECL68,	 "	SCECL68	"	,	2176, 	32,	0, false},
	{	69	,	 SCECL69,	 "	SCECL69	"	,	2208, 	32,	0, false},
	{	70	,	 SCECL70,	 "	SCECL70	"	,	2240, 	32,	0, false},
	{	71	,	 SCECL71,	 "	SCECL71	"	,	2272, 	32,	0, false},
	{	72	,	 SCECL72,	 "	SCECL72	"	,	2304, 	32,	0, false},
	{	73	,	 SCECL73,	 "	SCECL73	"	,	2336, 	32,	0, false},
	{	74	,	 SCECL74,	 "	SCECL74	"	,	2368, 	32,	0, false},
	{	75	,	 SCECL75,	 "	SCECL75	"	,	2400, 	32,	0, false},
	{	76	,	 SCECL76,	 "	SCECL76	"	,	2432, 	32,	0, false},
	{	77	,	 SCECL77,	 "	SCECL77	"	,	2464, 	32,	0, false},
	{	78	,	 SCECL78,	 "	SCECL78	"	,	2496, 	32,	0, false},
	{	79	,	 SCECL79,	 "	SCECL79	"	,	2528, 	32,	0, false},
	{	80	,	 SCECL80,	 "	SCECL80	"	,	2560, 	32,	0, false},
	{	81	,	 SCECL81,	 "	SCECL81	"	,	2592, 	32,	0, false},
	{	82	,	 SCECL82,	 "	SCECL82	"	,	2624, 	32,	0, false},
	{	83	,	 SCECL83,	 "	SCECL83	"	,	2656, 	32,	0, false},
	{	84	,	 SCECL84,	 "	SCECL84	"	,	2688, 	32,	0, false},
	{	85	,	 SCECL85,	 "	SCECL85	"	,	2720, 	32,	0, false},
	{	86	,	 SCECL86,	 "	SCECL86	"	,	2752, 	32,	0, false},
	{	87	,	 SCECL87,	 "	SCECL87	"	,	2784, 	32,	0, false},
	{	88	,	 SCECL88,	 "	SCECL88	"	,	2816, 	32,	0, false},
	{	89	,	 SCECL89,	 "	SCECL89	"	,	2848, 	32,	0, false},
	{	90	,	 SCECL90,	 "	SCECL90	"	,	2880, 	32,	0, false},
	{	91	,	 SCECL91,	 "	SCECL91	"	,	2912, 	32,	0, false},
	{	92	,	 SCECL92,	 "	SCECL92	"	,	2944, 	32,	0, false},
	{	93	,	 SCECL93,	 "	SCECL93	"	,	2976, 	32,	0, false},
	{	94	,	 SCECL94,	 "	SCECL94	"	,	3008, 	32,	0, false},
	{	95	,	 SCECL95,	 "	SCECL95	"	,	3040, 	32,	0, false},
	{	96	,	 SCECL96,	 "	SCECL96	"	,	3072, 	32,	0, false},
	{	97	,	 SCECL97,	 "	SCECL97	"	,	3104, 	32,	0, false},
	{	98	,	 SCECL98,	 "	SCECL98	"	,	3136, 	32,	0, false},
	{	99	,	 SCECL99,	 "	SCECL99	"	,	3168, 	24,	0, false}
};

static ConfDataStruct scanChainECL_FEI4B_ds[SC_ECL_B_ITEMS] = {				//FIXME
	{	0	,	 B_SCECL0,	 "	B_SCECL0	"	,	0, 	32,	0, false},
	{	1	,	 B_SCECL1,	 "	B_SCECL1	"	,	32, 	32,	0, false},
	{	2	,	 B_SCECL2,	 "	B_SCECL2	"	,	64, 	32,	0, false},
	{	3	,	 B_SCECL3,	 "	B_SCECL3	"	,	96, 	32,	0, false},
	{	4	,	 B_SCECL4,	 "	B_SCECL4	"	,	128, 	32,	0, false},
	{	5	,	 B_SCECL5,	 "	B_SCECL5	"	,	160, 	32,	0, false},
	{	6	,	 B_SCECL6,	 "	B_SCECL6	"	,	192, 	32,	0, false},
	{	7	,	 B_SCECL7,	 "	B_SCECL7	"	,	224, 	32,	0, false},
	{	8	,	 B_SCECL8,	 "	B_SCECL8	"	,	256, 	32,	0, false},
	{	9	,	 B_SCECL9,	 "	B_SCECL9	"	,	288, 	32,	0, false},
	{	10	,	 B_SCECL10,	 "	B_SCECL10	"	,	320, 	32,	0, false},
	{	11	,	 B_SCECL11,	 "	B_SCECL11	"	,	352, 	32,	0, false},
	{	12	,	 B_SCECL12,	 "	B_SCECL12	"	,	384, 	32,	0, false},
	{	13	,	 B_SCECL13,	 "	B_SCECL13	"	,	416, 	32,	0, false},
	{	14	,	 B_SCECL14,	 "	B_SCECL14	"	,	448, 	32,	0, false},
	{	15	,	 B_SCECL15,	 "	B_SCECL15	"	,	480, 	32,	0, false},
	{	16	,	 B_SCECL16,	 "	B_SCECL16	"	,	512, 	32,	0, false},
	{	17	,	 B_SCECL17,	 "	B_SCECL17	"	,	544, 	32,	0, false},
	{	18	,	 B_SCECL18,	 "	B_SCECL18	"	,	576, 	32,	0, false},
	{	19	,	 B_SCECL19,	 "	B_SCECL19	"	,	608, 	32,	0, false},
	{	20	,	 B_SCECL20,	 "	B_SCECL20	"	,	640, 	32,	0, false},
	{	21	,	 B_SCECL21,	 "	B_SCECL21	"	,	672, 	32,	0, false},
	{	22	,	 B_SCECL22,	 "	B_SCECL22	"	,	704, 	32,	0, false},
	{	23	,	 B_SCECL23,	 "	B_SCECL23	"	,	736, 	32,	0, false},
	{	24	,	 B_SCECL24,	 "	B_SCECL24	"	,	768, 	32,	0, false},
	{	25	,	 B_SCECL25,	 "	B_SCECL25	"	,	800, 	32,	0, false},
	{	26	,	 B_SCECL26,	 "	B_SCECL26	"	,	832, 	32,	0, false},
	{	27	,	 B_SCECL27,	 "	B_SCECL27	"	,	864, 	32,	0, false},
	{	28	,	 B_SCECL28,	 "	B_SCECL28	"	,	896, 	32,	0, false},
	{	29	,	 B_SCECL29,	 "	B_SCECL29	"	,	928, 	32,	0, false},
	{	30	,	 B_SCECL30,	 "	B_SCECL30	"	,	960, 	32,	0, false},
	{	31	,	 B_SCECL31,	 "	B_SCECL31	"	,	992, 	32,	0, false},
	{	32	,	 B_SCECL32,	 "	B_SCECL32	"	,	1024, 	32,	0, false},
	{	33	,	 B_SCECL33,	 "	B_SCECL33	"	,	1056, 	32,	0, false},
	{	34	,	 B_SCECL34,	 "	B_SCECL34	"	,	1088, 	32,	0, false},
	{	35	,	 B_SCECL35,	 "	B_SCECL35	"	,	1120, 	32,	0, false},
	{	36	,	 B_SCECL36,	 "	B_SCECL36	"	,	1152, 	32,	0, false},
	{	37	,	 B_SCECL37,	 "	B_SCECL37	"	,	1184, 	32,	0, false},
	{	38	,	 B_SCECL38,	 "	B_SCECL38	"	,	1216, 	32,	0, false},
	{	39	,	 B_SCECL39,	 "	B_SCECL39	"	,	1248, 	32,	0, false},
	{	40	,	 B_SCECL40,	 "	B_SCECL40	"	,	1280, 	32,	0, false},
	{	41	,	 B_SCECL41,	 "	B_SCECL41	"	,	1312, 	32,	0, false},
	{	42	,	 B_SCECL42,	 "	B_SCECL42	"	,	1344, 	32,	0, false},
	{	43	,	 B_SCECL43,	 "	B_SCECL43	"	,	1376, 	32,	0, false},
	{	44	,	 B_SCECL44,	 "	B_SCECL44	"	,	1408, 	32,	0, false},
	{	45	,	 B_SCECL45,	 "	B_SCECL45	"	,	1440, 	32,	0, false},
	{	46	,	 B_SCECL46,	 "	B_SCECL46	"	,	1472, 	32,	0, false},
	{	47	,	 B_SCECL47,	 "	B_SCECL47	"	,	1504, 	32,	0, false},
	{	48	,	 B_SCECL48,	 "	B_SCECL48	"	,	1536, 	32,	0, false},
	{	49	,	 B_SCECL49,	 "	B_SCECL49	"	,	1568, 	32,	0, false},
	{	50	,	 B_SCECL50,	 "	B_SCECL50	"	,	1600, 	32,	0, false},
	{	51	,	 B_SCECL51,	 "	B_SCECL51	"	,	1632, 	32,	0, false},
	{	52	,	 B_SCECL52,	 "	B_SCECL52	"	,	1664, 	32,	0, false},
	{	53	,	 B_SCECL53,	 "	B_SCECL53	"	,	1696, 	32,	0, false},
	{	54	,	 B_SCECL54,	 "	B_SCECL54	"	,	1728, 	32,	0, false},
	{	55	,	 B_SCECL55,	 "	B_SCECL55	"	,	1760, 	32,	0, false},
	{	56	,	 B_SCECL56,	 "	B_SCECL56	"	,	1792, 	32,	0, false},
	{	57	,	 B_SCECL57,	 "	B_SCECL57	"	,	1824, 	32,	0, false},
	{	58	,	 B_SCECL58,	 "	B_SCECL58	"	,	1856, 	32,	0, false},
	{	59	,	 B_SCECL59,	 "	B_SCECL59	"	,	1888, 	32,	0, false},
	{	60	,	 B_SCECL60,	 "	B_SCECL60	"	,	1920, 	32,	0, false},
	{	61	,	 B_SCECL61,	 "	B_SCECL61	"	,	1952, 	32,	0, false},
	{	62	,	 B_SCECL62,	 "	B_SCECL62	"	,	1984, 	32,	0, false},
	{	63	,	 B_SCECL63,	 "	B_SCECL63	"	,	2016, 	32,	0, false},
	{	64	,	 B_SCECL64,	 "	B_SCECL64	"	,	2048, 	32,	0, false},
	{	65	,	 B_SCECL65,	 "	B_SCECL65	"	,	2080, 	32,	0, false},
	{	66	,	 B_SCECL66,	 "	B_SCECL66	"	,	2112, 	32,	0, false},
	{	67	,	 B_SCECL67,	 "	B_SCECL67	"	,	2144, 	32,	0, false},
	{	68	,	 B_SCECL68,	 "	B_SCECL68	"	,	2176, 	32,	0, false},
	{	69	,	 B_SCECL69,	 "	B_SCECL69	"	,	2208, 	32,	0, false},
	{	70	,	 B_SCECL70,	 "	B_SCECL70	"	,	2240, 	32,	0, false},
	{	71	,	 B_SCECL71,	 "	B_SCECL71	"	,	2272, 	32,	0, false},
	{	72	,	 B_SCECL72,	 "	B_SCECL72	"	,	2304, 	32,	0, false},
	{	73	,	 B_SCECL73,	 "	B_SCECL73	"	,	2336, 	32,	0, false},
	{	74	,	 B_SCECL74,	 "	B_SCECL74	"	,	2368, 	32,	0, false},
	{	75	,	 B_SCECL75,	 "	B_SCECL75	"	,	2400, 	32,	0, false},
	{	76	,	 B_SCECL76,	 "	B_SCECL76	"	,	2432, 	32,	0, false},
	{	77	,	 B_SCECL77,	 "	B_SCECL77	"	,	2464, 	32,	0, false},
	{	78	,	 B_SCECL78,	 "	B_SCECL78	"	,	2496, 	32,	0, false},
	{	79	,	 B_SCECL79,	 "	B_SCECL79	"	,	2528, 	32,	0, false},
	{	80	,	 B_SCECL80,	 "	B_SCECL80	"	,	2560, 	32,	0, false},
	{	81	,	 B_SCECL81,	 "	B_SCECL81	"	,	2592, 	32,	0, false},
	{	82	,	 B_SCECL82,	 "	B_SCECL82	"	,	2624, 	32,	0, false},
	{	83	,	 B_SCECL83,	 "	B_SCECL83	"	,	2656, 	32,	0, false},
	{	84	,	 B_SCECL84,	 "	B_SCECL84	"	,	2688, 	32,	0, false},
	{	85	,	 B_SCECL85,	 "	B_SCECL85	"	,	2720, 	32,	0, false},
	{	86	,	 B_SCECL86,	 "	B_SCECL86	"	,	2752, 	32,	0, false},
	{	87	,	 B_SCECL87,	 "	B_SCECL87	"	,	2784, 	32,	0, false},
	{	88	,	 B_SCECL88,	 "	B_SCECL88	"	,	2816, 	32,	0, false},
	{	89	,	 B_SCECL89,	 "	B_SCECL89	"	,	2848, 	32,	0, false},
	{	90	,	 B_SCECL90,	 "	B_SCECL90	"	,	2880, 	32,	0, false},
	{	91	,	 B_SCECL91,	 "	B_SCECL91	"	,	2912, 	32,	0, false},
	{	92	,	 B_SCECL92,	 "	B_SCECL92	"	,	2944, 	32,	0, false},
	{	93	,	 B_SCECL93,	 "	B_SCECL93	"	,	2976, 	32,	0, false},
	{	94	,	 B_SCECL94,	 "	B_SCECL94	"	,	3008, 	32,	0, false},
	{	95	,	 B_SCECL95,	 "	B_SCECL95	"	,	3040, 	32,	0, false},
	{	96	,	 B_SCECL96,	 "	B_SCECL96	"	,	3072, 	32,	0, false},
	{	97	,	 B_SCECL97,	 "	B_SCECL97	"	,	3104, 	32,	0, false},
	{	98	,	 B_SCECL98,	 "	B_SCECL98	"	,	3136, 	32,	0, false},
	{	99	,	 B_SCECL99,	 "	B_SCECL99	"	,	3168, 	32,	0, false},
	{	100	,	 B_SCECL100,	 "	B_SCECL100	"	,	3200, 	32,	0, false},
	{	101	,	 B_SCECL101,	 "	B_SCECL101	"	,	3232, 	32,	0, false},
	{	102	,	 B_SCECL102,	 "	B_SCECL102	"	,	3264, 	32,	0, false},
	{	103	,	 B_SCECL103,	 "	B_SCECL103	"	,	3296, 	32,	0, false},
	{	104	,	 B_SCECL104,	 "	B_SCECL104	"	,	3328, 	32,	0, false},
	{	105	,	 B_SCECL105,	 "	B_SCECL105	"	,	3360, 	32,	0, false},
	{	106	,	 B_SCECL106,	 "	B_SCECL106	"	,	3392, 	32,	0, false},
	{	107	,	 B_SCECL107,	 "	B_SCECL107	"	,	3424, 	32,	0, false},
	{	108	,	 B_SCECL108,	 "	B_SCECL108	"	,	3456, 	14,	0, false}
};

#endif
