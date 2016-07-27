#ifndef _CALDATA_HEADER
#define _CALDATA_HEADER

// EEPROM (calibration data storage)
#define EEPROM_ADD (0xA8)    //slave address
#define CAL_EEPROM_ADD (0xA8)    //slave address
#define CAL_DATA_HEADER_V1 ((unsigned short)(0xa101))
#define CAL_DATA_HEADER_V2 ((unsigned short)(0xa102))
#define CAL_EEPROM_PAGE_SIZE 32
#define MAX_CHANNEL_NAME_SIZE 64
enum {CAL_DATA_V1, CAL_DATA_V2, CAL_DATA_FILE};

#endif
