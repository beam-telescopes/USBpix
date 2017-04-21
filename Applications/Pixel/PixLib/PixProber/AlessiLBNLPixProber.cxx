#include "PixProber/AlessiLBNLPixProber.h"
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
#include <iostream>
#include <cmath>			//necessary for (overloading) the abs() function
#include <fstream>
///*********************
#include <time.h>

using namespace PixLib;
using namespace std;

#ifdef WIN32		// Used to convert std::string to System::string
bool To_string(String^ source, string &target)
{
	int len = ((source->Length+1) * 2);
	char *ch = new char[len];
	errno_t result;
	size_t pReturnValue;
	{
		pin_ptr<const wchar_t> wch = PtrToStringChars(source);
		result = wcstombs_s(&pReturnValue, ch, len, wch, len);
	}
	target = ch;
	delete[] ch;
	return (result!=0);
}
#endif

AlessiLBNLPixProber::AlessiLBNLPixProber(DBInquire *dbInquire) : PixProber(dbInquire)
{
	chip_pitch_x = 20.320228;				//FE-I4
	chip_pitch_y = 19.26256;				//
	//chip_pitch_x =	7.419;				//FE-I3		Used for testing old wafers.
	//chip_pitch_y =	11.858;				//
	m_name = "AlessiLBNLPixProber";
	m_wasInit = false;
	configInit();
	if(dbInquire!=0) {
		m_conf->read(dbInquire);
	}
}

AlessiLBNLPixProber::~AlessiLBNLPixProber()
{
	delete m_conf;
	// close and delete serial connection
#ifdef WIN32
	if(static_cast<SerialPort^>(serialPort)!=nullptr) serialPort->Close();
	delete serialPort;
#endif
}

#ifdef WIN32
void AlessiLBNLPixProber::Sleep(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
#else
void AlessiLBNLPixProber::Sleep(unsigned int)
{
#endif
}
    
void AlessiLBNLPixProber::initHW()
{
#ifdef WIN32

	serialPort = gcnew SerialPort();

	// default windows communication port name
	serialPort->PortName = "COM1";

	// properties governed by the probe station
	serialPort->BaudRate = 9600;
	serialPort->Parity = Parity::Even;
	serialPort->DataBits = 7;
	serialPort->StopBits =  StopBits::One;
	serialPort->Handshake = Handshake::None;

	// Set the read/write timeouts
	serialPort->WriteTimeout = 1000;
	serialPort->ReadTimeout = 1000;

	// Open the connection
	serialPort->Open();

	// clean-up the I/O buffers, in case there is junk there.
	serialPort->DiscardInBuffer();
	serialPort->DiscardOutBuffer();

	System::String^ response = "";
	bool FLAG = true;

	// Check the position of the DUT, to check that the connection is active.
	serialPort->WriteLine("QP D");
	Sleep(1000);
	FLAG = true;
	while (FLAG)
	{
		response = serialPort->ReadExisting();
		FLAG = !(response->Contains("QP D") && response->Contains(" X ") && response->Contains(" Y ") && response->Contains(" Z ") && response->Contains(" T "));
	}
	
#endif
}

void AlessiLBNLPixProber::configInit()
{
	// Create the Config object
	m_conf = new Config("AlessiLBNLPixProber");
	Config &conf = *m_conf;
	
	// Group general
	conf.addGroup("general");
	conf["general"].addString("IPadd", m_myIP, "123.456.789.255", "IP address of probe station", true);
	conf["general"].addInt("Port", m_port, 1000, "Port at which to connect to probe station", true);
	conf["general"].addBool("notify", m_notify, false, "Suss docs: true if this client wants to receive notifications", true);
	conf.reset();
}

void AlessiLBNLPixProber::chuckGotoFirst()
{
#ifdef WIN32

	serialPort->WriteLine("MH D");

	System::String^ response;
	bool FLAG = true;

	Sleep(1000);
	while (FLAG)
	{
		response = serialPort->ReadExisting();
		FLAG = !(response->Contains("RR MH D"));
	}

#endif
}

void AlessiLBNLPixProber::chuckContact()
{
#ifdef WIN32

	for (int NScrubs=0; NScrubs<1; NScrubs++)		//Contacts NScrubs times to scrub the pads.
	{
		chuckContactOnce();							//Contact.
		chuckSeparate();							//Separate.
	}
	chuckContactOnce();
#endif
}

void AlessiLBNLPixProber::chuckContactOnce()
{
#ifdef WIN32

	serialPort->WriteLine("MZ D C");

	System::String^ response;
	bool FLAG = true;

	Sleep(1000);
	while (FLAG)								//Listen for response.
	{
		response = serialPort->ReadExisting();
		FLAG = !(response->Contains("RR MZ D"));
	}
#endif
}

void AlessiLBNLPixProber::chuckSeparate()
{
#ifdef WIN32

	serialPort->WriteLine("MZ D S");

	System::String^ response;
	bool FLAG = true;
	Sleep(1000);

	while (FLAG)
	{
		response = serialPort->ReadExisting();
		FLAG = !(response->Contains("RR MZ D"));
	}

#endif
}

bool AlessiLBNLPixProber::chuckNextDie()
{
#ifdef WIN32
	int col, row, unused_type_parameter;
	chuckGetPosition(col, row, unused_type_parameter);

	if ((col==7)&&(row==4)) return true;		//Reached end of wafer.
	
	if ((col==0)&&(row==4))						//Rather tedious filter down process for determining
	{											//whether the chuck needs to go to the next column or not.
		chuckGotoPosition(1, -1, unused_type_parameter);
		return false;
	}
	if ((col==1)&&(row==5))
	{
		chuckGotoPosition(2, -2, unused_type_parameter);
		return false;
	}
	if ((col==2)&&(row==6))
	{
		chuckGotoPosition(3, -2, unused_type_parameter);
		return false;
	}
	if ((col==3)&&(row==6))
	{
		chuckGotoPosition(4, -2, unused_type_parameter);
		return false;
	}
	if ((col==4)&&(row==6))
	{
		chuckGotoPosition(5, -2, unused_type_parameter);
		return false;
	}
	if ((col==5)&&(row==6))
	{
		chuckGotoPosition(6, -1, unused_type_parameter);
		return false;
	}
	if ((col==6)&&(row==5))
	{
		chuckGotoPosition(7, 0, unused_type_parameter);
		return false;
	}
	
	else									//If none of the above conditions are met, we are somewhere
	{										//in the middle of the wafer. Advance one step up.
		serialPort->WriteLine("MN D Y 1");

		System::String^ response;
		bool FLAG = true;

		Sleep(1000);
		while (FLAG)
		{
			response = serialPort->ReadExisting();
			FLAG = !(response->Contains("RR MN D Y"));
		}
	}
#endif
	return false;			//Reached end of wafer?
}

#ifdef WIN32
void AlessiLBNLPixProber::chuckGotoPosition(int col, int row, int type)
{
	int current_col = 0;
	int current_row = 0;
	char command[64];

	AlessiLBNLPixProber::chuckGetPosition(current_col, current_row, type);		//Get the position.
	sprintf_s(command,"MN D X %i Y %i", (col-current_col), (row-current_row));	//Format the output.


	bool FLAG = true;
	System::String^ response = "";
	System::String^ clistr = gcnew String(command);
	serialPort->WriteLine(clistr);

	Sleep(1000);

	while (FLAG)
	{
		response = serialPort->ReadExisting();
		FLAG = !(response->Contains("RR MN D"));
	}
#else
void AlessiLBNLPixProber::chuckGotoPosition(int, int, int)
{
#endif
}

#ifdef WIN32
void AlessiLBNLPixProber::chuckGetPosition(int &col, int &row, int &type)
{


	System::String^ response = "";
	std::string standardstring, xposstr, yposstr;
	double xpos, ypos;
	int end_x_index, end_y_index;
	const char *cstr_x, *cstr_y;

	serialPort->WriteLine("QP D");
	Sleep(1000);

	response = serialPort->ReadExisting();				//Read the response.
	To_string(response, standardstring);				//Convert to std::string.

	end_x_index = standardstring.find(" Y ") - 1;		//Find the X and Y coordinates in the string.
	end_y_index = standardstring.find(" Z ") - 1;

	xposstr = standardstring.substr(7,(end_x_index - 7));		//Snip the substrings which contain the coordinates.
	yposstr = standardstring.substr((end_x_index + 4),(end_y_index - (end_x_index + 4)));

	cstr_x = xposstr.c_str();				//Convert the substrings to const char* type.
	cstr_y = yposstr.c_str();

	xpos = (atof(cstr_x))/chip_pitch_x;		//Convert the const char* strings to double type numbers.
	ypos = (atof(cstr_y))/chip_pitch_y;		//Divide by chip pitch to find the current column.

	if (abs(xpos-int(xpos))>=0.5)			//Round them off, because columns are integers.
	{
		if (xpos>=0) col = int(xpos+1);
		else col = int(xpos-1);
	}
	else
	{
		if (xpos>=0) col = int(xpos);
		else col = int(xpos);
	};
	if (abs(ypos-int(ypos))>=0.5)
	{
		if (ypos>=0) row = int(ypos+1);
		else row = int(ypos-1);
	}
	else
	{
		if (ypos>=0) row = int(ypos);
		else row = int(ypos);
	};
#else
void AlessiLBNLPixProber::chuckGetPosition(int &, int &, int &)
{
#endif
}
void AlessiLBNLPixProber::chuckGetIndex(int &index)
{
	int col = 0;
	int row = 0;
	int unused_type_parameter;

	chuckGetPosition(col, row, unused_type_parameter);		//Get the chip position, in terms of col and row.

	switch (col)					//Get the chip number using this cool method. I bet there's a faster way.
    {
	case 0:
		index = 60;
		break;
	case 1:
		index = 54;
		break;
	case 2:
		index = 46;
		break;
	case 3:
		index = 37;
		break;
	case 4:
		index = 28;
		break;
	case 5:
		index = 19;
		break;
	case 6:
		index = 11;
		break;
	case 7:
		index = 5;
		break;
	default:
		index = 999;//Should throw an error here.
    };
    index -= row;
}
void AlessiLBNLPixProber::chuckGotoIndex(int index)
{
  int col = 0, row = 0;//, currentcol = 0, currentrow = 0, xdist = 0, ydist = 0;

    //Determine column and row of index to move to.
    if ((index > 60) || (index < 1))
    {
		//Should throw an exception here.
		return;
    }
    else if ((index <= 60) && (index >= 56))
    {
		row = 60;
		col = 0;
    }
    else if ((index <= 55) && (index >= 49))
    {
		row = 54;
		col = 1;
    }
    else if ((index <= 48) && (index >= 40))
    {
		row = 46;
		col = 2;
    }
    else if ((index <= 39) && (index >= 31))
    {
		row = 37;
		col = 3;
    }
    else if ((index <= 30) && (index >= 22))
    {
		row = 28;
		col = 4;
    }
    else if ((index <= 21) && (index >= 13))
    {
		row = 19;
		col = 5;
    }
    else if ((index <= 12) && (index >= 6))
    {
		row = 11;
		col = 6;
    }
    else if ((index <= 5) && (index >= 1))
    {
		row = 5;
		col = 7;
    }
    else return;//Throw an error here.
    row -= index;

	int type = 0;
	chuckGotoPosition(col, row, type);
}


void AlessiLBNLPixProber::disconnectPS(){}
void AlessiLBNLPixProber::connectPS(){}
