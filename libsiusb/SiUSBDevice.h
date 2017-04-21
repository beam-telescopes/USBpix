//------------------------------------------------------------------------------
//       USBDevice.h
//
//       SILAB, Phys. Inst Bonn, HK
//
//  USB devices associated to slbusb.sys device driver
//
//  History:
//  03.10.01	modifiedadded init functions (StartDriver, StopDriver)
//				for TUSBDeviceManager
//  31.08.01	added latch and adc access for mqube tracker
//  06.11.03	some changes for the Class Explorer View
//  01.08.10	modifications for running serveral applications without
//				each loading TUSBDeviceManager (added GetIndex(), changed
//				StartDriver and StopDriver to public functions)
//------------------------------------------------------------------------------

#ifndef USBDeviceH
#define USBDeviceH

#include "SURConstants.h"
#include "SiXilinxChip.h"
#include "interface_libusb.h"
#include "interface_libusb_ioctl.h"

#ifdef FreeBSD
#include <libusb.h>
#else
#include "include/libusb.h"
#endif

class QMutex;

#ifdef NO_ERROR_MESSAGES
#define SHOW_BASIC_ERRORS 0
#else
#define SHOW_BASIC_ERRORS 1
#endif

#ifdef NO_HIGHER_LEVEL_ERROR_MESSAGES
#define SHOW_HIGHER_LEVEL_ERRORS 0
#else
#define SHOW_HIGHER_LEVEL_ERRORS 1
#endif

#define MSB(word) (unsigned char)(((unsigned short)word >> 8) & 0xff)
#define LSB(word) (unsigned char)((unsigned short)word & 0xff)

#define SiU_min(a, b)  (((a) < (b)) ? (a) : (b))

// general constants
#define EZUSB_VENDOR_ID    0x0547
#define EZUSB_PRODUCT_ID   0x2131
#define SLUSB_VENDOR_ID    0x5312

#define MAX_PIPES          30
#define MAXTRANSFERSIZE    0x1000 // size of the transfer buffer (*NOT* the packet size)
#define MAX_USB_PACKET_SIZE    64

#define MAX_SUR_DATA_SIZE  0xffff

// vendor specific requests
#define VR_ANCHOR_DLD   0xa0 // handled by core
#define VR_EEPROM       0xa2 // loads (uploads) EEPROM
#define	VR_RAM          0xa3 // loads (uploads) external ram
#define	VR_UPLOAD       0xc0 // correspondes to REQUEST_DIR_IN
#define VR_DOWNLOAD     0x40 // correspondes to REQUEST_DIR_OUT
#define VR_SETI2CADDR   0xa4
#define VR_GETI2C_TYPE  0xa5 // 8 or 16 unsigned char address
#define VR_GET_CHIP_REV 0xa6 // Rev A, B = 0, Rev C = 2 // NOTE: New TNG Rev
#define VR_TEST_MEM     0xa7 // runs mem test and returns result
#define VR_RENUM        0xa8 // renum
#define VR_PREPARE_RAM  0xaa // prepare RAM bulk access

// parameter for vendor specific requests to endpoint 0
#define REQUEST_DIR_OUT     0
#define REQUEST_DIR_IN      1
#define REQUEST_TYPE_STD    0
#define REQUEST_TYPE_CLS    1
#define REQUEST_TYPE_VDR    2
#define REQUEST_RCP_DEV     0
#define REQUEST_RCP_INT     1
#define REQUEST_RCP_EP      2

//-----------------------------------------------------------------------------
// Addresses in Cypress chip
//-----------------------------------------------------------------------------
#define bmBIT0			0x01
#define bmBIT1			0x02
#define bmBIT2			0x04
#define bmBIT3			0x08
#define bmBIT4			0x10
#define bmBIT5			0x20
#define bmBIT6			0x40
#define bmBIT7			0x80

#define CPUCS_REG       0x7F92
#define CPUCS_REG_FX	0xE600
#define PORTACFG		0x7F93	// selects between port I/O (0) or spec. func (1)
#define PORTBCFG		0x7F94
#define PORTCCFG		0x7F95
#define OUTA			0x7F96	// output register
#define OUTB			0x7F97
#define OUTC			0x7F98
#define PINSA			0x7F99	// input line
#define PINSB			0x7F9A
#define PINSC			0x7F9B
#define OEA				0x7F9C	// active high output enable
#define OEB				0x7F9D
#define OEC				0x7F9E
#define SCON1           0xC0
#define I2CS			0x7FA5  // I2C bus status register
#define I2CS_NACK		0x02	// I2C Status: I2C error; No Acknowledge
#define I2CS_FX			0xE678  // I2C bus status register
#define EEPROM_SIZE		8096


//--- FX2 port register
#define PORTACFG_FX		0xE670
#define IOA_FX			0x80
#define OEA_FX			0xB2

typedef void* USBD_PIPE_HANDLE;
typedef void* USBD_CONFIGURATION_HANDLE;
typedef void* USBD_INTERFACE_HANDLE;

typedef enum _USBD_PIPE_TYPE {
	UsbdPipeTypeControl,
	UsbdPipeTypeIsochronous,
	UsbdPipeTypeBulk,
	UsbdPipeTypeInterrupt
} USBD_PIPE_TYPE;


typedef struct _USBD_PIPE_INFORMATION {
	//
	// OUTPUT
	// These fields are filled in by USBD
	//
	unsigned short MaximumPacketSize;  // Maximum packet size for this pipe
	unsigned char EndpointAddress;     // 8 bit USB endpoint address (includes direction)
	// taken from endpoint descriptor
	unsigned char Interval;            // Polling interval in ms if interrupt pipe

	USBD_PIPE_TYPE PipeType;   // PipeType identifies type of transfer valid for this pipe
	USBD_PIPE_HANDLE PipeHandle;

	//
	// INPUT
	// These fields are filled in by the client driver
	//
	unsigned long MaximumTransferSize; // Maximum size for a single request
	// in bytes.
	unsigned long PipeFlags;
} USBD_PIPE_INFORMATION, *PUSBD_PIPE_INFORMATION;


typedef struct _USBD_INTERFACE_INFORMATION {
	unsigned short Length;       // Length of this structure, including
	// all pipe information structures that
	// follow.
	//
	// INPUT
	//
	// Interface number and Alternate setting this
	// structure is associated with
	//
	unsigned char InterfaceNumber;
	unsigned char AlternateSetting;

	//
	// OUTPUT
	// These fields are filled in by USBD
	//
	unsigned char Class;
	unsigned char SubClass;
	unsigned char Protocol;
	unsigned char Reserved;

	USBD_INTERFACE_HANDLE InterfaceHandle;

	unsigned long NumberOfPipes;

	//
	// INPUT/OUPUT
	// see PIPE_INFORMATION
	USBD_PIPE_INFORMATION Pipes[32];  // ???

} USBD_INTERFACE_INFORMATION, *PUSBD_INTERFACE_INFORMATION;


typedef struct __usb_Dev_Descriptor__
{
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned short bcdUSB;
	unsigned char bDeviceClass;
	unsigned char bDeviceSubClass;
	unsigned char bDeviceProtocol;
	unsigned char bMaxPacketSize0;
	unsigned short idVendor;
	unsigned short idProduct;
	unsigned short bcdDevice;
	unsigned char iManufacturer;
	unsigned char iProduct;
	unsigned char iSerialNumber;
	unsigned char bNumConfigurations;
} Usb_Device_Descriptor, *pUsb_Device_Descriptor;

typedef struct __usb_Config_Descriptor__
{
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned short wTotalLength;
	unsigned char bNumInterfaces;
	unsigned char bConfigurationValue;
	unsigned char iConfiguration;
	unsigned char bmAttributes;
	unsigned char MaxPower;
} Usb_Configuration_Descriptor, *pUsb_Configuration_Descriptor;

typedef struct __usb_String_Descriptor__
{
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned short bString[80];  // fsv:: type "unsigned short" - important for IOCTL !!!
} Usb_String_Descriptor, *pUsb_String_Descriptor;

#define MAX_STRING_INDEX  3

typedef long USBD_STATUS;

typedef struct _USBD_ISO_PACKET_DESCRIPTOR {

	unsigned long Offset;       // INPUT Offset of the packet from the begining of the buffer.
	unsigned long Length;       // OUTPUT length of data received (for in) OUTPUT 0 for OUT.

	USBD_STATUS Status; // status code for this packet.
} USBD_ISO_PACKET_DESCRIPTOR, *PUSBD_ISO_PACKET_DESCRIPTOR;



//------------------------------------------------------------------------------
//  TI2CMaster
//  virtual base class
//  write and read I2C functions are implemented in derived class (TUSBDevice)
//------------------------------------------------------------------------------
class TI2CMaster
{
public:
	TI2CMaster(){};
	virtual bool I2CAck() = 0;
	virtual bool ReadI2C(unsigned char SlaveAdd, unsigned char *Data, unsigned short Length) = 0;
	virtual bool WriteI2C(unsigned char SlaveAdd, unsigned char *Data, unsigned short Length) = 0;
	virtual bool WriteI2Cnv(unsigned char SlaveAdd, unsigned char *Data, unsigned short Length) = 0;
};

//------------------------------------------------------------------------------
//  TSPIMaster
//  virtual base class
//  write and read SPI functions are implemented in derived class (TUSBDevice)
//------------------------------------------------------------------------------
class TSPIMaster  // generic SPI controller
{
public:
	TSPIMaster(){};
	virtual void InitSPI() = 0;
	virtual bool WriteSPI(int add, unsigned char *Data, unsigned short length) = 0;
	virtual bool ReadSPI(int add, unsigned char *Data, unsigned short length) = 0;
};


class TI2CClockGen;

//------------------------------------------------------------------------------
//  TUSBDevice
//  THE main class
//------------------------------------------------------------------------------
class TUSBDevice: TI2CMaster, TSPIMaster, public TXilinxChip
{
	friend class TUSBDeviceManager;        // members einschrnken ?

public:
	TUSBDevice(int index);
	virtual ~TUSBDevice();
	void Init();

	bool StartDriver();                               // open device driver
	bool StopDriver();                               // close device driver

	TI2CClockGen *ClkGen;

	// identification values
	int GetId();
	int GetIndex(){ return DeviceDriverIndex;};
	const char* GetName();
	int GetClass();
	int GetFWVersion();
	int GetVendorId(){ return (int)VendorId;};
	int GetDeviceId(){ return (int)DeviceId;};
	const char* GetEndpointInfo();


	int DeviceDriverIndex;                         // device index inside the driver
	unsigned short VendorId;
	unsigned short ProductId;
	unsigned short DeviceId;

	// access EEPROM programmable device parameters
	bool ReadIDFromEEPROM();
	bool WriteIDToEEPROM(int id);
	bool ReadNameFromEEPROM();
	bool WriteNameToEEPROM(const char* name);
	bool ReadDeviceClassFromEEPROM();
	bool WriteDeviceClassToEEPROM(unsigned char dc);
	bool ReadFirmwareVersion();

	bool is_fx;

	// access 8051 internal registers
	bool Write8051(unsigned short address, unsigned char *Data, unsigned short length = 1);
	bool Read8051(unsigned short address, unsigned char *Data, unsigned short length = 1);
	bool SetBit8051(unsigned short address, unsigned char  mask, bool   set);
	bool GetBit8051(unsigned short address, unsigned char  mask, bool & get);

	// FPGA configuration
	bool InitXilinxConfPort();
	bool SetXilinxConfPin(unsigned char pin, unsigned char data);
	bool GetXilinxConfPin(unsigned char pin);
	bool SetXilinxConfByte(unsigned char data);
	unsigned char GetXilinxConfByte(void);
	bool WriteXilinxConfData(unsigned char *data, int size);
	bool XilinxAlreadyLoaded();

	// FPGA access
	bool WriteXilinx(unsigned short address, unsigned char *Data, int length = 1);
	bool ReadXilinx(unsigned short address, unsigned char *Data, int length = 1);
	bool ConfigXilinx(unsigned char *Data, unsigned short length);

	// UART
	bool WriteSerial(unsigned char *Data, unsigned short length);
	bool ReadSerial(unsigned char *Data, unsigned short length);

	// generic access to external data bus
	bool WriteExternal(unsigned short address, unsigned char *Data, int length);
	bool ReadExternal(unsigned short address, unsigned char *Data, int length);

	// access to fast data bus
	bool FastBlockWrite(unsigned char *data, int length);
	bool FastBlockRead(unsigned char *data, int length);  

	// FIFO access (application specific)
	//bool WriteFIFO(unsigned char *Data, unsigned short Length);
	unsigned short ReadFIFO(unsigned char *Data, int Length);
	//unsigned short ReadFIFO2(unsigned char *Data, unsigned short Length);

	// EEPROM access
	bool WriteEEPROM(unsigned short address, unsigned char *Data, unsigned short Length);
	bool ReadEEPROM(unsigned short address, unsigned char *Data, unsigned short Length);

	// generic I2C access
	bool I2CAck();
	bool WriteI2C(unsigned char SlaveAdd, unsigned char *data, unsigned short length);
	bool WriteI2Cnv(unsigned char SlaveAdd, unsigned char *data, unsigned short length);
	bool ReadI2C(unsigned char SlaveAdd, unsigned char *data, unsigned short length);

	// SPI access (application specific)
	void InitSPI();
	bool WriteSPI(int add, unsigned char *Data, unsigned short length);
	bool ReadSPI(int add, unsigned char *Data, unsigned short length);
	bool ReadAdcSPI(unsigned char address, unsigned char *Data);

	// legacy functions
	bool WriteLatch(unsigned char *Data);
	bool WriteCommand(unsigned char *Data, unsigned short length = 1); // used for uCScan
	bool ReadADC(unsigned char address, int *Data);

	// direct access to dedicated endpoints
	bool WriteRegister(unsigned char * Data);
	bool ReadRegister(unsigned char * Data);

	// firmware download
	bool LoadFirmwareFromFile(std::string FileName); // download firmware
	bool LoadHexFileToEeprom(std::string FileName);
	// bool ReadFirmwareVersion(unsigned char *Data);

	bool ResetCPU(); // Linux ONLY:: reset USB 8051 CPU

	// driver low level functions
	bool CyclePort(); // emulates unplug and re-plug
	bool ResetPipe(int nEndPoint); // resets specific pipe
	bool AbortPipe(int nEndPoint); // resets specific pipe
	bool DeviceAvailable(int want);

	// to allow for locked sequence of calls, locking should be done on nect level but with
	// mutex owned by this class to allow for cross-class locking
	QMutex* getMutex(){return m_mutex;}; 


private:
	libusb_context *ctx;
	bool WriteDataRecordToEeprom(INTEL_HEX_RECORD HexRecordStruct, unsigned short Address);

	bool finddev(libusb_device *dev); /* find our devices */

	/* low level access to USB device */
	bool WriteBulkEndpoint(int nEndPoint, unsigned char *outData, int outPacketSize);
	bool ReadBulkEndpoint(int nEndPoint, unsigned char *inData, int inPacketSize, int *nBytesRead);
	bool FastByteWrite(unsigned char *data);
	bool FastByteRead(unsigned char *data);

	void Hold8051();
	void Run8051();

	bool GetRevision(unsigned char *rev);
	bool SilabUsbRequest(PSILAB_USB_REQUEST pslb, unsigned char* data);
	bool SilabUsbRequest(PSILAB_USB_REQUEST pslb, unsigned char* data, int len);
	bool VendorRequest(VENDOR_REQUEST_IN *MyRequest);
	bool GetPipeInfo(); // read interface information
	bool GetDeviceDescriptor(); // get some information
	bool GetStringDescriptor(); // get some more information
	std::string PipeInfoString; // pipe info output buffer
	int  Id;
	int  FwVer;
	std::string Name; //!< name specifying the type of device (i.e. service)
	unsigned char DeviceClass;


	int sur_control_pipe;
	int sur_data_out_pipe;
	int sur_data_in_pipe;
	int eeprom_user_data_offset;
	int eeprom_mfg_addr;
	int eeprom_name_addr;
	int eeprom_id_addr;
	int eeprom_liac_addr;

	unsigned short xp_conf_port_cfg;
	unsigned short xp_conf_port_oe;
	unsigned short xp_conf_port_rd;
	unsigned short xp_conf_port_wr;
	unsigned short cpu_cs_reg;

	bool started; // driver started
	bool configured; // firmware active
	Usb_Device_Descriptor *Desc; // accessable pointer to descriptor information
	char *DescString[MAX_STRING_INDEX]; // pointer to string information

	int fdUSBDeviceHandle;

	HANDLE USBDeviceHandle; // 'Treibergriff'
	libusb_device *dev_handle;

	std::string DeviceDriverName; // must be "slbusb-n" with n = [0..7]
	std::string FirmwareFilename;

	unsigned char InterfaceInfo[1024]; // memory for interface and pipe info
	PUSBD_INTERFACE_INFORMATION pInterface; // interface information structure
	PUSBD_PIPE_INFORMATION pPipe; // pipe information structure
	void *pvDescriptorBuffer; // void pointer to descriptor information
	void *pvStringBuffer[MAX_STRING_INDEX]; // void pointer to information string

	QMutex *m_mutex; // for thread-safe calls to this class
};
#endif

