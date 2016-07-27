#ifndef LIBSIUSB_USBCOMPAT_H
#define LIBSIUSB_USBCOMPAT_H

#define USHORT unsigned short int
#define ULONG unsigned long int
#define LONG long int
#define BOOL bool
#define UCHAR unsigned char
#define WCHAR unsigned short int
#define PVOID void *
#define PULONG unsigned long*

#define HANDLE libusb_device_handle*

/* typedef unsigned char UCHAR; */
/* typedef unsigned short USHORT; */
/* typedef unsigned long int ULONG; */
/* typedef long int LONG; */
/* typedef void* PVOID; */
/* typedef bool BOOL; */
/* typedef unsigned short int WCHAR; */
/* typedef unsigned long* PULONG; */
/* typedef libusb_device_handle* HANDLE; */

#ifndef WIN32
#define INVALID_HANDLE_VALUE NULL
#endif
#define __stdcall

typedef PVOID WINUSB_INTERFACE_HANDLE, *PWINUSB_INTERFACE_HANDLE;
typedef PVOID USBD_PIPE_HANDLE;

typedef struct _WINUSB_SETUP_PACKET {
  UCHAR   RequestType;
  UCHAR   Request;
  USHORT  Value;
  USHORT  Index;
  USHORT  Length;
} WINUSB_SETUP_PACKET, *PWINUSB_SETUP_PACKET;

typedef enum _USBD_PIPE_TYPE {
  UsbdPipeTypeControl,
  UsbdPipeTypeIsochronous,
  UsbdPipeTypeBulk,
  UsbdPipeTypeInterrupt
} USBD_PIPE_TYPE;

typedef struct _USBD_PIPE_INFORMATION {
  USHORT MaximumPacketSize;
  UCHAR EndpointAddress;
  UCHAR Interval;
  USBD_PIPE_TYPE PipeType;
  USBD_PIPE_HANDLE PipeHandle;
  ULONG MaximumTransferSize;
  ULONG PipeFlags;
} USBD_PIPE_INFORMATION, *PUSBD_PIPE_INFORMATION;

#endif //LIBSIUSB_USBCOMPAT_H
