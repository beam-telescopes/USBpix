#ifdef WIN32
  #include <windows.h>
#endif
#include "myutils.h"

using namespace std;

#ifdef WIN32

void DebugOutLastError(const char* lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
		std::stringstream lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

		lpDisplayBuf << lpszFunction << ": ERROR " << " (" << (int)dw  << ") "<< (char*)lpMsgBuf; 
		OutputDebugString(lpDisplayBuf.str().c_str());

    LocalFree(lpMsgBuf);
		return;
}
#else
void DebugOutLastError(const char* lpszFunction) {
	cout << lpszFunction << endl;
}
void OutputDebugString(const char* dbgstring){
	cout << dbgstring << endl;
}


#endif

#ifdef _LIBUSB_
//libusb_strerror is only available in libusb 1.0.16 and newer
const char *_libusb_strerror(enum libusb_error errcode) {
	switch(errcode) {
		case LIBUSB_SUCCESS:
			return "Success";
		case LIBUSB_ERROR_IO:
			return "Input/output error";
		case LIBUSB_ERROR_INVALID_PARAM:
			return "Invalid parameter";
		case LIBUSB_ERROR_ACCESS:
			return "Access denied (insufficient permissions)";
		case LIBUSB_ERROR_NO_DEVICE:
			return "No such device (it may have been disconnected)";
		case LIBUSB_ERROR_NOT_FOUND:
			return "Entity not found";
		case LIBUSB_ERROR_BUSY:
			return "Resource busy";
		case LIBUSB_ERROR_TIMEOUT:
			return "Operation timed out";
		case LIBUSB_ERROR_OVERFLOW:
			return "Overflow";
		case LIBUSB_ERROR_PIPE:
			return "Pipe error";
		case LIBUSB_ERROR_INTERRUPTED:
			return "System call interrupted (perhaps due to signal)";
		case LIBUSB_ERROR_NO_MEM:
			return "Insufficient memory";
		case LIBUSB_ERROR_NOT_SUPPORTED:
			return "Operation not supported or unimplemented on this platform";
		case LIBUSB_ERROR_OTHER:
			return "Other error";
	}
	return "Unknown error";
}
#endif
