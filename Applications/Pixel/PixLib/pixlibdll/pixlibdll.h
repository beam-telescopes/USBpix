
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PIXLIBDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PIXLIBDLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef PIXLIBDLL_EXPORTS
#define PIXLIBDLL_API __declspec(dllexport)
#else
#define PIXLIBDLL_API __declspec(dllimport)
#endif

// This class is exported from the pixlibdll.dll
class PIXLIBDLL_API CPixlibdll {
public:
	CPixlibdll(void);
	// TODO: add your methods here.
};

extern PIXLIBDLL_API int nPixlibdll;

PIXLIBDLL_API int fnPixlibdll(void);

