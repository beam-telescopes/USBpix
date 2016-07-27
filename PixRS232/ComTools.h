//*****************************************************************************
//*
//*		
//*		ComTools.h
//*
//*
//*****************************************************************************
#ifndef 	__COM_TOOLS_H__
#define 	__COM_TOOLS_H__



									//  Konstanten für Parity
#define 	P_NONE		0			//	0 = kein Parity Bit
#define 	P_EVEN		1			//	1 = gerade
#define 	P_ODD		2			//	2 = ungerade
#define 	P_SPACE		3			//	3 = immer 0
#define 	P_MARK		4			//	4 = immer 1

#define 	D_7BIT		0			// 	7-Databits
#define 	D_8BIT		1			// 	8-Databits

#define 	S_1BIT		0			// 	1   Stopbit
#define 	S_1_5BIT	1			// 	1.5 Stopbits
#define 	S_2BIT		2			// 	2   Stopbits

#include <dllexport.h>

	DllExport void	ComDetectPorts(int &iCount,int *pMode,int iMaxPorts);
	DllExport int		ComInit();
	DllExport int		ComExit();
	DllExport int		ComOpen (unsigned Nr,int Baud=9600,int Parity=P_NONE,int Stopbits=S_1BIT,int Databits=D_8BIT);
	DllExport int		ComClose(unsigned Nr);
	DllExport int		ComRead (unsigned Nr,void *Buffer,int Max);
	DllExport int		ComRead (unsigned Nr);
	DllExport int		ComWrite(unsigned Nr,void *Buffer,int Count);
	DllExport int		ComWrite(unsigned Nr,int Zeichen);
	DllExport int		ComGetReadCount (unsigned Nr);
	DllExport int		ComGetWriteCount(unsigned Nr);

#endif 	





