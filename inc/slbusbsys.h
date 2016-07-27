//////////////////////////////////////////////////////////////////////
//
// File:      slbusbsys.h
// $Archive: /visualc/slbusbsys.h $
//
// Purpose:
//    Header file for the SiLaBo USB Device Driver
//
// Environment:
//    kernel mode
//
// $Author: Hans $
//
// $History: slbusbsys.h $
//
//
// Based on ezusb.h from Anchor Chips EZ-USB
//
// *****************  Version 0.2  *****************
// 18.01.01  HK
// Added SLB read/write functions
//
// *****************  Version 0.1  *****************
// 05.01.01  HK
// Build test with MSVC 6.0
//////////////////////////////////////////////////////////////////////
#ifndef SLBUSBSYSH
#define SLBUSBSYSH
//
// Vendor specific request code for Anchor Upload/Download
//
// This one is implemented in the core
//
#define ANCHOR_LOAD_INTERNAL  0xA0

//
// This command is not implemented in the core.  Requires firmware
//
#define ANCHOR_LOAD_EXTERNAL  0xA3

//
// This is the highest internal RAM address for the AN2131Q
//
#define MAX_INTERNAL_ADDRESS  0x1B3F

#define INTERNAL_RAM(address) ((address <= MAX_INTERNAL_ADDRESS) ? 1 : 0)

#ifndef struct
#define struct struct
#endif



typedef struct _VENDOR_REQUEST_IN
{
    unsigned char    bRequest;
    unsigned short   wValue;
    unsigned short   wIndex;
    unsigned short   wLength;
    unsigned char    direction;
    unsigned char    bData;
} VENDOR_REQUEST_IN, *PVENDOR_REQUEST_IN;

///////////////////////////////////////////////////////////
//
// control structure for bulk and interrupt data transfers
//
///////////////////////////////////////////////////////////
typedef struct _BULK_TRANSFER_CONTROL
{
  unsigned int pipeNum;
} BULK_TRANSFER_CONTROL, *PBULK_TRANSFER_CONTROL;

typedef struct _BULK_LATENCY_CONTROL
{
   unsigned int bulkPipeNum;
   unsigned int intPipeNum;
   unsigned int loops;
} BULK_LATENCY_CONTROL, *PBULK_LATENCY_CONTROL;


///////////////////////////////////////////////////////////
//
// control structure isochronous loopback test
//
///////////////////////////////////////////////////////////
typedef struct _ISO_LOOPBACK_CONTROL
{
   // iso pipe to write to
   unsigned int outPipeNum;

   // iso pipe to read from
   unsigned int inPipeNum;

   // amount of data to read/write from/to the pipe each frame.  If not
   // specified, the MaxPacketSize of the out pipe is used.
   unsigned int packetSize;

} ISO_LOOPBACK_CONTROL, *PISO_LOOPBACK_CONTROL;

///////////////////////////////////////////////////////////
//
// control structure for sending vendor or class specific requests
// to the control endpoint.
//
///////////////////////////////////////////////////////////
typedef struct _VENDOR_OR_CLASS_REQUEST_CONTROL
{
   // transfer direction (0=host to device, 1=device to host)
   unsigned char direction;

   // request type (1=class, 2=vendor)
   unsigned char requestType;

   // recipient (0=device,1=interface,2=endpoint,3=other)
   unsigned char recepient;
   //
   // see the USB Specification for an explanation of the
   // following paramaters.
   //
   unsigned char requestTypeReservedBits;
   unsigned char request;
   unsigned short  value;
   unsigned short  index;
} VENDOR_OR_CLASS_REQUEST_CONTROL, *PVENDOR_OR_CLASS_REQUEST_CONTROL;

///////////////////////////////////////////////////////////
//
// control structure for isochronous data transfers
//
///////////////////////////////////////////////////////////
typedef struct _ISO_TRANSFER_CONTROL
{
   //
   // pipe number to perform the ISO transfer to/from.  Direction is
   // implied by the pipe number.
   //
   unsigned int PipeNum;
   //
   // ISO packet size.  Determines how much data is transferred each
   // frame.  Should be less than or equal to the maxpacketsize for
   // the endpoint.
   //
   unsigned int PacketSize;
   //
   // Total number of ISO packets to transfer.
   //
   unsigned int PacketCount;
   //
   // The following two parameters detmine how buffers are managed for
   // an ISO transfer.  In order to maintain an ISO stream, the driver
   // must create at least 2 transfer buffers and ping pong between them.
   // BufferCount determines how many buffers the driver creates to ping
   // pong between.  FramesPerBuffer specifies how many USB frames of data
   // are transferred by each buffer.
   //
   unsigned int FramesPerBuffer;     // 10 is a good value
   unsigned int BufferCount;         // 2 is a good value
} ISO_TRANSFER_CONTROL, *PISO_TRANSFER_CONTROL;


///////////////////////////////////////////////////////////
//
// control structure for Anchor Downloads
//
///////////////////////////////////////////////////////////
typedef struct _ANCHOR_DOWNLOAD_CONTROL
{
   unsigned short  Offset;
} ANCHOR_DOWNLOAD_CONTROL, *PANCHOR_DOWNLOAD_CONTROL;

#define MAX_INTEL_HEX_RECORD_LENGTH 16

typedef struct _INTEL_HEX_RECORD
{
   unsigned char  Length;
   unsigned short   Address;
   unsigned char  Type;
   unsigned char  Data[MAX_INTEL_HEX_RECORD_LENGTH];
} INTEL_HEX_RECORD, *PINTEL_HEX_RECORD;

struct HEX_RECORDS {
	struct   HEX_RECORDS   *next;
        int      line_nr;
	INTEL_HEX_RECORD      HexRecord;
};

typedef struct _SET_INTERFACE_IN
{
   unsigned char interfaceNum;
   unsigned char alternateSetting;
} SET_INTERFACE_IN, *PSET_INTERFACE_IN;

typedef struct _GET_STRING_DESCRIPTOR_IN
{
   unsigned char    Index;
   unsigned short    LanguageId;
} GET_STRING_DESCRIPTOR_IN, *PGET_STRING_DESCRIPTOR_IN;

typedef struct _SLBUSB_DRIVER_VERSION
{
   unsigned short      MajorVersion;
   unsigned short      MinorVersion;
   unsigned short      BuildVersion;
} SLBUSB_DRIVER_VERSION, *PSLBUSB_DRIVER_VERSION;

#ifdef DRIVER

typedef struct _RING_BUFFER
{
   Punsigned char      inPtr;
   Punsigned char      outPtr;
   unsigned int       totalSize;
   unsigned int       currentSize;
   KSPIN_LOCK	spinLock;
   Punsigned char      buffer;
} RING_BUFFER, *PRING_BUFFER;

PRING_BUFFER
AllocRingBuffer(
   unsigned int    Size
   );

VOID
FreeRingBuffer(
   PRING_BUFFER   ringBuffer
   );

unsigned int
ReadRingBuffer(
   PRING_BUFFER   ringBuffer,
   Punsigned char         readBuffer,
   unsigned int          numberOfBytesToRead
   );
   
unsigned int
WriteRingBuffer(
   PRING_BUFFER   ringBuffer,
   Punsigned char         writeBuffer,
   unsigned int          numberOfBytesToWrite
   );

typedef struct _SLBUSB_FIRMWARE
{
   // tag contains a string to identify the start of the firmware
   // image in the driver binary.  Another utilty can then be used
   // to replace the firmware image inthe driver without requiring
   // a recompile
   unsigned char tag[10];
   unsigned int size;
   unsigned char firmware[];
} SLBUSB_FIRMWARE, *PSLBUSB_FIRMWARE;

// 
// this is the default number of IRP's to queue for streaming ISO
// data.
//
#define DEFAULT_ISO_BUFFER_COUNT 2

//
// Default number of frames of ISO data transferred by a single ISO
// URB/IRP
//
#define DEFAULT_ISO_FRAMES_PER_BUFFER 10

typedef struct _ISO_STREAM_OBJECT ISO_STREAM_OBJECT, *PISO_STREAM_OBJECT;

typedef struct _ISO_TRANSFER_OBJECT
{
   unsigned int Frame;
   PISO_STREAM_OBJECT StreamObject;
   PURB Urb;
   PIRP Irp;
   KEVENT Done;
} ISO_TRANSFER_OBJECT, *PISO_TRANSFER_OBJECT;

typedef struct _ISO_STREAM_OBJECT
{
   PDEVICE_OBJECT DeviceObject;
   unsigned int PacketSize;
   unsigned int NumPackets;
   PUSBD_PIPE_INFORMATION PipeInfo;
   PVOID TransferBuffer;
   unsigned int TransferBufferLength;
   PVOID IsoDescriptorBuffer;
   unsigned int FramesPerBuffer;
   unsigned int BufferCount;
   unsigned int PendingTransfers;
   PRING_BUFFER DataRingBuffer;
   PRING_BUFFER DescriptorRingBuffer;
   PISO_TRANSFER_OBJECT TransferObject;
} ISO_STREAM_OBJECT, *PISO_STREAM_OBJECT;


#define SlbUsb_NAME_MAX  64




//
// This is an unused structure in this driver, but is provided here
// so when you extend the driver to deal with USB pipes, you may wish
// to use this structure as an example or model.
//
typedef struct _SLBUSB_PIPE {
   unsigned int Mode;
   unsigned int Option;
   unsigned int Param1;
   unsigned int Param2;
   wchar_t Name[SlbUsb_NAME_MAX];
   PUSBD_PIPE_INFORMATION PipeInfo;
} SLBUSB_PIPE, *PSLBUSB_PIPE;


/*
// The interface number on this device that this driver expects to use
// This would be in the bInterfaceNumber field of the Interface Descriptor, hence
// this device driver would need to know this value.
*/
#define SAMPLE_INTERFACE_NBR 0x00


//
// A structure representing the instance information associated with
// this particular device.
//
typedef struct _DEVICE_EXTENSION
{

   // physical device object
   PDEVICE_OBJECT PhysicalDeviceObject;        

   // Device object we call when submitting Urbs/Irps to the USB stack
   PDEVICE_OBJECT		StackDeviceObject;		

   // Indicates that we have recieved a STOP message
   BOOLEAN Stopped;

   // Indicates that we are enumerated and configured.  Used to hold
   // of requests until we are ready for them
   BOOLEAN Started;

   // Indicates the device needs to be cleaned up (ie., some configuration
   // has occurred and needs to be torn down).
   BOOLEAN NeedCleanup;

   // configuration handle for the configuration the
   // device is currently in
   USBD_CONFIGURATION_HANDLE ConfigurationHandle;

   // ptr to the USB device descriptor
   // for this device
   PUSB_DEVICE_DESCRIPTOR DeviceDescriptor;

   // we support up to one interface
   PUSBD_INTERFACE_INFORMATION Interface;

   // the number of device handles currently open to the device object.
   // Gets incremented by Create and decremented by Close
   unsigned int                OpenHandles;

   // Name buffer for our named Functional device object link
   wchar_t DeviceLinkNameBuffer[SlbUsb_NAME_MAX];

   // This member is used to store the URB status of the
   // most recently failed URB.  If a USB transfer fails, a caller
   // can use IOCTL_SLBUSB_GET_LAST_ERROR to retrieve this value.
   // There's only room for one, so you better get it quick (or at
   // least before the next URB failure occurs).
   USBD_STATUS LastFailedUrbStatus;

   // use counter for the device.  Gets incremented when the driver receives
   // a request and gets decremented when a request s completed.
   long usage;

   // this ev gets set when it is ok to remove the device
	KEVENT evRemove;

   // TRUE if we're trying to remove this device
   BOOLEAN removing;

   BOOLEAN StopIsoStream;

   PRING_BUFFER DataRingBuffer;
   PRING_BUFFER DescriptorRingBuffer;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


#if DBG

#define SlbUsb_KdPrint(_x_) DbgPrint("SlbUsb.sys: "); \
                             DbgPrint _x_ ;
#define TRAP() DbgBreakPoint()
#else
#define SlbUsb_KdPrint(_x_)
#define TRAP()
#endif


NTSTATUS
SlbUsb_Dispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

VOID
SlbUsb_Unload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
SlbUsb_StartDevice(
    IN  PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
SlbUsb_StopDevice(
    IN  PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
SlbUsb_RemoveDevice(
    IN  PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
SlbUsb_CallUSBD(
    IN PDEVICE_OBJECT DeviceObject,
    IN PURB Urb
    );

NTSTATUS
SlbUsb_PnPAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

NTSTATUS
SlbUsb_CreateDeviceObject(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT *DeviceObject,
    long Instance
    );

NTSTATUS
SlbUsb_ConfigureDevice(
    IN  PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
SlbUsb_Create(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SlbUsb_Close(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SlbUsb_Read_Write(
   IN  PDEVICE_OBJECT DeviceObject,
   IN  PIRP Irp
   );

NTSTATUS
SlbUsb_ProcessIOCTL(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
SlbUsb_SelectInterfaces(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    IN PUSBD_INTERFACE_INFORMATION Interface
    );

NTSTATUS
SlbUsb_ResetPipe(
   IN PDEVICE_OBJECT DeviceObject,
   IN unsigned int PipeNum
   );

NTSTATUS
SlbUsb_AbortPipe(
    IN PDEVICE_OBJECT DeviceObject,
    IN USBD_PIPE_HANDLE PipeHandle
    );

unsigned int
SlbUsb_GetCurrentFrameNumber(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
SlbUsb_Read_Write_Direct(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PIRP Irp,
    IN  BOOLEAN Read
    );

unsigned int
SlbUsb_DownloadTest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVENDOR_REQUEST_IN pVendorRequest
    );

NTSTATUS
SlbUsb_ResetParentPort(
    IN IN PDEVICE_OBJECT DeviceObject
    );

VOID
SlbUsb_Cleanup(
    PDEVICE_OBJECT DeviceObject
    );

unsigned int
SlbUsb_GetDeviceDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    PVOID             pvOutputBuffer
    );

unsigned int
SlbUsb_GetConfigDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    PVOID             pvOutputBuffer,
    unsigned int             ulngth
    );

unsigned int
SlbUsb_VendorRequest(
    IN PDEVICE_OBJECT DeviceObject,
    PVENDOR_REQUEST_IN pVendorRequest
    );

unsigned int
SlbUsb_GetCurrentConfig(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVENDOR_REQUEST_IN pVendorRequest
    );

unsigned int
SlbUsb_GetCurrentInterface(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVENDOR_REQUEST_IN pVendorRequest
    );

PUSB_CONFIGURATION_DESCRIPTOR
GetConfigDescriptor(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
ConfigureDevice(
    IN  PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
SetInterface(
   IN PDEVICE_OBJECT DeviceObject,
   IN unsigned char InterfaceNumber,
   IN unsigned char AlternateSetting
   );

unsigned int
SlbUsb_GetStringDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    unsigned char             Index,
    unsigned short             LanguageId,
    PVOID             pvOutputBuffer,
    unsigned int             ulLength
    );

NTSTATUS
SlbUsb_VendorRequest2(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );


NTSTATUS
ForwardAndWait(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );


NTSTATUS
SlbUsb_DefaultPnpHandler(
   IN PDEVICE_OBJECT fdo,
   IN PIRP Irp
   );


NTSTATUS
SlbUsb_DispatchPnp(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   );

NTSTATUS
SlbUsb_DispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
SlbUsb_HandleStartDevice(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp
   );

NTSTATUS
SlbUsb_HandleRemoveDevice(
   IN PDEVICE_OBJECT fdo,
   IN PIRP Irp
   );

NTSTATUS 
OnRequestComplete(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp,
   IN PKEVENT pev
   );

NTSTATUS
CompleteRequest(
   IN PIRP Irp,
   IN NTSTATUS status,
   IN unsigned int info
   );

BOOLEAN LockDevice(
   IN PDEVICE_OBJECT fdo
   );

void UnlockDevice(
   PDEVICE_OBJECT fdo
   );


NTSTATUS InitTransferObject(
   IN OUT PISO_STREAM_OBJECT streamObject,
   IN unsigned int index
   );

NTSTATUS SlbUsb_StartIsoTransfer(
   IN PDEVICE_OBJECT fdo,
   IN PIRP Irp
   );

NTSTATUS IsoTransferComplete(
   IN PDEVICE_OBJECT fdo,
   IN PIRP Irp,
   IN PVOID Context
   );


NTSTATUS SlbUsb_AnchorDownload(
   PDEVICE_OBJECT fdo,
   unsigned short  offset,
   Punsigned char downloadBuffer,
   unsigned int downloadSize
   );

NTSTATUS SlbUsb_DownloadIntelHex(
   PDEVICE_OBJECT fdo,
   PINTEL_HEX_RECORD hexRecord
   );

NTSTATUS SlbUsb_8051Reset(
   PDEVICE_OBJECT fdo,
   unsigned char resetBit
   );

NTSTATUS SlbUsb_StartIsoStream(
   IN PDEVICE_OBJECT fdo,
   IN PIRP Irp
   );

NTSTATUS SlbUsb_CyclePort(IN PDEVICE_OBJECT DeviceObject);


#endif      //DRIVER section


///////////////////////////////////////////////////////
//
//              IOCTL Definitions
//
// User mode applications wishing to send IOCTLs to a kernel mode driver
// must use this file to set up the correct type of IOCTL code permissions.
//
// Note: this file depends on the file DEVIOCTL.H which contains the macro
// definition for "CTL_CODE" below.  Include that file before  you include
// this one in your source code. DEVIOCTL.H is a Microsoft header file.
// DEVIOCTL.H is normally obtained by installing the Windows DDK.
//
///////////////////////////////////////////////////////

//
// Set the base of the IOCTL control codes.  This is somewhat of an
// arbitrary base number, so you can change this if you want unique
// IOCTL codes.  You should consult the Windows NT DDK for valid ranges
// of IOCTL index codes before you choose a base index number.
//

#define SLBUSB_IOCTL_INDEX  0x0800


#define IOCTL_SLBUSB_GET_PIPE_INFO     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+0,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_GET_DEVICE_DESCRIPTOR CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+1,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_GET_CONFIGURATION_DESCRIPTOR CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+2,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_BULK_OR_INTERRUPT_WRITE     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+3,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_BULK_OR_INTERRUPT_READ      CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+4,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_VENDOR_REQUEST              CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+5,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_GET_CURRENT_CONFIG          CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+6,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_ANCHOR_DOWNLOAD             CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+7,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_RESET  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+12,\
                                                   METHOD_IN_DIRECT,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_RESETPIPE  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+13,\
                                                   METHOD_IN_DIRECT,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_ABORTPIPE  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+15,\
                                                   METHOD_IN_DIRECT,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_SETINTERFACE  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+16,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_GET_STRING_DESCRIPTOR CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+17,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)


//
// Perform an IN transfer over the specified bulk or interrupt pipe.
//
// lpInBuffer: BULK_TRANSFER_CONTROL stucture specifying the pipe number to read from
// nInBufferSize: sizeof(BULK_TRANSFER_CONTROL)
// lpOutBuffer: Buffer to hold data read from the device.  
// nOutputBufferSize: size of lpOutBuffer.  This parameter determines
//    the size of the USB transfer.
// lpBytesReturned: actual number of unsigned chars read
// 
#define IOCTL_SLBUSB_BULK_READ             CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+19,\
                                                   METHOD_OUT_DIRECT,  \
                                                   FILE_ANY_ACCESS)

//
// Perform an OUT transfer over the specified bulk or interrupt pipe.
//
// lpInBuffer: BULK_TRANSFER_CONTROL stucture specifying the pipe number to write to
// nInBufferSize: sizeof(BULK_TRANSFER_CONTROL)
// lpOutBuffer: Buffer of data to write to the device
// nOutputBufferSize: size of lpOutBuffer.  This parameter determines
//    the size of the USB transfer.
// lpBytesReturned: actual number of unsigned chars written
// 
#define IOCTL_SLBUSB_BULK_WRITE            CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+20,\
                                                   METHOD_IN_DIRECT,  \
                                                   FILE_ANY_ACCESS)

//
// The following IOCTL's are defined as using METHOD_DIRECT_IN buffering.
// This means that the output buffer is directly mapped into system
// space and probed for read access by the driver.  This means that it is
// brought into memory if it happens to be paged out to disk.  Even though
// the buffer is only probed for read access, it is safe (probably) to
// write to it as well.  This read/write capability is used for the loopback
// IOCTL's
// 

// TODO Insert Loopback IOCTL's

//
// Retrieve the current USB frame number from the Host Controller
//
// lpInBuffer: NULL
// nInBufferSize: 0
// lpOutBuffer: Punsigned int to hold current frame number
// nOutputBufferSize: sizeof(Punsigned int)
// 
#define IOCTL_SLBUSB_GET_CURRENT_FRAME_NUMBER  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+21,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)


//
// Performs a vendor or class specific control transfer to EP0.  The contents of
// the input parameter determine the type of request.  See the USB spec
// for more information on class and vendor control transfers.
//
// lpInBuffer: PVENDOR_OR_CLASS_REQUEST_CONTROL
// nInBufferSize: sizeof(VENDOR_OR_CLASS_REQUEST_CONTROL)
// lpOutBuffer: pointer to a buffer if the request involves a data transfer
// nOutputBufferSize: size of the transfer buffer (corresponds to the wLength
//    field of the USB setup packet)
// 
#define IOCTL_SLBUSB_VENDOR_OR_CLASS_REQUEST   CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+22,\
                                                   METHOD_IN_DIRECT,  \
                                                   FILE_ANY_ACCESS)

//
// Retrieves the actual USBD_STATUS code for the most recently failed
// URB.
//
// lpInBuffer: NULL
// nInBufferSize: 0
// lpOutBuffer: Punsigned int to hold the URB status
// nOutputBufferSize: sizeof(unsigned int)
// 

#define IOCTL_SLBUSB_GET_LAST_ERROR   CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+23,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

//
// Reads from the specified ISO endpoint. (USB IN Transfer)
//
// lpInBuffer: ISO_TRANSFER_CONTROL
// nInBufferSize: sizeof(ISO_TRANSFER_CONTROL)
// lpOutBuffer: buffer to hold data read from the device
// nOutputBufferSize: size of the read buffer.
//
// 
// 

#define IOCTL_SLBUSB_ISO_READ          CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+25,\
                                                   METHOD_OUT_DIRECT,  \
                                                   FILE_ANY_ACCESS)

//
// Writes to the specified ISO endpoint. (USB OUT Transfer)
//
// lpInBuffer: ISO_TRANSFER_CONTROL
// nInBufferSize: sizeof(ISO_TRANSFER_CONTROL)
// lpOutBuffer: buffer to hold data to write to the device
// nOutputBufferSize: size of the write buffer.
//
// 
// 

#define IOCTL_SLBUSB_ISO_WRITE          CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+26,\
                                                   METHOD_IN_DIRECT,  \
                                                   FILE_ANY_ACCESS)
/*
//
// Performs and Anchor Download.  *** OUTDATED ***
//
// lpInBuffer: PANCHOR_DOWNLOAD_CONTROL
// nInBufferSize: sizeof(ANCHOR_DOWNLOAD_CONTROL)
// lpOutBuffer: pointer to a buffer of data to download to the device
// nOutputBufferSize: size of the transfer buffer
// 
#define IOCTL_SLBUSB_ANCHOR_DOWNLOAD   CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+27,\
                                                   METHOD_IN_DIRECT,  \
                                                   FILE_ANY_ACCESS)
*/



// 
// Perform emulation of port unplug and re-plug
//
#define IOCTL_SLBUSB_CYCLE_PORT           CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+27,\
                                                   METHOD_IN_DIRECT,  \
                                                   FILE_ANY_ACCESS)


//
// Returns driver version information
//
// lpInBuffer: NULL
// nInBufferSize: 0
// lpOutBuffer: PSLBUSB_DRIVER_VERSION
// nOutputBufferSize: sizeof(SLBUSB_DRIVER_VERSION)
// 
#define IOCTL_SLBUSB_GET_DRIVER_VERSION   CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+29,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_START_ISO_STREAM     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+30,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_STOP_ISO_STREAM     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+31,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_SLBUSB_READ_ISO_BUFFER     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   SLBUSB_IOCTL_INDEX+32,\
                                                   METHOD_OUT_DIRECT,  \
                                                   FILE_ANY_ACCESS)

#endif
