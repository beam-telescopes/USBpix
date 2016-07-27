/******************************************************************************
 * 
 *  Title  : pixelCommands.h 
 *  Version: 01 February 2004
 *   
 *  Description:  
 *  Related files: 
 *  Documentation: 
 *
 *  Author: John Richardson, john.richardson@cern.ch
 *
 *  modifications:    
 *   - Adjusted to make compatible with the SCT declarations
 *     where applicable.                                         14.03.03 dpsf
 ******************************************************************************/
#ifndef PIXEL_COMMANDS_H	 /* multiple inclusion protection */
#define PIXEL_COMMANDS_H

/******************************************************************************
 *                             Definitions                                    *
 ******************************************************************************/ 
#define PRIMARY_FIBRE       0x2    //Not used; included for compatibility
#define ALL_CHIPS           0x10   //For pixel modules chip == 16 => ALL.
#define CHIP_ADDR_LOOP      0xFF    

#define CHIP_ADDRESS(fibre, addressBits3to0) (addressBits3to0)

/* Generic Pixel module command tags */
#define NO_CMD           0
#define FEI2_OFFSET     22

typedef enum 
{ 
   PIX_FE_NULL          =1,
   PIX_FE_SOFT_RESET    =2,
   PIX_FE_WRITE_GLOBAL  =3,
   PIX_FE_READ_GLOBAL   =4,
   PIX_FE_WRITE_HITBUS  =5,
   PIX_FE_WRITE_SELECT  =6,
   PIX_FE_WRITE_PREAMP  =7,
   PIX_FE_WRITE_ENABLE  =8,
   PIX_FE_WRITE_TDAC0   =9,
   PIX_FE_WRITE_TDAC1   =10,
   PIX_FE_WRITE_TDAC2   =11,
   PIX_FE_WRITE_TDAC3   =12,
   PIX_FE_WRITE_TDAC4   =13,
   PIX_FE_WRITE_TDAC5   =14,
   PIX_FE_WRITE_TDAC6   =15,   
   PIX_FE_WRITE_FDAC0   =16,
   PIX_FE_WRITE_FDAC1   =17,
   PIX_FE_WRITE_FDAC2   =18,
   PIX_FE_WRITE_FDAC3   =19,
   PIX_FE_WRITE_FDAC4   =20,
   PIX_FE_READ_PIXEL    =21,
   PIX_FE_REF_RESET     =22
   
} FE_CMD_ID; 

#define FE_CMD_ID_MAX   49
 
typedef enum 
{ 
   PIX_FE_CLOCK_GLOBAL  =51,
   PIX_FE_CLOCK_PIXEL   =52
   
} FEDATA_CMD_ID;  
#define FEDATA_CMD_ID_MAX   99

typedef enum {
   PIX_MC_TRIGGER       =101,
   PIX_MC_ECR     		=102,
   PIX_MC_BCR     		=103,
   PIX_MC_SYNC    		=104,
   PIX_EDUMMY           =105,
   PIX_MC_CAL     		=106
   
} MCFAST_CMD_ID;
#define MCFAST_CMD_ID_MAX   111
#define L1_TRIGGER          (PIX_MC_TRIGGER)
#define SOFT_RESET          (PIX_MC_ECR)
#define BC_RESET            (PIX_MC_BCR)
#define CALIBRATION_PULSE   (PIX_MC_CAL)

typedef enum {
	DELAY              =112,
	MASK               =113     //dpsf ask John R about corresp. pixel cmd.
} EXTRA_CMD_ID;

typedef enum {
   PIX_MC_RD_REGISTER   =151,       
   PIX_MC_RD_FIFO       =152,
   PIX_MC_EN_DATA_TAKE  =153,     
   PIX_MC_RESET_MCC     =154, 
   PIX_MC_RESET_FE      =155

} MCSLOW_CMD_ID;
#define MCSLOW_CMD_ID_MAX   199
#define ENABLE_DATA_TAKING  (PIX_MC_EN_DATA_TAKE)

typedef enum {
   PIX_MC_WR_REGISTER   =201,      
   PIX_MC_WR_FIFO       =202

} MCDATA_CMD_ID;
#define MCDATA_CMD_ID_MAX   249

typedef enum {
   PIX_MC_WR_RECEIVER   =251          

} MCREC_CMD_ID;
#define MCREC_CMD_ID_MAX   299
 
 /* FE command encoding */

#define FE_NULL_1         0x00000
#define FE_SOFT_RESET_1   0x00001 
#define FE_CLOCK_GLOBAL_1 0x00002
#define FE_WRITE_GLOBAL_1 0x00004
#define FE_READ_GLOBAL_1  0x00008
#define FE_CLOCK_PIXEL_1  0x00010
#define FE_WRITE_HITBUS_1 0x00020
#define FE_WRITE_SELECT_1 0x00040
#define FE_WRITE_PREAMP_1 0x00080
#define FE_WRITE_ENABLE_1 0x00100
#define FE_WRITE_TDAC0_1  0x00200
#define FE_WRITE_TDAC1_1  0x00400
#define FE_WRITE_TDAC2_1  0x00800
#define FE_WRITE_TDAC3_1  0x01000
#define FE_WRITE_TDAC4_1  0x02000
#define FE_WRITE_FDAC0_1  0x04000
#define FE_WRITE_FDAC1_1  0x08000
#define FE_WRITE_FDAC2_1  0x10000
#define FE_WRITE_FDAC3_1  0x20000
#define FE_WRITE_FDAC4_1  0x40000
#define FE_READ_PIXEL_1   0x80000

#define FE_NULL_2         0x000000
#define FE_REF_RESET_2    0x000002
#define FE_SOFT_RESET_2   0x00000C 
#define FE_CLOCK_GLOBAL_2 0x000010
#define FE_WRITE_GLOBAL_2 0x000060
#define FE_READ_GLOBAL_2  0x000080
#define FE_CLOCK_PIXEL_2  0x000100
#define FE_WRITE_HITBUS_2 0x000200
#define FE_WRITE_SELECT_2 0x000400
#define FE_WRITE_ENABLE_2 0x000800
#define FE_WRITE_TDAC0_2  0x001000
#define FE_WRITE_TDAC1_2  0x002000
#define FE_WRITE_TDAC2_2  0x004000
#define FE_WRITE_TDAC3_2  0x008000
#define FE_WRITE_TDAC4_2  0x010000
#define FE_WRITE_TDAC5_2  0x020000
#define FE_WRITE_TDAC6_2  0x040000
#define FE_WRITE_FDAC0_2  0x080000
#define FE_WRITE_FDAC1_2  0x100000
#define FE_WRITE_FDAC2_2  0x200000
#define FE_WRITE_PREAMP_2 0x400000
#define FE_READ_PIXEL_2   0x800000

/* FE options */

#define FE_GA_BROADCAST 16

#define FE_MASK_ENABLE  0
#define FE_MASK_SELECT  1
#define FE_MASK_PREAMP  2
#define FE_MASK_HITBUS  3
#define FE_MASK_TDAC0   4
#define FE_MASK_TDAC1   5
#define FE_MASK_TDAC2   6
#define FE_MASK_TDAC3   7
#define FE_MASK_TDAC4   8
#define FE_MASK_TDAC5   9
#define FE_MASK_TDAC6   10
#define FE_MASK_FDAC0   11
#define FE_MASK_FDAC1   12 
#define FE_MASK_FDAC2   13 
#define FE_MASK_FDAC3   14
#define FE_MASK_FDAC4   15

#define FE_MASK_ACTUALLY_TDACS 20
#define FE_MASK_ACTUALLY_FDACS 21

#define FE_DO_MUX_GLOBCLK    0
#define FE_DO_MUX_HITBUS     1
#define FE_DO_MUX_SOFTRSTB   2
#define FE_DO_MUX_SERDATA    3
#define FE_DO_MUX_MONSEL     4
#define FE_DO_MUX_PIXCLK     5
#define FE_DO_MUX_DGND       6
#define FE_DO_MUX_DVDD       7
#define FE_DO_MUX_EVENT      8
#define FE_DO_MUX_COMMAND    9
#define FE_DO_MUX_READFIFO   10
#define FE_DO_MUX_PIXEL      11
#define FE_DO_MUX_ACCEPT     12
#define FE_DO_MUX_ROCK       13
#define FE_DO_MUX_GLOBSEROUT 14
#define FE_DO_MUX_GLOBAL     15

#define FE_CINJ_LOW  0
#define FE_CINJ_HIGH 1

#define FE_TOT_MODE_NORMAL 0
#define FE_TOT_MODE_LE     1
#define FE_TOT_MODE_TE     3

#define FE_TOT_THR_MODE_OFF  0
#define FE_TOT_THR_MODE_MIN  1
#define FE_TOT_THR_MODE_DUB  2
#define FE_TOT_THR_MODE_BOTH 3

#define FE_MASK_ALTERNATIVE 0
#define FE_MASK_ALL_OFF     1
#define FE_MASK_ALL_ON      2
#define FE_MASK_ODDS_ON     3
#define FE_MASK_EVENS_ON    4
#define FE_MASK_0_9         5
#define FE_MASK_0_1         6
#define FE_MASK_2_3         7
#define FE_MASK_4_5         8
#define FE_MASK_6_7         9
#define FE_MASK_8_9         10
#define FE_MASK_10_11       11
#define FE_MASK_12_13       12
#define FE_MASK_14_15       13
#define FE_MASK_16_17       14
#define FE_MASK_ALL_NO_GANG 15
#define FE_MASK_0_9_NO_GANG 16

#define FE_TDACS_ALTERNATIVE 128

/* FE properties */

#define FE_I1                 1
#define FE_I2                 2
#define FE_COMMAND_LEN        32
#define FE_COMMAND_BYTES      4
#define FE_TRUE_COMMAND_LEN_1 25
#define FE_TRUE_COMMAND_LEN_2 29
#define FE_GA_LEN             5
#define FE_GLOBAL_LEN_1       202
#define FE_GLOBAL_LEN_2       231
#define FE_PIXEL_LEN          2880
#define FE_NO_DATA_LEN        0 
#define FE_GA_LEN             5
#define FE_SYNC_SHORT_LEN     2
#define FE_SYNC_MED_LEN       5
#define FE_SYNC_LONG_LEN      10 
#define FE_GLOBAL_ELEMENTS_1  63
#define FE_GLOBAL_ELEMENTS_2  78

/* MCC command encoding */

#define MC_TRIGGER 29
#define MC_FAST    22
#define MC_BCR     1
#define MC_ECR     2
#define MC_CAL     4
#define MC_SYNC    8
#define MC_SLOW    11

/* MCC slow command encoding */

#define MC_WR_REGISTER  0           
#define MC_RD_REGISTER  1     
#define MC_WR_FIFO      2     
#define MC_RD_FIFO      3     
#define MC_WR_FRONT_END 4			 
#define MC_RD_FRONT_END 5          
#define MC_WR_RECEIVER  6           
#define MC_UNDEFINED    7            
#define MC_EN_DATA_TAKE 8          
#define MC_RESET_MCC    9      
#define MC_RESET_FE     10       

/* MCC register address' */

#define MC_CSR     0
#define MC_LV1     1
#define MC_FEE     2
#define MC_WFE	   3
#define MC_WMCC    4
#define MC_CNT	   5
#define MC_CALREG  6
#define MC_PEF	   7
#define MC_SBSR    8
#define MC_WBITD   9
#define MC_WRECD   10

/* MCC options */

#define MCC_SINGLE_40 0
#define MCC_DOUBLE_40 1
#define MCC_SINGLE_80 2
#define MCC_DOUBLE_80 3

/* Command lengths */

#define TRIGGER_CMD_LENGTH    5
#define FAST_CMD_LENGTH       9
#define MCC_SLOW_CMD_LENGTH   17
#define MCC_REG_CMD_LENGTH    33
#define MCC_FIFO_CMD_LENGTH   38
#define FE_CMD_LENGTH         273 /* Not including 8*DCNT (variable) */  

#define CNT_DATA(dcnt) (FE_COMMAND_BYTES+((dcnt)<<3))

#endif   /* multiple inclusion protection */
