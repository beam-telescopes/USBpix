//*****************************************************************************
//*
//*
//*		ComToolsLinux.c
//*
//*
//*****************************************************************************
//
//	(C) Copyright Anton Zechner 2007
//
#include	<unistd.h>
#include	<memory.h>
#include	<errno.h>
#include	<termio.h>
#include	<sys/time.h>
#include	<sys/fcntl.h>
#include	<sys/ioctl.h>
#include	<linux/serial.h>
#include	"ComTools.h"



#include <linux/version.h>
#if LINUX_VERSION_CODE < 0x020400
#warning "LINUX Version NOT OK"
#endif


typedef	struct
	{
	unsigned long	iBaud;
	speed_t 		iCode;
	}TcSpeed;

static TcSpeed aTcSpeeds[] =
	{
#ifdef B300
		{300, B300},
	#endif
	#ifdef B600
		{600, B600},
	#endif
	#ifdef B1200
		{1200, B1200},
	#endif
	#ifdef B1800
		{1800, B1800},
	#endif
	#ifdef B2400
		{2400, B2400},
	#endif
	#ifdef B4800
		{4800, B4800},
	#endif
	#ifdef B9600
		{9600, B9600},
	#endif
	#ifdef B19200
		{19200, B19200},
	#endif
	#ifdef B38400
		{38400, B38400},
	#endif
	#ifdef B57600
		{57600, B57600},
	#endif
	#ifdef B115200
		{115200, B115200},
	#endif
	};

#define		MAX_SPEED		(int)(sizeof(aTcSpeeds)/sizeof(aTcSpeeds[0]))

#define 	MAX_COM_PORTS	8

static int	iComFile[MAX_COM_PORTS];
static char cIsOpen	[MAX_COM_PORTS];


int		iProtocolHandle=-1;

//*****************************************************************************
//*
//*		ComInit
//*
//*****************************************************************************
int ComInit()
{
return 1;
}

//*****************************************************************************
//*
//*		ComExit
//*
//*****************************************************************************
int ComExit()
{
int	i;

	for(i=0;i<MAX_COM_PORTS;i++)
		{
		if(!cIsOpen[i])continue;
		ComClose(i);
		}

return 1;
}

//*****************************************************************************
//*
//*		ComOpen
//*
//*****************************************************************************
//	Öffnet eine serielle Verbindung
//	uNr			: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	iBaud		: Ist die Bautrate
//	iParity		: 0 = kein iParity Bit	(P_NONE)
//				  1 = gerade			(P_EVEN)
//				  2 = ungerade			(P_ODD)
//				  3	= immer 0			(P_SPACE)
//				  4 = immer 1			(P_MARK)
//	iStopbits	: 0 = Ein Stopbit		(S_1BIT)
//				  1 = Zwei iStopbits	(S_2BIT)
//	Bits		: 0 = 8 Datenbits
//				  1 = 7 Datenbits
//				  7 = 7 Datenbits		(D_7BIT)
//				  8 = 8 Datenbits		(D_8BIT)
//	Ergibt 1 wenn eine Schnittstelle geöffnet wurde
int ComOpen(unsigned uNr,int iBaud,int iParity,int iStopbits,int iDatabits)//,int iRs485)
{
struct		serial_struct sData;
struct      termios sPortInfo;
char		cName[]="/dev/ttyUSB0";
int			iHandle,i;


	if(uNr>=MAX_COM_PORTS)return 0;

	cName[0]+=uNr;
    iHandle=open(cName,O_RDWR|O_NONBLOCK|O_NOCTTY);		// Device oeffnen

    if(iHandle<0)return 0;

	tcgetattr(iHandle, &sPortInfo);
	cfmakeraw(&sPortInfo);
	sPortInfo.c_cc[VMIN] = 0;
	sPortInfo.c_cc[VTIME]= 0;

	if(iStopbits)										// Stopbits einstellen
    		sPortInfo.c_cflag &= ~CSTOPB;
    else	sPortInfo.c_cflag |=  CSTOPB;

	sPortInfo.c_cflag &= ~CSIZE; 						// Datenbits einstellen
	sPortInfo.c_cflag |= (iDatabits&1)? CS8:CS7;


	switch(iParity)										// Parity-Bits einstellen
		{
	case P_NONE:    sPortInfo.c_cflag =  sPortInfo.c_cflag & ~PARENB;				break;
	case P_ODD :	sPortInfo.c_cflag =  sPortInfo.c_cflag |  PARENB  |  PARODD;	break;
	case P_EVEN:	sPortInfo.c_cflag = (sPortInfo.c_cflag |  PARENB) & ~PARODD;	break;
	default:			close(iHandle);
						return 0;
		}

	tcsetattr(iHandle,TCSANOW,&sPortInfo);

	for(i=0;i<MAX_SPEED;i++)
		{
		  if(aTcSpeeds[i].iBaud==(unsigned int)iBaud)break;
		}

	if(iBaud<MAX_SPEED)
		{
		errno=-EFAULT;
		close(iHandle);
		return 0;
		}

	ioctl(iHandle,TIOCGSERIAL,&sData);					// Spezial-Baudraten löschen
	sData.flags &= ~ASYNC_SPD_MASK;
	ioctl(iHandle,TIOCSSERIAL,&sData);

	iBaud=aTcSpeeds[i].iCode;
    tcgetattr(iHandle,&sPortInfo);
    cfsetospeed(&sPortInfo, iBaud);
    cfsetispeed(&sPortInfo, iBaud);
    tcsetattr(iHandle,TCSANOW,&sPortInfo);

	i = TIOCM_RTS|TIOCM_DTR;							// RTS DTR setzen
	ioctl(iHandle, TIOCMBIS,&i);


	iComFile[uNr] = iHandle;
	cIsOpen   [uNr] = true;

return 1;
}

//*****************************************************************************
//*
//*		ComGetReadCount
//*
//*****************************************************************************
//	Ergibt die Anzahl der Bytes die im Lesepuffer der Schnittstelle sind
//	uNr			: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	Ergibt die Anzahl der Bytes im Buffer

#include	<stdio.h>

int ComGetReadCount(unsigned uNr)
{
long	lLenght;

	if(uNr>=MAX_COM_PORTS)return 0;
	if(!cIsOpen[uNr])return 0;


	if(ioctl(iComFile[uNr],TIOCINQ,&lLenght)<0)return 0;

return lLenght;
}

//*****************************************************************************
//*
//*		ComGetWriteCount
//*
//*****************************************************************************
//	Ergibt die Anzahl der Bytes die im Schreibpuffer der Schnittstelle sind
//	uNr			: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	Ergibt die Anzahl der Bytes im Buffer
int ComGetWriteCount(unsigned uNr)
{
long	lLenght;

	if(uNr>=MAX_COM_PORTS)return 0;
	if(!cIsOpen[uNr])return 0;

	if(ioctl(iComFile[uNr]+1,TIOCOUTQ,&lLenght)<0)return 0;

return lLenght;
}



//*****************************************************************************
//*
//*		ComWrite
//*
//*****************************************************************************
//	Ein Zeichen senden
//	uNr			: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	iZeichen	: Ist das Zeichen das gesendet werden soll.
//	Ergibt die Anzahl der gesendeten Zeichen
int ComWrite(unsigned uNr,int iZeichen)
{
int		iRet;


	if(uNr>=MAX_COM_PORTS)return 0;
	if(!cIsOpen[uNr])return 0;

	   iRet=write(iComFile[uNr],&iZeichen,1);
	if(iRet<0)iRet=0;


return iRet;
}

//*****************************************************************************
//*
//*		ComWrite
//*
//*****************************************************************************
//	Mehrere Zeichen schreiben
//	uNr		: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	pBuffer	: Buffer in dem die Zeichen gespeichert werden
//	iCount	: Anzahl der zu sendenden Zeichen
//	Ergibt die Anzahl der gesendeten Zeichen
int	ComWrite(unsigned uNr,void *pBuffer,int iCount)
{
int		iRet;


	if(uNr>=MAX_COM_PORTS)return 0;
	if(!cIsOpen[uNr])return 0;

	   iRet=write(iComFile[uNr],pBuffer,iCount);
	if(iRet<0)iRet=0;


return iRet;
}


//*****************************************************************************
//*
//*		ComRead
//*
//*****************************************************************************
//	Ein Zeichen lesen
//	uNr		: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	Ergibt -1 wenn nichts gelesen wurde sonst das Zeichen
int ComRead(unsigned uNr)
{
int				iRet;
unsigned char 	ucChar;


	if(uNr>=MAX_COM_PORTS)return 0;
	if(!cIsOpen[uNr])return 0;

	   iRet=read(iComFile[uNr],&ucChar,1);
	if(iRet<=0)return -1;

return ucChar;
}

//*****************************************************************************
//*
//*		ComWrite
//*
//*****************************************************************************
//	Mehrere Zeichen lesen
//	uNr		: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	pBuffer	: Buffer in dem die Zeichen gespeichert werden
//	iMax	: Maximale Anzahl der zu lesenden Zeichen
//	Ergibt die Anzahl der gelesenen Zeichen
int	ComRead(unsigned uNr,void *pBuffer,int iMax)
{
int		iRet;


	if(uNr>=MAX_COM_PORTS)return 0;
	if(!cIsOpen[uNr])return 0;

	   iRet=read(iComFile[uNr],pBuffer,iMax);
	if(iRet<0)iRet=0;


return iRet;
}


//*****************************************************************************
//*
//*		ComClose
//*
//*****************************************************************************
//	Schließt eine serielle Verbindung
//	uNr	: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	Ergibt 1 wenn eine Schnittstelle geschlossen wurde
int	ComClose(unsigned uNr)
{

	if(uNr>=MAX_COM_PORTS)return 0;
	if(!cIsOpen[uNr])return 0;

	close(iComFile[uNr]);
	iComFile[uNr]=0;
	cIsOpen [uNr]=false;


return 1;
}
