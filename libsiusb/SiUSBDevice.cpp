/*
 *       USBDevice.cpp
 *
 *       SILAB, Phys. Inst Bonn, HK
 *
 *  USB devices associated to slbusb.sys device driver
 *
 *  History:
 *  04.03.02  added generic I2C functions
 *  03.10.01  modifiedadded init functions (StartDriver, StopDriver)
 *            for TUSBDeviceManager
 *  31.08.01  added latch and adc access for mqube tracker
 */

#ifdef __VISUALC__
#include <Windows.h>
#endif


#include <sstream>

#include "SURConstants.h"
#include "SiUSBDevice.h"
#include "SiI2CDevice.h"

#include <QMutex>
#include <QMutexLocker>

#ifdef WIN32LOCK
#include "SiUSBDeviceManager.h"

extern TUSBDeviceManager *USBDevMan;
#endif

#define MTRANS 0xffff /* The Maximum Transfer lenght, now 64KiBytes */

const char PIPE_TYPE_STRINGS[4][5] = {
	"CTRL",
	"ISO",
	"BULK",
	"INT"
};

const char PIPE_DIRECTION[2][4] = {
	"OUT",
	"IN"
};

int
HexToInt2(std::string hString) {
	int val;

	val = strtol(hString.c_str(),(char **)NULL,16);

	return val;
}

void
SwapBytes(unsigned char *data, int size) {
	unsigned char temp;

	for (int i = 0; i < size; i++) {
		temp = 0;

		for (unsigned char j = 0; j < 8; j++) {
			temp |= (unsigned char)(((data[i] & (0x01 << j)) >>  j) << (7 - j));
		}

		data[i] = temp;
	}
}

/*
 * Constructor: initializes member variables
 */
TUSBDevice::TUSBDevice(int index):TI2CMaster(), TSPIMaster(), TXilinxChip(true)
{
	USBDeviceHandle = INVALID_HANDLE_VALUE;        // handle to the driver
	pvDescriptorBuffer = NULL;
	dev_handle = NULL;
	ClkGen = NULL;

	for (unsigned char sIndex = 0; sIndex < MAX_STRING_INDEX; ++sIndex) {
	  pvStringBuffer[sIndex] = NULL;
	  DescString[sIndex] = NULL;
	}
	
	Desc = NULL;                   // pointer to descriptor information
	DeviceDriverIndex = index;
	std::stringstream stream;

	stream << "silab" << DeviceDriverIndex;

	std::string arraystring = stream.str();
	DeviceDriverName = arraystring; // device name associated within the driver
	started = false;
	configured = false;            // firmware active
	Name  = "";
	Id    = 0;
	FwVer = -1;       

	m_mutex = new QMutex();
}

/*
 * Starts the USB driver, opens the device handle.
 */
bool
TUSBDevice::StartDriver()
{
	/* open Device handler */
	if (libusb_open(dev_handle, &USBDeviceHandle)) {
		perror("Error: libusb_open");

		return false;
	}

	/* Claim interface 0 */
	if (libusb_claim_interface(USBDeviceHandle, 0)) {
		perror("Error: libusb_claim");

		return false;
	}

	// seems to be the only way to avoid commands getting
	// stuck if device was connected in previous session
	libusb_reset_device(USBDeviceHandle);

	/* Init rest */
	Init();

	started = true;
	return true;
}

/* 
 * Initialize the USB device. It gets the device information, checks Vendor and Product
 * ID to set up the proper configuration of the chip. Further it reads the EEPROM and
 * gather ID, firmware version and name.
 */
void
TUSBDevice::Init()
{
	/* This is for debug output */
	if(SF_DEBUG_USB>0) {
		printf("***> TUSBDevice::Init() ===========================================================\n");
		printf("***> TUSBDevice::Init() ==              B o a r d   I N I T                      ==\n");
		printf("***> TUSBDevice::Init() ===========================================================\n");
	}
	
	/* Get the device descriptor */
	if (!GetDeviceDescriptor()) {
		printf("TUSBDevice::Init() error \n");
		return;
	}

	if(SF_DEBUG_USB>0) printf("  VendorId = 0x%x ProductId = 0x%x\n",Desc->idVendor,Desc->idProduct);

	VendorId = Desc->idVendor;
	ProductId = Desc->idProduct;
	
	/* Get data about the endpoints and pipes */
	GetPipeInfo();

	// If we got a FX2LP device
	if ((ProductId & 0x0200) || (ProductId == 0x8613)) {
		sur_control_pipe  = SUR_CONTROL_PIPE_FX;
		sur_data_out_pipe = SUR_DATA_OUT_PIPE_FX;
		sur_data_in_pipe  = SUR_DATA_IN_PIPE_FX;
		cpu_cs_reg        = CPUCS_REG_FX;
		eeprom_user_data_offset = EEPROM_USER_DATA_OFFSET_FX;
		eeprom_mfg_addr  = EEPROM_MFG_ADDR_FX;
		eeprom_name_addr = EEPROM_NAME_ADDR_FX;
		eeprom_id_addr   = EEPROM_ID_ADDR_FX;
		eeprom_liac_addr = EEPROM_LIAC_ADDR_FX;
		xp_conf_port_cfg = PORTACFG_FX;
		xp_conf_port_oe  = OEA_FX;
		xp_conf_port_rd  = IOA_FX;
		xp_conf_port_wr  = IOA_FX;
		is_fx = true;
	} else {
		sur_control_pipe  = SUR_CONTROL_PIPE;
		sur_data_out_pipe = SUR_DATA_OUT_PIPE;
		sur_data_in_pipe  = SUR_DATA_IN_PIPE;
		cpu_cs_reg        = CPUCS_REG;
		eeprom_user_data_offset = EEPROM_USER_DATA_OFFSET;
		eeprom_mfg_addr  = EEPROM_MFG_ADDR;
		eeprom_name_addr = EEPROM_NAME_ADDR;
		eeprom_id_addr   = EEPROM_ID_ADDR;
		eeprom_liac_addr = EEPROM_LIAC_ADDR;
		xp_conf_port_cfg = PORTCCFG;
		xp_conf_port_oe  = OEC;
		xp_conf_port_rd  = PINSC;
		xp_conf_port_wr  = OUTC;
		is_fx = false;
	}

	Id = -1;
	FwVer = -1;
	DeviceClass = 0xff;

	/* if firmware is availabe */
	if(pInterface->NumberOfPipes != 0) {
		//bool got_strdesc = GetStringDescriptor(); /* FIXME */
		bool got_strdesc = false;

		if (!ReadIDFromEEPROM()) {
			Id = -1;
		}

		if (!ReadFirmwareVersion()) {
			FwVer = -1;
		}

		if (!ReadNameFromEEPROM()) {
			if (got_strdesc)
				Name = std::string(DescString[2]);
			else
				Name = "Unknown";
		}

		if (!ReadDeviceClassFromEEPROM()){
			DeviceClass = 0xff;
		}

		configured = true;
	} else {
		Name = "not available";
		Id = DeviceDriverIndex + 100;
	}

	/* init clock */
	ClkGen = new TI2CClockGen((TI2CMaster *) this, CG_FREF);

	if(!ClkGen->isOk) {
		delete ClkGen;
		ClkGen = NULL;
	}

	if (SF_DEBUG_USB>0) printf("   < TUSBDevice::Init() Id=%d  Name=%s DeviceClass=0x%x\n"
		,Id,Name.c_str(),DeviceClass);
	if (SF_DEBUG_USB>0) {
		printf("   < TUSBDevice::Init() ============================================================\n");
		printf("   < TUSBDevice::Init() ==              E N D  Board  I N I T                     ==\n");
		printf("   < TUSBDevice::Init() ============================================================\n");
	}
}

/*
 * Check if device is available. Therefore it has to match a special vendor and
 * product ID. If it does, it returns true.
 */
bool
TUSBDevice::DeviceAvailable(int want)
{
	libusb_device **list;
	int found_devices = 0;
	int i;
	ssize_t cnt;
	
	/* Get the USB device list */
	cnt = libusb_get_device_list(NULL, &list);

	for (i = 0; i < cnt; i++) {
		 if (finddev(list[i])) {
			++found_devices;

			if (found_devices == (want + 1)) {
				dev_handle = list[i];

				libusb_free_device_list(list, 0); /* free list */

				return true;
			}
		 }
        }

	libusb_free_device_list(list, 0); /* free list */

	return false;
}

/*
 * Find specified vendor and product ID. It only supports the current Spartan 3 models.
 */
bool
TUSBDevice::finddev(libusb_device *dev)
{
        libusb_device_descriptor dev_desc;

        if (libusb_get_device_descriptor(dev, &dev_desc) < 0) {
                perror("Error: usb_get_device_descriptor");

                return false;
        }

        if (dev_desc.idVendor == 21266 && dev_desc.idProduct == 512) {
                return true;
        }

        return false;
}

/*
 * Stops the driver and close the device handle.
 */
bool
TUSBDevice::StopDriver()
{
	if(started) {
		if (SF_DEBUG_USB>0) printf("***> TUSBDevice::StopDriver()  \n");

		if (USBDeviceHandle == INVALID_HANDLE_VALUE) {
			return false;
		} else {
			libusb_close(USBDeviceHandle); 
		}

		return true;
	} else {
		return false;
	}
}


/*
 * Destructor: Closes device driver and frees memory
 */
TUSBDevice::~TUSBDevice()
{
	if(ClkGen != NULL)
		delete ClkGen;

	if(configured) {
		for (unsigned char sIndex = 0; sIndex < MAX_STRING_INDEX; ++sIndex) {
			if (pvStringBuffer[sIndex] != NULL)
				free(pvStringBuffer[sIndex]);
			if (DescString[sIndex] != NULL)
				free(DescString[sIndex]);
		}
	}

	if(pvDescriptorBuffer != NULL)
		free(pvDescriptorBuffer);
	
	StopDriver();

	delete m_mutex;
}


bool
TUSBDevice::ReadIDFromEEPROM()
{
	EEPROM_ID_STRUCT idstr;
	void *ptr;
	bool status;
	std::string str;

	if (SF_DEBUG_USB > 2) printf("***> TUSBDevice::ReadIDFromEEPROM()  \n");

	ptr = &idstr;

	status = ReadEEPROM(eeprom_id_addr, (unsigned char*)ptr, EEPROM_ID_SIZE);

	if (!status) {
		ShowLastError("TUSBDevice::ReadIDFromEEPROM");
		return false;
	}

	Id = atoi(idstr.content);

	if (SF_DEBUG_USB > 0) printf("   < TUSBDevice::ReadIDFromEEPROM()     Id=%d Str=%s \n",Id,idstr.content);

	return true;
}

bool
TUSBDevice::ReadNameFromEEPROM()
{
	EEPROM_NAME_STRUCT namestr;
	bool status;
	void *ptr;
	std::string str;

	if (SF_DEBUG_USB>2) printf("***> TUSBDevice::ReadNameFromEEPROM()  \n");

	ptr = &namestr;

	status = ReadEEPROM(eeprom_name_addr, (unsigned char*)ptr, EEPROM_NAME_SIZE);

	if (!status)
	{
		ShowLastError("TUSBDevice::ReadNameFromEEPROM");

		return false;
	}

	str = namestr.content;
	Name.assign(str,0,namestr.length);
	if (SF_DEBUG_USB>0) printf("     TUSBDevice::ReadNameFromEEPROM()   Name=%s \n",Name.c_str());

	return true;
}

bool
TUSBDevice::WriteIDToEEPROM(int id)
{
	EEPROM_ID_STRUCT idstr;
	void *ptr;
	bool status;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::WriteIDToEEPROM(id=%d)  \n",id);

	sprintf(idstr.content, "%d", id);
	idstr.length = SiU_min(strlen(idstr.content), (unsigned int)EEPROM_ID_SIZE-1);

	ptr = &idstr;

	status = WriteEEPROM(eeprom_id_addr, (unsigned char*)ptr, (unsigned int)EEPROM_ID_SIZE);

	if (!status) {
#if (SHOW_HIGHER_LEVEL_ERRORS)
		ShowLastError("TUSBDevice::WriteIDToEEPROM");
#endif  
		return false;
	}

	return true;
}

bool
TUSBDevice::WriteNameToEEPROM(const char* Name)
{
	EEPROM_NAME_STRUCT namestr;
	void *ptr;
	bool status;
	std::string newName(Name);

	strcpy(namestr.content, newName.c_str());

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::WriteNameToEEPROM(%s)  \n",newName.c_str());

	namestr.length  = SiU_min(newName.Length(), EEPROM_NAME_SIZE-1);

	ptr = &namestr;

	status = WriteEEPROM(eeprom_name_addr, (unsigned char*)ptr, EEPROM_NAME_SIZE);

	if (!status) {
#if (SHOW_HIGHER_LEVEL_ERRORS)
		ShowLastError("TUSBDevice::WriteNameToEEPROM");
#endif
		return false;
	}

	return true;
}

bool
TUSBDevice::WriteDeviceClassToEEPROM(unsigned char dc)
{
	bool status;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::WriteDeviceClassToEEPROM(dc=0x%x) \n",dc);

	status = WriteEEPROM(eeprom_liac_addr, &dc, (unsigned int)EEPROM_LIAC_SIZE);

	if (!status) {
#if (SHOW_HIGHER_LEVEL_ERRORS)
		ShowLastError("TUSBDevice::WriteDeviceClassToEEPROM");
#endif
		return false;
	}

	return true;
}

bool
TUSBDevice::ReadDeviceClassFromEEPROM()
{
	bool status;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::ReadDeviceClassFromEEPROM() \n");

	status = ReadEEPROM(eeprom_liac_addr, &DeviceClass, EEPROM_LIAC_SIZE);

	if (!status) {
#if (SHOW_HIGHER_LEVEL_ERRORS)
		ShowLastError("TUSBDevice::ReadDeviceClassFromEEPROM");
#endif  
		return false;
	}

	if (SF_DEBUG_USB>0) printf("TUSBDevice::ReadDeviceClassFromEEPROM() DeviceClass=0x%x\n",DeviceClass);

	return true;
}

int
TUSBDevice::GetId()
{
	ReadIDFromEEPROM();

	return Id;
}

const char *
TUSBDevice::GetName()
{
	ReadNameFromEEPROM();

	return Name.c_str();
}

int
TUSBDevice::GetClass()
{
	ReadDeviceClassFromEEPROM();
	return DeviceClass;
}

int
TUSBDevice::GetFWVersion()
{
	return FwVer;
}

const char *
TUSBDevice::GetEndpointInfo() {
	return PipeInfoString.c_str();
}


/*
 *  GetPipeInfo: Reads information about pipes and endpoints.
 */
bool
TUSBDevice::GetPipeInfo()
{
	libusb_config_descriptor *config;
        const libusb_interface *inter;
        const libusb_interface_descriptor *interdesc;
        const libusb_endpoint_descriptor *epdesc;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::GetPipeInfo() \n");

	/* Get Configuration descriptor */
	if (libusb_get_config_descriptor(libusb_get_device(USBDeviceHandle), 0, &config))
    return false;

	/* Check number of interfaces */
        if (config->bNumInterfaces == 1) {
                inter = &config->interface[0];
        } else {
                printf("Wrong number of interfaces: %i\n", config->bNumInterfaces);
                libusb_free_config_descriptor(config);

                return false;
        }

	/* Get interface description */
	interdesc = &inter->altsetting[0];

	pInterface = (PUSBD_INTERFACE_INFORMATION) InterfaceInfo;  // FIXME

	/* Assign Number of Endpoints/pipes, class... */
	pInterface->InterfaceNumber = 0;
	pInterface->AlternateSetting = interdesc->bAlternateSetting;
	pInterface->Class = interdesc->bInterfaceClass;
	pInterface->SubClass = interdesc->bInterfaceSubClass;
	pInterface->Protocol = interdesc->bInterfaceProtocol;
	pInterface->NumberOfPipes = interdesc->bNumEndpoints;

	/* FIXME Don't know what it is, so set it to zero... */
	pInterface->Reserved = 0;

	/* Make a pointer for easy access */
	pPipe = pInterface->Pipes;

	/* Calculate Lenght */
	pInterface->Length = (6 * sizeof(unsigned char) + sizeof(short) +
				pInterface->NumberOfPipes * sizeof(USBD_PIPE_INFORMATION));

	PipeInfoString = "";
	std::stringstream stream;
	if (SF_DEBUG_USB>0) printf("Number of pipes: %ld \n", pInterface->NumberOfPipes);

	for (int i = 0; i < (int) pInterface->NumberOfPipes; i++) {
		/* get endpoint descriptions */
		epdesc = &interdesc->endpoint[i];
	
		/* Assign MaxPacketSize, Endpointaddress... */
		pPipe[i].MaximumPacketSize = (unsigned short) epdesc->wMaxPacketSize;
		pPipe[i].EndpointAddress = epdesc->bEndpointAddress;
		pPipe[i].Interval = epdesc->bInterval;

		if (i == SUR_CONTROL_PIPE_FX) {
			pPipe[i].MaximumTransferSize = epdesc->wMaxPacketSize;
		} else {
			pPipe[i].MaximumTransferSize = MTRANS;
		}

		/* Descriptor Type conversion */
		if (epdesc->bDescriptorType == 5) {
			pPipe[i].PipeType = UsbdPipeTypeBulk;
		} else {
			pPipe[i].PipeType = UsbdPipeTypeControl;
		}

		/* Fill it with null pointer */
		pPipe[i].PipeHandle = NULL;
		
	  stream << " Pipe: " << i
		 << " Type: " << PIPE_TYPE_STRINGS[pPipe[i].PipeType]
		 << " Endpoint: " << (int)(epdesc->bEndpointAddress & 0x0F) << " "
		 << PIPE_DIRECTION[0x01 & (epdesc->bEndpointAddress >> 7)]
		 << " MaxPktSize: " << (int)epdesc->wMaxPacketSize
		 << std::endl;
	}

	if (SF_DEBUG_USB>0) printf("%s",stream.str().c_str());

	PipeInfoString = stream.str();
  libusb_free_config_descriptor(config);
	return true;
}

/*
 * Resets the CPU.
 */
bool
TUSBDevice::ResetCPU()
{
	bool result=false;
	unsigned long nBytes;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::ResetCPU() \n");

	if (USBDeviceHandle != NULL)
		result = DeviceIoControl(USBDeviceHandle,
		IOCTL_SLBUSB_RESET_CPU,
		pvDescriptorBuffer,
		sizeof (Usb_Device_Descriptor),
		pvDescriptorBuffer,
		sizeof (Usb_Device_Descriptor),
		&nBytes,
		NULL);

	return result;
}


/*
 * Reads configuration data
 */
bool
TUSBDevice::GetDeviceDescriptor()
{
	int result;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::GetDeviceDescriptor() ... ");

	/* Get some memory, plus some guardband area */
	if ((pvDescriptorBuffer = malloc(sizeof (Usb_Device_Descriptor) + 128)) == NULL)
	{
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::GetDeviceDescriptor");
#endif
		return false;
	}

	if (USBDeviceHandle != NULL) {
		result = libusb_get_device_descriptor(libusb_get_device(USBDeviceHandle),
				(struct libusb_device_descriptor *) pvDescriptorBuffer);
		if (!result) {
			Desc = (Usb_Device_Descriptor*) pvDescriptorBuffer;
			return true;
		}

		return false;
	}
	return false;
}

/*
 * Reads more configuration data.
 * Its not implemented yet!
 */
bool
TUSBDevice::GetStringDescriptor()
{
	bool result = 0;
	unsigned long nBytes;
	unsigned char ulLength;
	pUsb_String_Descriptor pSD[MAX_STRING_INDEX];      // array of pointer to string descriptor
	GET_STRING_DESCRIPTOR_IN input;

	GET_STRING_DESCRIPTOR_IN *input2;
	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::GetStringDescriptor() \n");

	input.LanguageId = 27;  // ignored anyway

	for (unsigned char sIndex = 0; sIndex < MAX_STRING_INDEX; sIndex++) // number of strings to read
	{
		input.Index = sIndex;
		DescString[sIndex] = NULL;

		if((pvStringBuffer[sIndex] = malloc (sizeof (Usb_String_Descriptor) + 128)) == NULL)
		{
#if (SHOW_BASIC_ERRORS)
			ShowLastError("TUSBDevice::GetStringDescriptor");
#endif
			return false;
		}

		input2 = (GET_STRING_DESCRIPTOR_IN *)pvStringBuffer[sIndex];
		input2->LanguageId = 27;  // ignored anyway
		input2->Index = sIndex;

		if (SF_DEBUG_USB>3) printf(" GetStringDescriptor(): IOCTL=%ld \n",IOCTL_SLBUSB_GET_STRING_DESCRIPTOR);

		/* Get the first unsigned chars of the descriptor to determine the size of the entire descriptor */
		if (USBDeviceHandle != NULL)
		{
			result = DeviceIoControl(USBDeviceHandle,
				IOCTL_SLBUSB_GET_STRING_DESCRIPTOR,
				&input,
				sizeof (GET_STRING_DESCRIPTOR_IN),
				pvStringBuffer[sIndex],
				sizeof (Usb_String_Descriptor),
				&nBytes,
				NULL);
		}
		else // fatal
		{
#if (SHOW_BASIC_ERRORS)
			ShowLastError("ERROR: USBDeviceHandle :: TUSBDevice::GetStringDescriptor() \n");
#endif
			return false;
		}

		pSD[sIndex] = (pUsb_String_Descriptor) pvStringBuffer[sIndex];

		DescString[sIndex] = (char *) malloc(pSD[sIndex]->bLength+1); DescString[sIndex][0]=0;

		strncpy (DescString[sIndex],(char*)pSD[sIndex]->bString,pSD[sIndex]->bLength); 
		if (SF_DEBUG_USB>0) 
			printf("     sIndex=%d, sz=%2d, Str=%s \n", sIndex,
			pSD[sIndex]->bLength,DescString[sIndex]);
		continue;

		if (result != true) // this will happen with no string defined
		{
			free (pvStringBuffer[sIndex]);
			pvStringBuffer[sIndex]=0;
#if (SHOW_BASIC_ERRORS)
			ShowLastError("oops");
#endif  
			return false;
		}

		ulLength = ((pUsb_String_Descriptor)pvStringBuffer[sIndex])->bLength;

		/* For empty strings */
		if (ulLength != 0) {
			/* Now get the entire descriptor */
			if((pvStringBuffer[sIndex] = realloc (pvStringBuffer[sIndex], ulLength)) == NULL)
			{

#if (SHOW_BASIC_ERRORS)
				ShowLastError("TUSBDevice::GetStringDescriptor");
#endif
				return false;
			}

			input2 = (GET_STRING_DESCRIPTOR_IN *)pvStringBuffer[sIndex];
			input2->LanguageId = 27;  // ignored anyway
			input2->Index = sIndex;
			if (SF_DEBUG_USB>3) 
				printf(" GetStringDescriptor(2)::iStr=%d, IOCTL_SLBUSB_GET_STRING_DESCRIPTOR=0x%x\n ",sIndex
				, (unsigned int) IOCTL_SLBUSB_GET_STRING_DESCRIPTOR);

			result = DeviceIoControl(USBDeviceHandle,
				IOCTL_SLBUSB_GET_STRING_DESCRIPTOR,
				&input,
				ulLength,
				pvStringBuffer[sIndex],
				ulLength,
				&nBytes,
				NULL);

			if (result == true) {
				/* parse string decriptor */
				pSD[sIndex] = (pUsb_String_Descriptor) pvStringBuffer[sIndex];
				DescString[sIndex] = (char *) malloc((pSD[sIndex]->bLength)/2);
				for(int i=0; i<((pSD[sIndex]->bLength/2)-1) /*&& i<sizeof(DescString[sIndex])*/; i++)
					sprintf (DescString[sIndex]+i, "%c", pSD[sIndex]->bString[i]);

			}
		}
	}
	return true;
}

/*
 * LoadFirmwareFromFile: loads *.bix files (memory images) to internal 8051 RAM
 * limited to 8k of program data, no access to external RAM or EEPROM
 */
bool
TUSBDevice::LoadFirmwareFromFile(std::string FileName)
{
//#define MAX_FILE_SIZE (1024*16)  // 16 k for FX2LP devices

	FILE *fpointer;
	unsigned char *buffer;
	int numread;
	unsigned long nBytes, fsize;
	bool result;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::LoadFirmwareFromFile(%s)\n",FileName.c_str());
	
	fpointer = fopen(FileName.c_str(),"rb");

	if (fpointer == NULL)
	{
		AnsiString Msg = "File not found." + FileName;

		if (SF_DEBUG_USB>0) printf("!!!! File not found:(%s)\n",FileName.c_str());

		return false;
	}

	fseek(fpointer, 0, SEEK_END);
	fsize = ftell(fpointer);
	rewind(fpointer);

	buffer = new unsigned char[fsize];

	numread = fread(buffer, 1, fsize, fpointer);

	if (numread != (int)fsize)
	{
		AnsiString Msg = "Reading error." + FileName;

		if (SF_DEBUG_USB>0) printf("!!!! Reading error:(%s)\n",FileName.c_str());

		return false;
	}

	fclose(fpointer);

	configured = false;

	Hold8051();

	result = DeviceIoControl(USBDeviceHandle,
		IOCTL_SLBUSB_ANCHOR_DOWNLOAD,
		buffer,
		numread,
		NULL,
		0,
		&nBytes,
		NULL);

	Run8051();

	delete[] buffer;
	if (result) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::LoadFirmwareFromFile");
#endif  
		return false;
	}
}

/*
 * WriteBulkEndpoint: writes outData with size of outPacketSize
 * to OUT endpoint (nEndPoint).
 */
bool
TUSBDevice::WriteBulkEndpoint(int nPipe, unsigned char *outData, int outPacketSize)
{
	bool result;
	unsigned long nBytes;
	BULK_TRANSFER_CONTROL bulkControl;

	bulkControl.pipeNum = nPipe;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::WriteBulkEndpoint(pipe=0x%x size=%d)\n",nPipe,outPacketSize);

	/* Performane the request to the libusb wrapper function */
	result = DeviceIoControl(USBDeviceHandle,
		IOCTL_SLBUSB_BULK_WRITE,
		&bulkControl,
		sizeof(BULK_TRANSFER_CONTROL),
		outData,
		outPacketSize,
		&nBytes,
		NULL);

	if (!result) {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::WriteBulkEndpoint");
#endif
		return false;
	}
	return true;
}

/*
 * ReadBulkEndpoint: reads inData with size of inPacketSize (max 64 unsigned char)
 * from IN endpoint (nEndPoint)
 */
bool
TUSBDevice::ReadBulkEndpoint(int nPipe, unsigned char *inData, int inPacketSize, int *nBytesRead)
{
	bool result;
	unsigned long nBytes;
	BULK_TRANSFER_CONTROL bulkControl;

	bulkControl.pipeNum = nPipe;

	if (SF_DEBUG_USB>1) printf("***> TUSBDevice::ReadBulkEndpoint(pipe=0x%x p-size=%d) ...."
		,nPipe,inPacketSize);

	/* Performane the request to the libusb wrapper function */
	result = DeviceIoControl(USBDeviceHandle,
		IOCTL_SLBUSB_BULK_READ,
		&bulkControl,
		sizeof(BULK_TRANSFER_CONTROL),
		inData,
		inPacketSize,
		&nBytes,
		NULL);

	if (!result) {
		*nBytesRead = 0;

#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::WriteBulkEndpoint");
#endif

		return false;
	}

	*nBytesRead = (int) nBytes;

	return true;
}

/*
 * VendorRequest: used for one unsigned char transfers.
 *
 * setupdata   name              meaning
 *   0        bmRequestType     Request Type, Direction, and Recepient
 *   1        bRequest          The actual request
 *   2        wValueL           Word-size value, varies according to bRequest
 *   3        wValueH
 *   4        wIndexL           Word-size field, varies according to bRequest
 *   5        wIndexH
 *   6        wLengthL          Number of unsigned chars to transfer if there is a data phase
 *   7        wLengthH
 */
bool
TUSBDevice::VendorRequest(VENDOR_REQUEST_IN *MyRequest)
{
	unsigned long nBytes;
	bool result;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::VendorRequest, size=%ld \n", sizeof(VENDOR_REQUEST_IN));

	result = DeviceIoControl(USBDeviceHandle,
		IOCTL_SLBUSB_VENDOR_REQUEST,
		MyRequest,
		sizeof(VENDOR_REQUEST_IN),
		&(MyRequest->bData),
		1,
		(unsigned long *)&nBytes,
		NULL);

	if (result) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::VendorRequest");
#endif  
		return false;
	}
}

/* 
 * The 'big' request is one with more than 64 KiBytes in length and is split into
 * 64 KiBytes before it is pusched to SilabUsbRequest.
 */ 
bool
TUSBDevice::SilabUsbRequest(PSILAB_USB_REQUEST psur, unsigned char* data, int biglength)
{
	bool status;
	int dataleft;
	int dataptr;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::SilabUsbRequest (BIG) len=%d MAX_SUR_DATA_SIZE=%d\n"
		,biglength,MAX_SUR_DATA_SIZE); 

	if ((biglength == 0) || (biglength <= MAX_SUR_DATA_SIZE)) {
		return SilabUsbRequest(psur, data);
	} else {
		dataleft = biglength;
		dataptr  = 0;

		/* Transfer 64k blocks */
		do {
			psur->length = MAX_SUR_DATA_SIZE;
			status = SilabUsbRequest(psur,(unsigned char*)(data + dataptr));
			if (!status)
				return false;
			dataleft -= MAX_SUR_DATA_SIZE;
			dataptr  += MAX_SUR_DATA_SIZE;
		} while (dataleft >= MAX_SUR_DATA_SIZE);

		psur->length = dataleft;
		status = SilabUsbRequest(psur, (unsigned char*)(data + dataptr));
	}

	return status;
}

/* 
 * This is one of the main wrapper caller functions and implements most
 * of the request to libusb.
 */
bool
TUSBDevice::SilabUsbRequest(PSILAB_USB_REQUEST psur, unsigned char* data)
{
	unsigned short ptr = 0; // data offset in current transfer
	int blocksize; // data size in current transfer
	unsigned long dataleft = psur->length; // initial requested data size
	unsigned long  MaxTransferSize;
	int nBytes;
	bool status;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::SilabUsbRequest()  size=%d MAX_SUR_DATA_SIZE=%d\n", 
			(int)dataleft, (int)MAX_SUR_DATA_SIZE);

#ifdef WIN32LOCK
	while (USBDevMan->usb_rwlock == 1) {
		Sleep(1);
	}

	USBDevMan->usb_rwlock = 1;

	if (USBDevMan->usb_rwlock == 1) {
		printf("RW locked!\n");
	} else {
		printf("RW not locked!\n");

		return false;
	}
#else
	QMutexLocker locker(m_mutex);
#endif

	unsigned char buffer[10];
	int buffersize;

	buffer[0] = psur->type;
	buffer[1] = psur->dir;

	if (is_fx) {
		buffer[2] = (unsigned char)(0xff &  psur->addr);
		buffer[3] = (unsigned char)(0xff & (psur->addr >> 8));
		buffer[4] = (unsigned char)(0xff & (psur->addr >> 16));
		buffer[5] = (unsigned char)(0xff & (psur->addr >> 24));
		buffer[6] = (unsigned char)(0xff &  psur->length);
		buffer[7] = (unsigned char)(0xff & (psur->length >> 8));
		buffer[8] = (unsigned char)(0xff & (psur->length >> 16));
		buffer[9] = (unsigned char)(0xff & (psur->length >> 24));
		buffersize = 10;
	} else {
		buffer[2] = (unsigned char)(0xff &  psur->addr);
		buffer[3] = (unsigned char)(0xff & (psur->addr >> 8));
		buffer[4] = (unsigned char)(0xff &  psur->length);
		buffer[5] = (unsigned char)(0xff & (psur->length >> 8));
		buffersize = 6;
	}

	if (SF_DEBUG_USB>5) printf("---| TUSBDevice::SilabUsbRequest()  sur_control_pipe=0x%x, bufsize=%d\n"
		,sur_control_pipe,buffersize);

	/* send setup data to bulk endpoint 1 to initialize data transfer */
	status = WriteBulkEndpoint(sur_control_pipe, buffer, buffersize);

	if (!status) {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::SilabUsbRequest(setup phase)");
#endif
		return false;
	}

	/* Remark: MAXTRANSFERSIZE is *NOT* the endpoints maximum *packet* size. It is
	 * the size of a transfer buffer which is handled by the USBD. In fact a single IO_CTL
	 * request can handle larger blocks than 64 unsigned char maximum endpoint packet size.
	 * MAXTRANSFERSIZE is controlled by the USBD. Get value with GetPipeInfo()
	 */
	if(psur->dir == SUR_DIR_OUT) {
		MaxTransferSize = pPipe[sur_data_out_pipe].MaximumPacketSize; /* MaximumTransferSize */
	} else {
		MaxTransferSize = pPipe[sur_data_in_pipe].MaximumPacketSize; /* MaximumTransferSize */
	}

	if (SF_DEBUG_USB>5) printf("---| TUSBDevice::SilabUsbRequest()  DIR=%d  MaxTransferSize=MaxPacketSize=%d Len=%d\n"
		, (int) psur->dir, (int) MaxTransferSize, (int) dataleft);

	while (dataleft > 0) {
		/* If data is less or more than the accepted buffer size */
		if (dataleft < MaxTransferSize) {
			blocksize = dataleft;
		} else {
			blocksize = MaxTransferSize;
		}

		if (psur->dir == SUR_DIR_OUT) {
			status &= WriteBulkEndpoint(sur_data_out_pipe, (unsigned char *) (data + ptr), blocksize);
		} else {
			status &= ReadBulkEndpoint(sur_data_in_pipe, (unsigned char *) (data + ptr), blocksize, &nBytes);
		}

		ptr += blocksize;	/* increment data pointer for next transfer */
		dataleft -= blocksize;  /* remaining data fraction */
	}

	if (status && (dataleft == 0)) {
#ifdef WIN32LOCK
		USBDevMan->usb_rwlock = 0;

		printf("RW lock released!\n");
#endif
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::SilabUsbRequest(data phase)");
#endif  
		return false;
	}

}

bool
TUSBDevice::FastByteWrite(unsigned char *data)
{
	SILAB_USB_REQUEST sur;
	bool status;

#ifdef WIN32LOCK
	while (USBDevMan->usb_rwlock == 1) {
		Sleep(1);
	}

	USBDevMan->usb_rwlock = 1;

	if (USBDevMan->usb_rwlock == 1) {
		printf("RW locked!\n");
	} else {
		printf("RW not locked!\n");

		return false;
	}
#else
	QMutexLocker locker(m_mutex);
#endif

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::FastByteWrite()  \n");

	sur.type   = SUR_TYPE_GPIFBYTE;
	sur.dir    = SUR_DIR_OUT;
	sur.addr   = 0;
	sur.length = 1;

	/* send setup data to bulk endpoint 1 to initialize data transfer */
	status  = WriteBulkEndpoint(sur_control_pipe, (unsigned char *)&sur, 10);
	status &= WriteBulkEndpoint(sur_data_out_pipe, data, 1);

	if (!status) {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::SilabUsbRequest(setup phase)");
#endif
		return false;
	}

#ifdef WIN32LOCK
	USBDevMan->usb_rwlock = 0;

	printf("RW lock released!\n");
#endif
	return true;
}

bool
TUSBDevice::FastByteRead(unsigned char *data)
{
	SILAB_USB_REQUEST sur;
	int nBytesRead;
	bool status;

#ifdef WIN32LOCK
	while (USBDevMan->usb_rwlock == 1) {
		Sleep(1);
	}

	USBDevMan->usb_rwlock = 1;

	if (USBDevMan->usb_rwlock == 1) {
		printf("RW locked!\n");
	} else {
		printf("RW not locked!\n");

		return false;
	}
#else
	QMutexLocker locker(m_mutex);
#endif
	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::FastByteRead() \n");

	sur.type   = SUR_TYPE_GPIFBYTE;
	sur.dir    = SUR_DIR_IN;
	sur.addr   = 0;
	sur.length = 1;

	/* send setup data to bulk endpoint 1 to initialize data transfer */
	status  = WriteBulkEndpoint(sur_control_pipe, (unsigned char *)&sur, 10);
	status &= ReadBulkEndpoint(sur_data_in_pipe, data, 1, &nBytesRead);

	if (status) {
#ifdef WIN32LOCK
	USBDevMan->usb_rwlock = 0;

	printf("RW lock released!\n");
#endif
		return true;
	} else {
		ShowLastError("TUSBDevice::FastBytewrite");

		return false;
	}
}

/* 
 * This is for fast write access to the USB device. It's only for USB 2.0 devices and uses
 * a special endpoint.
 */
bool
TUSBDevice::FastBlockWrite(unsigned char *data, int length)
{
	int addr = 0;
	unsigned char buffer[10];
	unsigned char buffersize = 10;
	int ptr = 0;
	int blocksize;          /* data size in current transfer */
	int dataleft = length;  /* initial requested data size */
	bool status;
	int MaxTransferSize;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::FastBlockWrite(len=%d) \n",length);

#ifdef WIN32LOCK
	while (USBDevMan->usb_rwlock == 1) {
		Sleep(1);
	}

	USBDevMan->usb_rwlock = 1;

	if (USBDevMan->usb_rwlock == 1) {
		printf("RW locked!\n");
	} else {
		printf("RW not locked!\n");

		return false;
	}
#else
	QMutexLocker locker(m_mutex);
#endif
	if (!length) {
		return false;
	}

	/* send setup data to bulk endpoint 0 to initialize data transfer */
	buffer[0] = SUR_TYPE_GPIFBLOCK;
	buffer[1] = SUR_DIR_OUT;
	buffer[2] = (unsigned char)(0xff & addr);
	buffer[3] = (unsigned char)(0xff & (addr >> 8));
	buffer[4] = (unsigned char)(0xff & (addr >> 16));
	buffer[5] = (unsigned char)(0xff & (addr >> 24));
	buffer[6] = (unsigned char)(0xff &  length);
	buffer[7] = (unsigned char)(0xff & (length >> 8));
	buffer[8] = (unsigned char)(0xff & (length >> 16));
	buffer[9] = (unsigned char)(0xff & (length >> 24));


	MaxTransferSize = (int)pPipe[SUR_DATA_FASTOUT_PIPE].MaximumTransferSize;

	/* send setup data to bulk endpoint 0 to initialize data transfer */
	status = WriteBulkEndpoint(sur_control_pipe, buffer, buffersize);

	if (!status) {
		ShowLastError("TUSBDevice::FastBlockWrite(setup phase)");

		return false;
	}

	while (dataleft > 0) {
		/* Check if it is less or more than buffer size */
		if (dataleft < MaxTransferSize) {
			blocksize = dataleft;
		} else {
			blocksize = MaxTransferSize;
		}

		status &= WriteBulkEndpoint(SUR_DATA_FASTOUT_PIPE, (unsigned char *) (data + ptr), blocksize);

		ptr      += blocksize; /* increment data pointer for next transfer */
		dataleft -= blocksize; /* remaining data fraction */
	}

	if (status && (dataleft == 0)) {
#ifdef WIN32LOCK
	USBDevMan->usb_rwlock = 0;

	printf("RW lock released!\n");
#endif
		return true;
	} else {
		ShowLastError("TUSBDevice::FastBlockwrite");

		return false;
	}
}

/* 
 * This is for fast write access to the USB device. It's only for USB 2.0 devices and uses
 * a special endpoint.
 */
bool
TUSBDevice::FastBlockRead(unsigned char *data, int length)
{
	int addr = 0;
	unsigned char buffer[10];
	unsigned char buffersize = 10;
	int ptr = 0;
	int blocksize;          /* data size in current transfer */
	int dataleft = length;  /* initial requested data size */
	int MaxTransferSize;
	int nBytesRead;
	bool status;

#ifdef WIN32LOCK
	while (USBDevMan->usb_rwlock == 1) {
		Sleep(1);
	}

	USBDevMan->usb_rwlock = 1;

	if (USBDevMan->usb_rwlock == 1) {
		printf("RW locked!\n");
	} else {
		printf("RW not locked!\n");

		return false;
	}
#else
	QMutexLocker locker(m_mutex);
#endif
	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::FastBlockRead(len=%d) \n",length);

	if (!length)
		return false;

	MaxTransferSize = pPipe[SUR_DATA_FASTIN_PIPE].MaximumTransferSize;
	
	while (dataleft > 0) {
		/* Check if it is less or more than buffer size */
		if (dataleft < MaxTransferSize) {
			blocksize = dataleft;
		} else {
			blocksize = MaxTransferSize;
		}

		/* send setup data to bulk endpoint 0 to initialize data transfer */
		buffer[0] = SUR_TYPE_GPIFBLOCK;
		buffer[1] = SUR_DIR_IN;
		buffer[2] = (unsigned char)(0xff & (addr+ptr));
		buffer[3] = (unsigned char)(0xff & ((addr+ptr) >> 8));
		buffer[4] = (unsigned char)(0xff & ((addr+ptr) >> 16));
		buffer[5] = (unsigned char)(0xff & ((addr+ptr) >> 24));
		buffer[6] = (unsigned char)(0xff &  blocksize);
		buffer[7] = (unsigned char)(0xff & (blocksize >> 8));
		buffer[8] = (unsigned char)(0xff & (blocksize >> 16));
		buffer[9] = (unsigned char)(0xff & (blocksize >> 24));
		status = WriteBulkEndpoint(sur_control_pipe, buffer, buffersize);

		if (!status)
		{
			ShowLastError("TUSBDevice::FastBlockRead(setup phase)");

			return false;
		}

		if (SF_DEBUG_USB>3) printf("***> TUSBDevice::FastBlockRead blocksize=%d MaxTransferSize=%d  dataleft=%d\n"
			,blocksize,MaxTransferSize,dataleft);

		status &= ReadBulkEndpoint(SUR_DATA_FASTIN_PIPE, (unsigned char *) (data + ptr),
				blocksize, &nBytesRead);

		ptr      += nBytesRead; /* increment data pointer for next transfer */
		dataleft -= nBytesRead; /* remaining data fraction */
		if (status == false)
			break;
	}


	if (status && (dataleft == 0)) {
#ifdef WIN32LOCK
	USBDevMan->usb_rwlock = 0;

	printf("RW lock released!\n");
#endif
		return true;
	} else {
		ShowLastError("TUSBDevice::FastBlockRead");

		return false;
	}
}

bool
TUSBDevice::WriteEEPROM(unsigned short address, unsigned char *Data, unsigned short Length)
{
#define EEPROM_PAGESIZE      64

	SILAB_USB_REQUEST sur;
	int temp_count;
	bool status = false;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::WriteEEPROM(adr=0x%x len=%d) \n",address,Length);

	sur.type   = SUR_TYPE_EEPROM;
	sur.dir    = SUR_DIR_OUT;
	sur.addr   = address;
	sur.length = Length;

	if (Length < EEPROM_PAGESIZE) {
		status = SilabUsbRequest(&sur, Data);
	} else {
		temp_count = Length;
		sur.length = EEPROM_PAGESIZE;

		while (temp_count > 0) {
			status = SilabUsbRequest(&sur, Data);
			temp_count -= EEPROM_PAGESIZE;
			sur.length  = SiU_min(EEPROM_PAGESIZE, temp_count);
			sur.addr   += EEPROM_PAGESIZE;
			Data       += EEPROM_PAGESIZE;
		}
	}

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::WriteEEPROM");
#endif  
		return false;
	}
}

bool
TUSBDevice::ReadEEPROM(unsigned short address, unsigned char *Data, unsigned short Length)
{
	SILAB_USB_REQUEST sur;
	int temp_count;
	bool status = false;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::ReadEEPROM(addr=0x%x, len=%d) \n",address,Length);

	sur.type   = SUR_TYPE_EEPROM;
	sur.dir    = SUR_DIR_IN;
	sur.addr   = address;
	sur.length = Length;

	if (Length <= MAX_USB_PACKET_SIZE) {
		status = SilabUsbRequest(&sur, Data);
	} else {
		temp_count = Length;
		sur.length = MAX_USB_PACKET_SIZE;
		while (temp_count > 0) {
			status = SilabUsbRequest(&sur, Data);
			temp_count -= MAX_USB_PACKET_SIZE;
			sur.length = SiU_min(MAX_USB_PACKET_SIZE, temp_count);
			sur.addr   += MAX_USB_PACKET_SIZE;
			Data       += MAX_USB_PACKET_SIZE;
		}
	}
	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::ReadEEPROM");
#endif  
		return false;
	}
}

bool
TUSBDevice::WriteExternal(unsigned short address, unsigned char *Data, int Length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::WriteExternal(adr=0x%x len=%d) \n",address,Length);

	sur.type   = SUR_TYPE_EXTERNAL;
	sur.dir    = SUR_DIR_OUT;
	sur.addr   = address;
	sur.length = Length;

	if (Length <= MTRANS) {
		status = SilabUsbRequest(&sur, Data);
	} else {
		status = SilabUsbRequest(&sur, Data, Length);
	}

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::WriteExternal");
#endif  
		return false;
	}
}


bool
TUSBDevice::ReadExternal(unsigned short address, unsigned char *Data, int Length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::ReadExternal(adr=0x%x len=%d) \n",address,Length);

	sur.type   = SUR_TYPE_EXTERNAL;
	sur.dir    = SUR_DIR_IN;
	sur.addr   = address;
	sur.length = Length;

	if (Length <= MTRANS) {
		status = SilabUsbRequest(&sur, Data);
	} else {
		status = SilabUsbRequest(&sur, Data, Length);
	}


	if (status)
		return true;
	else
	{
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::ReadExternal");
#endif
		return false;
	}
}

unsigned short
TUSBDevice::ReadFIFO(unsigned char *data, int size)
{
	int nBytes;
	bool status;
	unsigned char buffer[10];
	int buffersize;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::ReadFIFO(size=%d) \n",size);

#ifdef WIN32LOCK
	while (USBDevMan->usb_rwlock == 1) {
		Sleep(1);
	}

	USBDevMan->usb_rwlock = 1;

	if (USBDevMan->usb_rwlock == 1) {
		printf("RW locked!\n");
	} else {
		printf("RW not locked!\n");

		return false;
	}
#else
	QMutexLocker locker(m_mutex);
#endif

	buffer[0] = SUR_TYPE_FIFO;
	buffer[1] = SUR_DIR_IN;

	if(is_fx) {
		buffer[2] = 0;
		buffer[3] = 0;
		buffer[4] = 0;
		buffer[5] = 0;
		buffer[6] = (unsigned char)(0xff &  size);
		buffer[7] = (unsigned char)(0xff & (size >> 8));
		buffer[8] = (unsigned char)(0xff & (size >> 16));
		buffer[9] = (unsigned char)(0xff & (size >> 24));
		buffersize = 10;
	} else {
		buffer[2] = 0;
		buffer[3] = 0;
		buffer[4] = (unsigned char)(0xff &  size);
		buffer[5] = (unsigned char)(0xff & (size >> 8));
		buffersize = 6;
	}

	status = WriteBulkEndpoint(sur_control_pipe, buffer, buffersize);
	status &= ReadBulkEndpoint(sur_data_in_pipe, data, size, &nBytes);

	if(!status) {
		return 0;
	} else {
#ifdef WIN32LOCK
	USBDevMan->usb_rwlock = 0;

	printf("RW lock released!\n");
#endif
		return nBytes;
	}
}

bool
TUSBDevice::Write8051(unsigned short address, unsigned char *Data, unsigned short Length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::Write8051(adr=0x%x len=%d) \n",address,Length);

	sur.type   = SUR_TYPE_8051;
	sur.dir    = SUR_DIR_OUT;
	sur.addr   = address;
	sur.length = Length;

	status = SilabUsbRequest(&sur, Data);

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::Write8051");
#endif
		return false;
	}
}

bool
TUSBDevice::Read8051(unsigned short address, unsigned char *Data, unsigned short Length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::Read8051(adr=0x%x len=%d) \n",address,Length);

	sur.type   = SUR_TYPE_8051;
	sur.dir    = SUR_DIR_IN;
	sur.addr   = address;
	sur.length = Length;

	status = SilabUsbRequest(&sur, Data);

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::Read8051");
#endif  
		return false;
	}
}

bool
TUSBDevice::SetBit8051(unsigned short address, unsigned char mask, bool set)
{
	unsigned char portreg;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::SetBit8051(adr=0x%x mask=0x%x \n",address,mask);

	if (!Read8051(address, &portreg)) {
	       	return false;
	}

	portreg = set ? portreg | mask : portreg & (unsigned char) ~mask;

	/*
	if (!set)
	portreg &= (unsigned char) ~mask;
	else
	portreg |= mask;
	*/

	if (!Write8051(address, &portreg)) {
		return false;
	}

	return true;
}

bool
TUSBDevice::GetBit8051(unsigned short address, unsigned char mask, bool& get)
{
	unsigned char portreg;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::GetBit8051(adr=0x%x mask=0x%x \n",address,mask);

	if (!Read8051(address, &portreg)) { 
		return false;
	}

	get = ((portreg & mask) != 0);

	return true;
}

bool
TUSBDevice::WriteSerial(unsigned char *Data, unsigned short Length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::WriteSerial(len=%d) \n",Length);

	sur.type   = SUR_TYPE_SERIAL;
	sur.dir    = SUR_DIR_OUT;
	sur.addr   = 0;
	sur.length = Length;

	status = SilabUsbRequest(&sur, Data);

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::WriteSerial");
#endif  
		return false;
	}
}

bool
TUSBDevice::ReadSerial( unsigned char *Data, unsigned short Length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::ReadSerial(len=%d) \n",Length);

	sur.type   = SUR_TYPE_SERIAL;
	sur.dir    = SUR_DIR_IN;
	sur.addr   = 0;
	sur.length = Length;

	status = SilabUsbRequest(&sur, Data);

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::ReadSerial");
#endif
		return false;
	}
}


bool
TUSBDevice::I2CAck()
{
	unsigned char dummy;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::I2CAck()  \n");

	if (is_fx) {
		Read8051(I2CS_FX, &dummy, 1);
	} else {
		Read8051(I2CS, &dummy, 1);
	}

	return ((dummy & I2CS_NACK) != 0);
}


bool
TUSBDevice::ReadI2C(unsigned char SlaveAdd, unsigned char *data, unsigned short length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::ReadI2C(add=0x%x  len=%d) \n",SlaveAdd,length);

	sur.type   = SUR_TYPE_I2C;
	sur.dir    = SUR_DIR_IN;
	sur.addr   = SlaveAdd;
	sur.length = length;

	status = SilabUsbRequest(&sur, data);

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::ReadI2C");
#endif
		return false;
	}
}

bool
TUSBDevice::WriteI2C(unsigned char SlaveAdd, unsigned char *data, unsigned short length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::WriteI2C(add=0x%x len=%d) \n",SlaveAdd,length);

	sur.type   = SUR_TYPE_I2C;
	sur.dir    = SUR_DIR_OUT;
	sur.addr   = SlaveAdd;
	sur.length = length;

	status = SilabUsbRequest(&sur, data);

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::WriteI2C");
#endif  
		return false;
	}
}

bool
TUSBDevice::WriteI2Cnv(unsigned char SlaveAdd, unsigned char *data, unsigned short length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::WriteI2Cnv(len=%d) \n",length);
	sur.type   = SUR_TYPE_I2C_NV;
	sur.dir    = SUR_DIR_OUT;
	sur.addr   = SlaveAdd;
	sur.length = length;

	status = SilabUsbRequest(&sur, data);

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::WriteI2Cnv");
#endif
		return false;
	}
}

bool
TUSBDevice::WriteLatch(unsigned char *Data)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::WriteLatch()  \n");

	sur.type   = SUR_TYPE_LATCH;
	sur.dir    = SUR_DIR_OUT;
	sur.addr   = 0;
	sur.length = 1;

	status = SilabUsbRequest(&sur, Data);

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::WriteLatch");
#endif  
		return false;
	}
}

bool
TUSBDevice::WriteCommand(unsigned char *Data, unsigned short lenght)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>5) printf("***> TUSBDevice::WriteCommand()  \n");

	sur.type   = SUR_TYPE_CMD;
	sur.dir    = SUR_DIR_OUT;
	sur.addr   = 0;
	sur.length = lenght;

	status = SilabUsbRequest(&sur, Data);

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("!!!!Error: TUSBDevice::WriteCommand");
#endif
		return false;
	}
}


/* 
 * Enable special function port lines for SPI use
 *
 * SDI   PA7/RxD1out
 * SDO   PB2/RxD1
 * SCK   PB3/TxD1  
 */
void
TUSBDevice::InitSPI()
{
	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::InitSPI()  \n");

	SetBit8051(PORTACFG, bmBIT7, 1);

	SetBit8051(PORTBCFG, bmBIT2 | bmBIT3, 1);


	SetBit8051(PORTACFG, bmBIT6, 0);
	SetBit8051(OUTA, bmBIT6, 0);
	SetBit8051(OEA, bmBIT6, 1);

	/* address lines for chip select
	 *
	 *   ADD0  PB0
	 *   ADD1  PB1
	 *   ADD2  PB4
	 *   ADD3  PB5
	 *   ADD4  PB6
	 *
	 * ADC chip select
	 *  /CSADC PB7
	 */
	SetBit8051(PORTBCFG, bmBIT0 | bmBIT1 | bmBIT4 | bmBIT5 | bmBIT6 | bmBIT7, 0);

	SetBit8051(OUTB, bmBIT0 | bmBIT1 | bmBIT4 | bmBIT5 | bmBIT6, 0);

	SetBit8051(OUTB, bmBIT7, 1); /* de-select ADC */

	SetBit8051(OEB, bmBIT0 | bmBIT1 | bmBIT4 | bmBIT5 | bmBIT6 | bmBIT7, 1); /* enable outputs */

	/* enable UART for SPI functionality
	 *
	 * *** remember: UART sends and receives LSB first      ***
	 * *** transmit on falling edge, receive on rising edge ***
	 * *** half duplex communication, can't send and receive at the same time ***
	 *
	 * SCON1
	 * bit7   serial mode bit0
	 * bit6   serial mode bit1  (see EZ-USB C-31)
	 * bit5   multiprocessor enable
	 * bit4   receive enable
	 * bit3   9th data bit transmitted (mode 2 and 3 only)
	 * bit2   9th data bit received (mode 2 and 3 only)
	 * bit1   transmit interrupt flag
	 * bit0   receive interrupt flag
	 */
	SetBit8051(SCON1, 0x13, 1);  /* mode 0, baud 24MHz/12, enable receive */
}

bool
TUSBDevice::WriteSPI(int add, unsigned char *Data, unsigned short length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::WriteSPI(len=%d)  \n",length);

	sur.type   = SUR_TYPE_SPI;
	sur.dir    = SUR_DIR_OUT;
	sur.addr   = add;
	sur.length = length;

	status = SilabUsbRequest(&sur, Data);

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::WriteSPI");
#endif  
		return false;
	}
}

bool
TUSBDevice::ReadSPI(int add, unsigned char *Data, unsigned short length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::ReadSPI(len=%d)  \n",length);

	sur.type   = SUR_TYPE_SPI;
	sur.dir    = SUR_DIR_IN;
	sur.addr   = add;
	sur.length = length;

	status = SilabUsbRequest(&sur, Data);

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::ReadSPI");
#endif
		return false;
	}
}


bool
TUSBDevice::ReadADC(unsigned char address, int *Data)
{
	SILAB_USB_REQUEST sur;
	bool status;
	unsigned char bData[2];

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::ReadADC()  \n");

	sur.type   = SUR_TYPE_ADC;
	sur.dir    = SUR_DIR_IN;
	sur.addr   = address;
	sur.length = 2;

	status = SilabUsbRequest(&sur, bData);

	*Data = (unsigned short) (bData[1] + (bData[0] << 8));

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::ReadADC");
#endif  
		return false;
	}
}

bool
TUSBDevice::ReadAdcSPI(unsigned char address, unsigned char *Data)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::ReadAdcSPI() \n");

	sur.type   = SUR_TYPE_ADCSPI;
	sur.dir    = SUR_DIR_IN;
	sur.addr   = address;
	sur.length = 4;

	status = SilabUsbRequest(&sur, Data);

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::ReadAdcSPI");
#endif
		return false;
	}
}

bool
TUSBDevice::ReadFirmwareVersion()
{
	SILAB_USB_REQUEST sur;
	bool status;
	unsigned char Data[3];

	sur.type   = SUR_TYPE_FWVER;
	sur.dir    = SUR_DIR_IN;
	sur.addr   = 0;
	sur.length = 2;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::ReadFirmwareVersion() \n");

	status = SilabUsbRequest(&sur, Data);
	Data[2] = '\0';

	if (status) {
		FwVer = atoi((char*)Data);

		if (SF_DEBUG_USB>0)  printf("     TUSBDevice::ReadFirmwareVersion()  VERSION=%d  \n",FwVer);

		return true;
	} else {
#if (SHOW_HIGHER_LEVEL_ERRORS)
		ShowLastError("TUSBDevice::ReadFirmewareVersion");
#endif  
		return false;
	}

}

/*
 * Reset Pipe.
 * FIXME: Don't know if it works with libusb. It needs some more testing. So disabled, 
 * just to be sure.
 */
bool
TUSBDevice::ResetPipe(int pipenum)
{
	BOOL result;

	result = false;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::ResetPipe(pipenum=%d)  \n",pipenum);

	puts("not yet!!! ResetPipe  USB_RESETPIPE ");

#ifdef IMPLEMENTED
	unsigned long nBytes;
	DWORD iBuf = pipenum;
	result = DeviceIoControl(USBDeviceHandle,
		IOCTL_SLBUSB_RESETPIPE,
		//IOCTL_SLBUSB_ABORTPIPE,
		&iBuf,
		sizeof(iBuf),
		NULL,
		0,
		&nBytes,
		NULL);
#endif

	if (!result) {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::ResetPipe");
#endif
		return false;
	}
	return true;
}

/*
 * Abort Pipe.
 * FIXME: Don't know if it works with libusb. It needs some more testing. So disabled, 
 * just to be sure.
 */
bool
TUSBDevice::AbortPipe(int pipenum)
{
	BOOL result;

	result = false;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::AbortPipe(pipenum=%d)  \n",pipenum);

#ifdef IMPLEMENTED
	unsigned long nBytes;
	DWORD iBuf = pipenum;
	result = DeviceIoControl(USBDeviceHandle,
		IOCTL_SLBUSB_ABORTPIPE,
		&iBuf,
		sizeof(iBuf),
		NULL,
		0,
		&nBytes,
		NULL);
#endif

	if (!result) {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::AbortPipe");
#endif  
		return false;
	}
	return true;
}

/*
 * This simulates an unplug and replug of the device.
 * FIXME: Don't know if it works with libusb. It needs some more testing. So disabled, 
 * just to be sure.
 */
bool
TUSBDevice::CyclePort()
{
	BOOL result;

	result = false;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::CyclePort()  \n");

#ifdef IMPLEMENTED
	unsigned long nBytes;
	result = DeviceIoControl(USBDeviceHandle,
		IOCTL_SLBUSB_CYCLE_PORT,
		//IOCTL_SLBUSB_RESET,
		NULL,
		0,
		NULL,
		0,
		&nBytes,
		NULL);
#endif
	if (!result) {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::CyclePort");
#endif  
		return false;
	}

	return true;
}

void
TUSBDevice::Hold8051()
{
	VENDOR_REQUEST_IN  myRequest;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::Hold8051()  \n");

	myRequest.bRequest   = VR_ANCHOR_DLD;   /* defines request handled by USB core */
	myRequest.wValue     = cpu_cs_reg;      /* 8051 control register */
	myRequest.wIndex     = 0;               /* not used */
	myRequest.wLength    = 1;               /* data length */
	myRequest.bData      = 1;               /* set reset bit; */
	myRequest.direction  = REQUEST_DIR_OUT; /* PC -> USB device */

	VendorRequest(&myRequest);
}

void
TUSBDevice::Run8051()
{
	VENDOR_REQUEST_IN  myRequest;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::Run8051()  \n");

	myRequest.bRequest   = VR_ANCHOR_DLD;   /* defines request handled by USB core */
	myRequest.wValue     = cpu_cs_reg;      /* 8051 control register */
	myRequest.wIndex     = 0;               /* not used */
	myRequest.wLength    = 1;               /* data length */
	myRequest.bData      = 0;               /* clear reset bit; */
	myRequest.direction  = REQUEST_DIR_OUT; /* PC -> USB device */

	VendorRequest(&myRequest);
}


bool
TUSBDevice::GetRevision(unsigned char *rev)
{
	VENDOR_REQUEST_IN  myRequest;
	bool status;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::GetRevision()  \n");

	/* request handled by USB core */
	if (Desc->idVendor == EZUSB_VENDOR_ID) {
		myRequest.bRequest = VR_ANCHOR_DLD;
	} else {
		myRequest.bRequest   = VR_GET_CHIP_REV;
	}

	myRequest.wValue     = cpu_cs_reg;      /* 8051 control register */
	myRequest.wIndex     = 0;               /* not used */
	myRequest.wLength    = 1;               /* data length */
	myRequest.bData      = 0;               /* readback data */
	myRequest.direction  = REQUEST_DIR_IN;  /* USB device -> PC */

	status = VendorRequest(&myRequest);

	if (status) {
		/* shift cpucs register to get rev. field */
		*rev = (unsigned char)((myRequest.bData >> 4) & 0x0F); // shift cpucs register to get rev. field
		return true;
	}
	else
		return false;
}

/* EEPROM data structure for firmware load:
 * 
 *   Address      Contents
 * ------------------------------------------------------
 *    0          0xB2 (0xC2), enables firmware download from EEPROM
 *    1          VID lb, Vendor ID
 *    2          VID hb
 *    3          PID lb, Product ID
 *    4          PID hb
 *    5          DID lb, Device ID
 *    6          DID hb
 *    7          config unsigned char (FX2LP only)
 * --------------------------------------------------------
 *     beginn of data records
 * ------------------------------------------------------
 *    7 (8)          Length hb, start of first data record
 *    8 (9)          Length lb
 *    9 (10)         Address hb
 *    10 (11)        Address lb
 *    11 (12)        first data unsigned char
 *    .           ...
 * -----------------------------------------------------
 *    more data records
 * -----------------------------------------------------
 * -----------------------------------------------------
 *    last data record (MSB of Length must be 1)
 *    write to CPUCS register to bring 8051 out of eset
 * -----------------------------------------------------
 *               0x80
 *               0x01
 *               0x7F  (0xE6)
 *               0x92  (0x00)
 *               0x00
 */
bool
TUSBDevice::LoadHexFileToEeprom(std::string FileName) {
	struct HEX_RECORDS   *Hex_Records=NULL;
	struct HEX_RECORDS   *entry, *entry_prev=NULL, *current=NULL;
	FILE                 *image;
	int i, status=0;

	size_t              data_len = 0;
	int lines=0, bytes=0;

	unsigned char  magicword;
	unsigned char  VendorDeviceID[6];
	unsigned char  LastRecord[6];
	unsigned short  addoffset;


	if (is_fx) {
		addoffset = 8;
		magicword = 0xc2;
	} else {
		addoffset = 7;
		magicword = 0xb2;
	}

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::LoadHexFileToEeprom(file=%s)  \n",FileName.c_str());

	image = fopen (FileName.c_str(), "r");

	for (;;) {
		char            buf [512], *cp;
		char            tmp, type;
		size_t          len;
		unsigned        idx, off;

		cp = fgets(buf, sizeof buf, image);
		if (cp == 0) {
			fprintf (stderr, "EOF without EOF record!\n");
			status = -1;
			
			break;
		}

		lines++;

		if (buf[0] == '#')   continue;

		if (buf[0] != ':') {
			fprintf (stderr, "not an ihex record: %s", buf);
			status = -2;
			
			break;
		}

		tmp = buf[3]; buf[3] = 0;  len = strtoul(buf+1, 0, 16);  buf[3] = tmp;
		tmp = buf[7]; buf[7] = 0;  off = strtoul(buf+3, 0, 16);  buf[7] = tmp;
		tmp = buf[9]; buf[9] = 0; type = (char)strtoul(buf+7, 0, 16);  buf[9] = tmp;

		if (type == 1) {
			if (SF_DEBUG_USB>1) printf("***>  Found EOF on hexfile\n");
			
			break;
		}

		if (type != 0) {
			fprintf (stderr, "unsupported record type: %u\n", type);
			status = -3;
			
			break;
		}

		if ((len * 2) + 11 >= strlen(buf)) {
			fprintf (stderr, "record too short?\n");
			status = -4;
			
			break;
		}

		if ((entry = (HEX_RECORDS*)calloc (1, sizeof *entry)) == 0) {
			status = -5; break;
		}

		entry->line_nr=lines;
		entry->HexRecord.Length=len;
		entry->HexRecord.Address=off;
		entry->HexRecord.Type=type;

		if (current)      current->next=entry;
		if (!Hex_Records) Hex_Records=entry;
		current=entry;

		for (idx = 0, cp = buf+9 ;  idx < len ;  idx += 1, cp += 2) {
			tmp = cp[2];   cp[2] = 0;
			entry->HexRecord.Data[idx]=(unsigned char)strtoul(cp, 0, 16);
			cp[2] = tmp;   bytes++;
		}
		data_len += len;
	}

	/* Write to EEPROM and free the memory */
	for (entry = Hex_Records; entry; entry = entry->next) {
		if(SF_DEBUG_USB>0) {
			printf(" write line=%03d Len=%02d  ADDR=%04X "
				,entry->line_nr, entry->HexRecord.Length, entry->HexRecord.Address);
			for (i=0; i<entry->HexRecord.Length; i++) printf(" %02X",entry->HexRecord.Data[i]);
			printf("\n");
		}
		if (status==0) {
			WriteDataRecordToEeprom(entry->HexRecord, addoffset); /* write data to EEPROM */
			addoffset += (unsigned short)(entry->HexRecord.Length + 4);
		}
		free(entry_prev);  entry_prev=entry;
	} free(entry_prev);  

	fclose(image);

	if (status==0) {
		WriteEEPROM(0, &magicword, 1);

		VendorDeviceID[0] = LSB(VendorId);
		VendorDeviceID[1] = MSB(VendorId);
		VendorDeviceID[2] = LSB(ProductId);
		VendorDeviceID[3] = MSB(ProductId);
		VendorDeviceID[4] = LSB(DeviceId);
		VendorDeviceID[5] = MSB(DeviceId);
		WriteEEPROM(1, VendorDeviceID, 6);

		if(is_fx) {
			unsigned char config = 0x01; /* connected, 400 kHz I2C */
			WriteEEPROM(7, &config, 1);
		}

		/* write last data record */
		LastRecord[0] = 0x80;
		LastRecord[1] = 0x01;
		LastRecord[2] = MSB(cpu_cs_reg);
		LastRecord[3] = LSB(cpu_cs_reg);
		LastRecord[4] = 0x00;
		WriteEEPROM(addoffset, LastRecord, 5); /* write last data to EEPROM */
	}

	if (status<0) return false;  else return true;
}

bool TUSBDevice::WriteDataRecordToEeprom(INTEL_HEX_RECORD HexRecordStruct, unsigned short Address)
{
	unsigned char *buffer;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::WriteDataRecordToEeprom()  \n");

	if (SF_DEBUG_USB>5) {
		printf("***>  Len=%02d  ADDR=%04X ",HexRecordStruct.Length, HexRecordStruct.Address);
		for (int i=0; i<HexRecordStruct.Length; i++) printf(" %02X",HexRecordStruct.Data[i]);
		printf("\n");
	}

	buffer = (unsigned char*) malloc (sizeof(((INTEL_HEX_RECORD*)(0))->Data));

	*buffer     = MSB(HexRecordStruct.Length);
	*(buffer+1) = LSB(HexRecordStruct.Length);
	*(buffer+2) = MSB(HexRecordStruct.Address);
	*(buffer+3) = LSB(HexRecordStruct.Address);

	for (int j = 0; j < (HexRecordStruct.Length & 0x03ff); j++) {
		*(buffer+4+j) = HexRecordStruct.Data[j]; /* set data */
	}

	if ((Address + HexRecordStruct.Length + 4) >= eeprom_user_data_offset) {
		ShowLastError("Error: EEPROM code space overlaps with user data!");

		free (buffer);
		return false;
	}

	WriteEEPROM(Address, buffer, (unsigned short)(HexRecordStruct.Length + 4));

	free (buffer);
	return true;
}

/* 
 * Configure port bits for xilinx configuration
 *
 *   signal    dir    func
 *   INIT     read    signals ready(1)/error(0)  (old USB card only !!!)
 *   RDWR     write   selects configuration write or readback (FX only)
 *   BUSY     read    reads whether S3 chip is busy and not accepting new data (FX only)
 *   PROG     write   clear config data, active low
 *   DONE     read    device active, active high
 *   CS1      write   selects express config mode, active high
 */
bool
TUSBDevice::InitXilinxConfPort()
{
	unsigned char portreg;

	/* configure bits on portc to I/O mode */
	if (!Read8051(xp_conf_port_cfg, &portreg, 1)) {
		return false;
	}


	/* select port i/o func. for all lines */
	if (is_fx) {
		portreg &= (unsigned char)~xp_rdwr;
		portreg &= (unsigned char)~xp_busy;
	} else {
		portreg &= ~xp_init;
	}

	portreg &= (unsigned char)~xp_prog;
	portreg &= (unsigned char)~xp_done;
	portreg &= (unsigned char)~xp_cs1;

	if (!Write8051(xp_conf_port_cfg, &portreg, 1)) {
		return false;
	}


	/* select direction on portc lines */
	if (!Read8051(xp_conf_port_oe, &portreg, 1)) {
		return false;
	}

	if (is_fx) {
		portreg |= (unsigned char) xp_rdwr;     /* write,  OE = 1 */
		portreg &= (unsigned char)~xp_busy;     /* read,  OE = 0 */
	} else {
		portreg &= (unsigned char)~xp_init;     /* read,  OE = 0 */
	}

	portreg |= (unsigned char) xp_prog;     /* write, OE = 1 */
	portreg &= (unsigned char)~xp_done;     /* read,  OE = 0 */
	portreg |= (unsigned char) xp_cs1;      /* write, OE = 1 */

	if (!Write8051(xp_conf_port_oe, &portreg, 1)) {
		return false;
	}

	return true;
}

bool
TUSBDevice::SetXilinxConfPin(unsigned char pin, unsigned char data)
{
	unsigned char portreg;

	if (!Read8051(xp_conf_port_rd, &portreg, 1)) {
		return false;
	}

	if ((data & 0x01) == 0) {
		portreg &= (unsigned char)~pin;
	} else {
		portreg |= (unsigned char) pin;
	}

	if (!Write8051(xp_conf_port_wr, &portreg, 1)) {
		return false;
	}

	return true;
}

bool
TUSBDevice::GetXilinxConfPin(unsigned char pin)
{
	unsigned char portreg;

	if (!Read8051(xp_conf_port_rd, &portreg, 1)) {
		return false;
	}

	return((portreg & pin) != 0);
}

bool
TUSBDevice::WriteXilinxConfData(unsigned char *data, int size)
{
	unsigned char dummy[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	SwapBytes(data, size);

	if (!WriteXilinx(0, data, size)) {
		return false;
	}

	WriteXilinx(0, dummy, 8);  /* eight extra clock to enable start-up */

	return true;
}

bool
TUSBDevice::SetXilinxConfByte(unsigned char data)
{
	if(!Write8051(xp_conf_port_wr, &data, 1)) {
		return false;
	}

	return true;
}

unsigned char
TUSBDevice::GetXilinxConfByte(void)
{
	unsigned char portreg;

	if (!Read8051(xp_conf_port_rd, &portreg, 1)) {
		return 0;
	}

	return(portreg);
}

bool
TUSBDevice::XilinxAlreadyLoaded()
{
	InitXilinxConfPort();

	return GetXilinxConfPin(xp_done);
}

bool
TUSBDevice::WriteXilinx(unsigned short address, unsigned char *Data, int length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::WriteXilinx(add=%d,data=%3d ..., len=%d)  \n",address,Data[0],length);

	sur.type   = SUR_TYPE_XILINX;
	sur.dir    = SUR_DIR_OUT;
	sur.addr   = address;
	sur.length = (unsigned short) length;

	if (length <= MTRANS) {
		status = SilabUsbRequest(&sur, Data);
	} else {
		status = SilabUsbRequest(&sur, Data, length);
	}

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::WriteXilinx");
#endif
		return false;
	}
}

bool
TUSBDevice::ReadXilinx(unsigned short address, unsigned char *Data, int Length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>3) printf("***> TUSBDevice::ReadXilinx(len=%d)  \n",Length);

	sur.type   = SUR_TYPE_XILINX;
	sur.dir    = SUR_DIR_IN;
	sur.addr   = address;
	sur.length = (unsigned short) Length;

	if (Length <= MTRANS) {
		status = SilabUsbRequest(&sur, Data);
	} else {
		status = SilabUsbRequest(&sur, Data, Length);
	}

	if (status) {
		return true;
	} else {
#if (SHOW_BASIC_ERRORS)
		ShowLastError("TUSBDevice::ReadXilinx");
#endif
		return false;
	}
}

bool
TUSBDevice::ConfigXilinx(unsigned char *Data, unsigned short length)
{
	SILAB_USB_REQUEST sur;
	bool status;

	if (SF_DEBUG_USB>0) printf("***> TUSBDevice::ConfigXilinx(len=%d)  \n",length);

	sur.type   = SUR_TYPE_XCONF;
	sur.dir    = SUR_DIR_OUT;
	sur.addr   = 0;
	sur.length = length;

	status = SilabUsbRequest(&sur, Data);

	if (status) {
		return true;
	} else {
#if (SHOW_HIGHER_LEVEL_ERRORS)
		ShowLastError("TUSBDevice::ConfigXilinx");
#endif  
		return false;
	}
}

bool TUSBDevice::WriteRegister(unsigned char * Data)
{
	bool status;

	/* minimum length of data array is 4 */
	status = WriteBulkEndpoint(SUR_DIRECT_OUT_PIPE, Data, 4);

	if (!status) {
		return false;
	}
	return true;
}

bool TUSBDevice::ReadRegister(unsigned char * Data)
{
	int nBytes;
	bool status;

	/* minimum length of data array is 4 */
	status = ReadBulkEndpoint(SUR_DIRECT_IN_PIPE, Data, 4, &nBytes);

	if (!status) {
		return false;
	}
	return true;
}

