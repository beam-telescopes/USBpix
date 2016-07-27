////////////////////////////////////////////////////////////////////////////////
//// CascadePixPorber.cxx
//// Implementation of CascadeProber Class
//// Andrew G Stewart
//// University of Glasgow
//// 30 August 2012
/////////////////////////////////////////////////////////////////////////////////

#include "PixProber/CascadePixProber.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include "PixConfDBInterface/PixConfDBInterface.h"

#ifdef WIN32
#include "ni4882.h"
#include "visa.h"
#endif

#include <string>
#include <sstream>
#include <string.h>
#include <stdio.h>

#include <iostream>
// #include "stdafx.h"

using namespace PixLib;
using namespace std;

// CascadePixProber Constructor

CascadePixProber::CascadePixProber(DBInquire *dbInquire) : PixProber(dbInquire)
{
	m_name = "CascadePixProber";
	m_wasInit = false;
	configInit();
	if(dbInquire!=0){ 
		m_conf->read(dbInquire);
	}
}

// CascadePixProber Destructor

CascadePixProber::~CascadePixProber()
{
	delete m_conf;
}

// Definition of initHW
void CascadePixProber::initHW()
{	
	
	if(!m_wasInit){
		connectPS();
		m_wasInit = true;
	}
	
#ifdef WIN32
	// Set m_idnumber (identification number) of the Cascade
	char Response[255];
	char command[] = ":set:station?";
	ibwrt(m_device,command,strlen(command));
	ibrd(m_device,Response,50);
	sscanf_s(Response,"%d",&m_idnumber); 

	// Set Prober velocity to fast
	char Response2[255];
	char Command2[255];
	sprintf_s(Command2,":set:vel %i xy fast", m_idnumber);
	ibwrt(m_device,Command2,strlen(Command2));
	ibrd(m_device,Response2,50); // AGS Added 26/09/12
#endif
}


// Definition of Config object
void CascadePixProber::configInit()
{
	// Create the Config object
	m_conf = new Config("CascadePixProber");
	Config &conf = *m_conf;

	// Group general
	conf.addGroup("general");
	conf["general"].addInt("boardId",  m_boardId,   0,  "GPIB board index", true);
	conf["general"].addInt("GPIBaddr", m_GPIB_addr, 28, "Address of the GPIB interface", true);
	conf.reset();
}

// Definition of connectPS
void CascadePixProber::connectPS()
{
#ifdef WIN32
	m_device = ibdev(m_boardId, m_GPIB_addr, 0, T30s, 1, 0);
	if(m_device == -1){
		throw PixProberExc(PixProberExc::ERROR, "CascadePixProber::connectPS : Probestation Connection Error");
	}
#else
	throw PixProberExc(PixProberExc::FATAL, "CascadePixProber::connectPS : only implemented for Windows");
#endif
}

// Definition of disconnectPS()
void CascadePixProber::disconnectPS()
{
#ifdef WIN32
	ibonl(m_device, 0); // Take the device offline
#endif
}

// Definition of runProberCommand
#ifdef WIN32
void CascadePixProber::runProberCommand(const char *command, char *response)
{
	connectPS(); // Connect to device
	// ibclr(m_device); // Clear the device
	ibwrt(m_device,command,strlen(command)); // Send command
	ibrd(m_device,response,50); // Read response
	if(ibsta & ERR){
		throw PixProberExc(PixProberExc::ERROR, "CascadePixProber::runProberCommand : Probestation Command Error!");
	}
#else
void CascadePixProber::runProberCommand(const char *, char *)
{
	throw PixProberExc(PixProberExc::FATAL, "CascadePixProber::runProberCommand : only implemented for Windows");
#endif

}

// Probestation movement function definitions
// Definition for chuckGotoFirst. Moves chuck to position of first (index = 1) die in probeplan
// Added while loop to mitigate timeout error
void CascadePixProber::chuckGotoFirst()
{
#ifdef WIN32
	char Response[255];
	char Command[255];
	char prober_resp[] = "COMPLETE\n";
	sprintf_s(Command, ":move:probeplan:first:die %i", m_idnumber);
	runProberCommand(Command, Response);
	Response[ibcnt] = '\0';
	while(strcmp(Response,prober_resp) < 0)
	{
		ibrd(m_device,Response,50); // Added AGS 26/09/2012
		Response[ibcnt] = '\0'; // Added AGS 26/09/2012
	}
#endif
}

// Definition of chuckContact
void CascadePixProber::chuckContact()
{
#ifdef WIN32
	char Response[255];
	char Command[255];
	char prober_resp[] = "COMPLETE\n";
	sprintf_s(Command, ":move:contact %d", m_idnumber);
	runProberCommand(Command,Response);
	Response[ibcnt] = '\0';
	while(strcmp(Response,prober_resp) != 0)
	{
		ibrd(m_device,Response,50); // Added AGS 26/09/2012
		Response[ibcnt] = '\0'; // Added AGS 26/09/2012
	}
	// retract probe needles after 2 second delay
	char Command2[] = ":system:delay 2000";
	char Response2[255];
	runProberCommand(Command2, Response2);
	Response2[ibcnt] = '\0';
	while(strcmp(Response2,prober_resp) != 0)
	{
		ibrd(m_device,Response,50); // Added AGS 26/09/2012
		Response[ibcnt] = '\0'; // Added AGS 26/09/2012
	}
	chuckSeparate();
	runProberCommand(Command, Response);
	Response[ibcnt] = '\0';
	while(strcmp(Response,prober_resp) != 0)
	{
		ibrd(m_device,Response,50); // Added AGS 26/09/2012
		Response[ibcnt] = '\0'; // Added AGS 26/09/2012
	}
	m_contact = true;
#endif
}

// Definition of chuckSeparate
void CascadePixProber::chuckSeparate()
{
#ifdef WIN32
	char Response[255];
	char Command[255];
	// char prober_resp[] = "COMPLETE\n";
	sprintf_s(Command, ":move:separate %d", m_idnumber);
	runProberCommand(Command, Response);
	/*
	while(strcmp(Response2,prober_resp) != 0)
	{
		ibrd(m_device,Response2,50); // Added AGS 26/09/2012
		Response2[ibcnt] = '\0'; // Added AGS 26/09/2012
	}
	*/
	m_contact = false;
#endif
}

// Definition of chuckNextDie
bool CascadePixProber::chuckNextDie()
{
#ifdef WIN32
	char Response[255];
	char Command[255];
	char prober_resp[] = "COMPLETE\n";
	char move_error[] = "@Error moving chuck to next die\n\":MOVe:PROBeplan:NEXT:DIE\"";
	sprintf_s(Command, ":move:probeplan:next:die %d", m_idnumber);
	runProberCommand(Command, Response);
	Response[ibcnt] = '\0';
	while(!strcmp(Response,""))
	{
		ibrd(m_device,Response,50);
		Response[ibcnt] = '\0';
	}
	if(strcmp(Response,move_error) == 0)
		return true;
	else
		if(strcmp(Response,prober_resp) == 0)
			return false;
	// throw PixProberExc(PixProberExc::ERROR, "CascadePixProber::runProberCommand : Probestation Command Error");
#endif
	return false;
}

// Definition of chuckGotoPosition - moves chuck. Add while loop to mitigate possible timeout
#ifdef WIN32
void CascadePixProber::chuckGotoPosition(int col, int row, int type)
{
	char Response[255];
	char Command[255];
	char prober_resp[] = "COMPLETE\n";
	sprintf_s(Command,":move:probeplan:absolute:die %i %i %i", m_idnumber, col, row);
	runProberCommand(Command,Response);
	Response[ibcnt] = '\0';
	while(strcmp(Response,prober_resp) != 0)
	{
		ibrd(m_device,Response,50); // Added AGS 26/09/2012
		Response[ibcnt] = '\0'; // Added AGS 26/09/2012
	}
#else
void CascadePixProber::chuckGotoPosition(int, int, int)
{
#endif
}

// Definition of chuckGetPosition
#ifdef WIN32
void CascadePixProber::chuckGetPosition(int &col, int &row, int &type)
{
	char Response[255];
	char Command[255];
	sprintf_s(Command, ":move:probeplan:absolute:die? %i", m_idnumber);
	//runProberCommand(Command,Response);
	ibwrt(m_device,Command,strlen(Command));
	ibrd(m_device,Response,50);
	sscanf_s(Response,"%d %d",&col,&row);
#else
void CascadePixProber::chuckGetPosition(int &, int &, int &)
{
#endif
}

// Definition of chuckGetIndex
#ifdef WIN32
void CascadePixProber::chuckGetIndex(int &index)
{
	// Returns the current die site's position
	char Response[255];
	char Command[255];
	sprintf_s(Command,":move:probeplan:absolute:index? %i", m_idnumber);
	// runProberCommand(Command, Response);
	ibwrt(m_device,Command,strlen(Command));
	ibrd(m_device,Response,50);
	sscanf_s(Response,"%d",&index);
#else
void CascadePixProber::chuckGetIndex(int &)
{
#endif
}

// Definition of chuckGotoIndex - moves chuck (add while loop to mitigate timeout)
#ifdef WIN32
void CascadePixProber::chuckGotoIndex(int index)
{
	// moves the chuck to the nth die site listed in the probe plan
	char Response[255];
	char Command[255];
	char prober_resp[] = "COMPLETE\n";
	sprintf_s(Command,":move:probeplan:absolute:index %i %i", m_idnumber, index);
	runProberCommand(Command, Response);
	Response[ibcnt] = '\0';
	// if gpib command times out, repeat ibrd command
	while(strcmp(Response,prober_resp) != 0) // Added AGS 26/09/2012
	{
		ibrd(m_device,Response,50); // Added AGS 26/09/2012
		Response[ibcnt] = '\0'; // Added AGS 26/09/2012
	}
#else
void CascadePixProber::chuckGotoIndex(int)
{
#endif
}
