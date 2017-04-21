#ifdef WIN32
#define SLBUSB_IOCTL_INDEX 5
#define _IOWR(a, b, c) b
#endif

/* Struct for linux, keeped for compatibilty */
typedef struct __InterfaceInfo {
	unsigned char InterfaceInfo[1024];
} InterfaceInfo2;

#ifndef WIN32
#define SLBUSB_IOCTL_INDEX 0
#endif

//#define IOCTL_SLBUSB_GET_PIPE_INFO                       _IOWR('U', SLBUSB_IOCTL_INDEX+1, InterfaceInfo) 
#define IOCTL_SLBUSB_GET_PIPE_INFO                       _IOWR('U', SLBUSB_IOCTL_INDEX+1, InterfaceInfo2) 
#define IOCTL_SLBUSB_GET_DEVICE_DESCRIPTOR               _IOWR('U', SLBUSB_IOCTL_INDEX+2, Usb_Device_Descriptor) 
#define IOCTL_SLBUSB_GET_CONFIGURATION_DESCRIPTOR        _IOWR('U', SLBUSB_IOCTL_INDEX+3, struct param) 
#define IOCTL_SLBUSB_BULK_OR_INTERRUPT_WRITE             _IOWR('U', SLBUSB_IOCTL_INDEX+4, struct param) 
#define IOCTL_SLBUSB_BULK_OR_INTERRUPT_READ              _IOWR('U', SLBUSB_IOCTL_INDEX+5, struct param) 
#define IOCTL_SLBUSB_VENDOR_REQUEST                      _IOWR('U', SLBUSB_IOCTL_INDEX+6, VENDOR_REQUEST_IN) 
#define IOCTL_SLBUSB_GET_CURRENT_CONFIG                  _IOWR('U', SLBUSB_IOCTL_INDEX+7, struct param) 
#define IOCTL_SLBUSB_RESET                               _IOWR('U', SLBUSB_IOCTL_INDEX+8, struct param) 
#define IOCTL_SLBUSB_RESETPIPE                           _IOWR('U', SLBUSB_IOCTL_INDEX+9, struct param) 
#define IOCTL_SLBUSB_ABORTPIPE                           _IOWR('U', SLBUSB_IOCTL_INDEX+10, struct param) 
#define IOCTL_SLBUSB_SETINTERFACE                        _IOWR('U', SLBUSB_IOCTL_INDEX+11, struct param) 
#define IOCTL_SLBUSB_GET_STRING_DESCRIPTOR               _IOWR('U', SLBUSB_IOCTL_INDEX+12, Usb_String_Descriptor) 
#define IOCTL_SLBUSB_BULK_READ                           _IOWR('U', SLBUSB_IOCTL_INDEX+13, BULK_IOCTL) 
#define IOCTL_SLBUSB_BULK_WRITE                          _IOWR('U', SLBUSB_IOCTL_INDEX+14, BULK_IOCTL) 
#define IOCTL_SLBUSB_GET_CURRENT_FRAME_NUMBER            _IOWR('U', SLBUSB_IOCTL_INDEX+15, struct param) 
#define IOCTL_SLBUSB_VENDOR_OR_CLASS_REQUEST             _IOWR('U', SLBUSB_IOCTL_INDEX+16, struct param) 
#define IOCTL_SLBUSB_GET_LAST_ERROR                      _IOWR('U', SLBUSB_IOCTL_INDEX+17, struct param) 
#define IOCTL_SLBUSB_ISO_READ                            _IOWR('U', SLBUSB_IOCTL_INDEX+18, struct param) 
#define IOCTL_SLBUSB_ISO_WRITE                           _IOWR('U', SLBUSB_IOCTL_INDEX+19, struct param) 
#define IOCTL_SLBUSB_ANCHOR_DOWNLOAD                     _IOWR('U', SLBUSB_IOCTL_INDEX+20, BULK_IOCTL) 
#define IOCTL_SLBUSB_CYCLE_PORT                          _IOWR('U', SLBUSB_IOCTL_INDEX+21, struct param) 
#define IOCTL_SLBUSB_GET_DRIVER_VERSION                  _IOWR('U', SLBUSB_IOCTL_INDEX+22, struct param) 
#define IOCTL_SLBUSB_START_ISO_STREAM                    _IOWR('U', SLBUSB_IOCTL_INDEX+23, struct param) 
#define IOCTL_SLBUSB_STOP_ISO_STREAM                     _IOWR('U', SLBUSB_IOCTL_INDEX+24, struct param) 
#define IOCTL_SLBUSB_READ_ISO_BUFFER                     _IOWR('U', SLBUSB_IOCTL_INDEX+25, struct param) 

#define IOCTL_SLBUSB_RESET_CPU                           _IOWR('U', SLBUSB_IOCTL_INDEX+35, BULK_IOCTL) 


/* control structure for bulk and interrupt data transfers */
typedef struct _BULK_TRANSFER_CONTROL {
	ULONG pipeNum;
} BULK_TRANSFER_CONTROL, *PBULK_TRANSFER_CONTROL;

#define MAX_BULK_IOCTL 16*1024-6*4

typedef struct _bulk_8192 { 
	int   IOCTL_SLBUSB;
	BULK_TRANSFER_CONTROL Control;
	int   sizeControl;
	char  ioBuffer[MAX_BULK_IOCTL]; /* FIXME */
	int   sizeBuffer;
	unsigned long  doneBytes;
} BULK_8192;

typedef struct __bulk_ioctl { 
	int    IOCTL_SLBUSB;
	void*  Control;
	int    sizeControl;
	void*  ioBuffer; //---  fsv FIXME!!!
	int    sizeBuffer;
	unsigned long  doneBytes;
} BULK_IOCTL;

#define MAX_INTEL_HEX_RECORD_LENGTH 16
typedef struct _INTEL_HEX_RECORD {
	BYTE  Length;
	WORD  Address;
	BYTE  Type;
	BYTE  Data[MAX_INTEL_HEX_RECORD_LENGTH];
} INTEL_HEX_RECORD, *PINTEL_HEX_RECORD;

struct HEX_RECORDS {
	struct   HEX_RECORDS   *next;
	int      line_nr;
	INTEL_HEX_RECORD      HexRecord;
};

typedef struct _VENDOR_REQUEST_IN {
	BYTE    bRequest;
	WORD    wValue;
	WORD    wIndex;
	WORD    wLength;
	BYTE    direction;
	BYTE    bData;
} VENDOR_REQUEST_IN, *PVENDOR_REQUEST_IN;

typedef struct _GET_STRING_DESCRIPTOR_IN {
	UCHAR    Index;
	USHORT   LanguageId;
} GET_STRING_DESCRIPTOR_IN, *PGET_STRING_DESCRIPTOR_IN;

/* control structure for sending vendor or class specific requests
 * to the control endpoint.
 */
typedef struct _VENDOR_OR_CLASS_REQUEST_CONTROL {
	/* transfer direction (0=host to device, 1=device to host) */
	UCHAR direction;

	/* request type (1=class, 2=vendor) */
	UCHAR requestType;

	/* recipient (0=device,1=interface,2=endpoint,3=other) */
	UCHAR recepient;

	/* see the USB Specification for an explanation of the
	 * following paramaters.
	 */
	UCHAR requestTypeReservedBits;
	UCHAR request;
	USHORT value;
	USHORT index;
} VENDOR_OR_CLASS_REQUEST_CONTROL, *PVENDOR_OR_CLASS_REQUEST_CONTROL;


/* control structure for isochronous data transfers */
typedef struct _ISO_TRANSFER_CONTROL
{
	/* pipe number to perform the ISO transfer to/from.  Direction is
	 * implied by the pipe number.
	 */
	ULONG PipeNum;

	/* ISO packet size.  Determines how much data is transferred each
	 * frame.  Should be less than or equal to the maxpacketsize for
	 * the endpoint.
	 */
	ULONG PacketSize;

	/* Total number of ISO packets to transfer. */
	ULONG PacketCount;

	/* The following two parameters detmine how buffers are managed for
	 * an ISO transfer.  In order to maintain an ISO stream, the driver
	 * must create at least 2 transfer buffers and ping pong between them.
	 * BufferCount determines how many buffers the driver creates to ping
	 * pong between.  FramesPerBuffer specifies how many USB frames of data
	 * are transferred by each buffer.
	 */
	ULONG FramesPerBuffer;     /* 10 is a good value */
	ULONG BufferCount;         /* 2 is a good value */
} ISO_TRANSFER_CONTROL, *PISO_TRANSFER_CONTROL;

