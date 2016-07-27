#include "PixProber/LBNLPixProber.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include "PixConfDBInterface/PixConfDBInterface.h"

#ifdef WIN32
// windows serial stuff
#using <system.dll>
#include <gcroot.h>
#include <vcclr.h>

using namespace System;
using namespace System::IO::Ports;
#endif
/////****************
#include <stdlib.h>
#include <string>
///*********************
#include <time.h>
 
void Sleep(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}


using namespace PixLib;

#ifdef WIN32
bool To_CharStar( String^ source, char*& target )
{
    pin_ptr<const wchar_t> wch = PtrToStringChars( source );
    int len = (( source->Length+1) * 2);
    target = new char[ len ];
    errno_t result;
    size_t pReturnValue;
    result = wcstombs_s(&pReturnValue, target, len, wch, len);
    return (result!=0);
}
#endif

LBNLPixProber::LBNLPixProber(DBInquire *dbInquire) : PixProber(dbInquire)
{
	m_name = "LBNLPixProber";
	m_wasInit = false;
	configInit();
	if(dbInquire!=0) {
		m_conf->read(dbInquire);
	}
}

LBNLPixProber::~LBNLPixProber()
{
	delete m_conf;
	// close and delete serial connection
#ifdef WIN32
	if(static_cast<SerialPort^>(serialPort)!=nullptr) serialPort->Close();
	delete serialPort;
#endif

}
    
void LBNLPixProber::initHW()
{
#ifdef WIN32
	serialPort = gcnew SerialPort();

	// default windows communication port name
	serialPort->PortName = "COM1";

	// properties governed by the probe station
	serialPort->BaudRate = 9600;
	serialPort->Parity = Parity::None;
	serialPort->DataBits = 8;
	serialPort->StopBits =  StopBits::One;
	serialPort->Handshake = Handshake::XOnXOff;

	// Set the read/write timeouts
	serialPort->WriteTimeout = 60000;
	serialPort->ReadTimeout = 60000;
	
	// default command termination for (Write/Read)Line() is a LF
	// the probe station likes a CR
	serialPort->NewLine = "\r";

	// open the connection
	serialPort->Open();

	// clean-up the I/O buffers
	serialPort->DiscardInBuffer();
	serialPort->DiscardOutBuffer();
	
	// display some text on pendant to assure connection
	serialPort->WriteLine(":MESsage banana");

	// set units to micrometers
	serialPort->WriteLine(":UN UM");
	// poll until "OK" is recieved
	bool FLAG = true;
	System::String^ response;
	while (FLAG) {
		response = serialPort->ReadLine();
		FLAG = abs(String::Compare(response, "OK"));
	}

	// set die size
	serialPort->WriteLine(":DIES 20330.16 19262.56");
	// poll until "OK" is received
	FLAG = true;
	while (FLAG) {
		response = serialPort->ReadLine();
		FLAG = abs(String::Compare(response, "OK"));
	}



#endif
}

void LBNLPixProber::configInit()
{
	// Create the Config object
	m_conf = new Config("LBNLPixProber");
	Config &conf = *m_conf;
	
	// Group general
	conf.addGroup("general");
	conf["general"].addString("IPadd", m_myIP, "123.456.789.255", "IP address of probe station", true);
	conf["general"].addInt("Port", m_port, 1000, "Port at which to connect to probe station", true);
	conf["general"].addBool("notify", m_notify, false, "Suss docs: true if this client wants to receive notifications", true);
	conf.reset();
}

void LBNLPixProber::chuckGotoFirst()
{

#ifdef WIN32
	// Set the read/write timeouts
	serialPort->WriteTimeout = 60000;
	serialPort->ReadTimeout = 60000;

	char command[64];
	sprintf_s(command,":PRB:MOVe:CRAbs %i %i", 1, 1);
	System::String^ clistr = gcnew String(command);
	serialPort->WriteLine(clistr);

	// poll until "OK" is recieved
	bool FLAG = true;
	System::String^ response;
	while (FLAG) {
		response = serialPort->ReadLine();
		FLAG = abs(String::Compare(response, "OK"));
	}

	Sleep(30000);
#endif
}

void LBNLPixProber::chuckContact()
{
#ifdef WIN32
	char command[64];
	sprintf_s(command,":PRB:DN");
	Sleep(30000);

	System::String^ clistr = gcnew String(command);
	serialPort->WriteLine(clistr);

	// poll until "OK" is recieved
	bool FLAG = true;
	System::String^ response;
	while (FLAG) {
		response = serialPort->ReadLine();
		FLAG = abs(String::Compare(response, "OK"));
	}

#endif
}

void LBNLPixProber::chuckSeparate()
{
#ifdef WIN32
	char command[64];
	sprintf_s(command,":PRB:UP");
	System::String^ clistr = gcnew String(command);
	serialPort->WriteLine(clistr);
	// poll until "OK" is recieved
	bool FLAG = true;
	System::String^ response;
	while (FLAG) {
		response = serialPort->ReadLine();
		FLAG = abs(String::Compare(response, "OK"));
	}
	Sleep(30000);
#endif
}

bool LBNLPixProber::chuckNextDie()
{
	//move by one row

#ifdef WIN32
	char command[64];
	sprintf_s(command,":PRB:MOVe:CRR %i %i", 0, 1);
	System::String^ clistr = gcnew String(command);
	serialPort->WriteLine(clistr);
	// poll until "OK" is recieved
	bool FLAG = true;
	System::String^ response;
	while (FLAG) {
		response = serialPort->ReadLine();
		FLAG = abs(String::Compare(response, "OK"));
	}
	Sleep(30000);
#endif

	return false;//end of wafer?
}

#ifdef WIN32
void LBNLPixProber::chuckGotoPosition(int col, int row, int type)
{

	char command[64];
	sprintf_s(command,":PRB:MOVe:CRAbs %i %i", col, row);
	System::String^ clistr = gcnew String(command);
	serialPort->WriteLine(clistr);
	// poll until "OK" is recieved
	bool FLAG = true;
	System::String^ response;
	while (FLAG) {
		response = serialPort->ReadLine();
		FLAG = abs(String::Compare(response, "OK"));
	}
	Sleep(30000);

#else
void LBNLPixProber::chuckGotoPosition(int, int, int)
{
#endif
}

#ifdef WIN32
void LBNLPixProber::chuckGetPosition(int &col, int &row, int &type)
{

	char command[64];
	System::String^ response;
	System::String^ position;
  
	sprintf_s(command,":PRB:POS:CR?");
	System::String^ clistr = gcnew String(command);
	serialPort->WriteLine(clistr);

	// poll until "RESP" is recieved
	bool FLAG = true;
	char * position_char;
	while (FLAG) {
		response = serialPort->ReadLine();
		FLAG = abs(String::Compare(response, "RESP"));
	}

	// this read has the data
	position = serialPort->ReadLine();
	To_CharStar( position, position_char );
	sscanf_s (position_char, "%d %d", &col, &row);
	

	// this read has "OK"
	response = serialPort->ReadLine();
#else
void LBNLPixProber::chuckGetPosition(int &, int &, int &)
{
#endif
}


void LBNLPixProber::disconnectPS(){}
void LBNLPixProber::connectPS(){}
void LBNLPixProber::chuckGetIndex(int &index){
  // to be implemented
  index = 0;
}
 void LBNLPixProber::chuckGotoIndex(int /*index*/){
  // to be implemented
}
