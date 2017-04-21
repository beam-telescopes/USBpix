#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <errno.h>

#ifdef FreeBSD
#include <libusb.h>
#else
#include "include/libusb.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <ftw.h>
#endif

#define SF_DEBUG_USB 0

//------------------------------
#define USHORT unsigned short int
#define ULONG unsigned long int
#define LONG long int
#define _WORD_DEFINED
#define WORD unsigned short int
#define _BYTE_DEFINED
#define BYTE unsigned char
#define byte unsigned char
#define BOOL bool
#define UCHAR unsigned char
#define WCHAR unsigned short int
#define PVOID void *
#define PULONG unsigned long*
#define AnsiString std::string
// #define LPVOID void*
#define Length() length()
#ifndef WIN32
#define INVALID_HANDLE_VALUE NULL
//#define GENERIC_READ 1 
//#define GENERIC_WRITE 2
//#define FILE_SHARE_READ 3
//#define OPEN_EXISTING 4
//#define FILE_DEVICE_UNKNOWN 5
//#define METHOD_BUFFERED 6
//#define FILE_ANY_ACCESS 7
//#define ICONEXCLAMATION 8
//#define MB_ICONEXCLAMATION 9
//#define TRUE 10
//#define METHOD_IN_DIRECT 11
//#define METHOD_OUT_DIRECT 12
#endif
#define DWORD unsigned long int
#define tpHigher 1

#define __fastcall 
#define __stdcall

#define HANDLE libusb_device_handle*

#ifndef WIN32
#define MB_OK 1
#define MB_ICONINFORMATION 4
#endif

/* For Vendor request */
#define USB_TYPE_VENDOR 0x40
#define USB_DIR_OUT 0
#define USB_DIR_IN 0x80

void ShowLastError(const char *msg);
void ShowMessage(std::string  msg);
int usbdev_ioctl (int fd, int ifno, unsigned request, void *param);

bool DeviceIoControl (HANDLE USBDeviceHandle,
					 int IOCTL_SLBUSB,
					 void *Control,
					 int  sizeControl,
					 void *ioBuffer,
					 int  sizeBuffer,
					 unsigned long  *doneBytes,
					 void *dummyNULL);

unsigned char findepa(int, HANDLE);
