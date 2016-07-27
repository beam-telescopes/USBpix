#ifndef MYUTILS_H
#define MYUTILS_H 1

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdarg.h>
#ifdef _LIBUSB_
#include <libusb.h>
#endif

#ifdef DEBUG
#define DBGOUT(s)            \
{                             \
   std::ostringstream os;    \
   os << s;                   \
   OutputDebugString( os.str().c_str() );  \
} 
#else
#define DBGOUT(...) ((void)0)
#endif

void DebugOutLastError(const char* lpszFunction);
#ifndef WIN32
void OutputDebugString(const char* dbgstring);
#endif

#ifdef _LIBUSB_
//libusb_strerror is only available in libusb 1.0.16 and newer
const char *_libusb_strerror(enum libusb_error errcode);
#endif

#endif
