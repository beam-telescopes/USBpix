/////////////////////////////////////////////////////////////////////
// SuessPixProber.cxx
/////////////////////////////////////////////////////////////////////

#include "PixProber/SuessPixProber.h"
#include "Config/Config.h"
#include "Config/ConfGroup.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#if defined(_WIN32) && !defined(DONT_LINK_SUESSPROBER)
#include <nclient.h>
#endif
#include <string>
#include <sstream>
#include <string.h>

using namespace PixLib;
using namespace std;

#ifdef DONT_LINK_SUESSPROBER

int __stdcall  SNI_Connect(char *IPAddress,
                                int ListenPort,
                                const char *Name,
                                int Notify
                               )
{
	return 0;
}

int __stdcall  SNI_Disconnect(void)
{
	return 0;
}
int __stdcall  SNI_DoProberCommand(char *CmdStr, char *RspStr)
{
	return 0;
}
int __stdcall  SNI_SendCommand(int Id, int CmdNum, char *Data)
{
	return 0;
}
int __stdcall  SNI_GetResponse(int *Id, int *ErrorCode, char *Data)
{
	return 0;
}
int __stdcall  SNI_GetNotification(int *NtfNum, char *Data)
{
	return 0;
}
void __stdcall  SNI_ShowDebug(int ShowIt)
{
}
void __stdcall  SNI_EnableErrorMsg(int EnableIt)
{
}
#endif


SuessPixProber::SuessPixProber(DBInquire *dbInquire) : PixProber(dbInquire)
{
  m_name = "SuessPixProber";
  m_wasInit = false;
  configInit();
  if(dbInquire!=0) m_conf->read(dbInquire);
}
SuessPixProber::~SuessPixProber()
{
  delete m_conf;
}
    
void SuessPixProber::initHW()
{
  if(!m_wasInit){
    // nothing really to initialise, just check connection
    connectPS();
    disconnectPS();
    m_wasInit = true;
  }
}
void SuessPixProber::configInit()
{
  // Create the Config object
  m_conf = new Config("SuessPixProber"); 
  Config &conf = *m_conf;
  
  // Group general
  conf.addGroup("general");
  conf["general"].addString("IPadd", m_myIP, "131.220.165.157",
		  "IP address of probe station", true);
  conf["general"].addInt("Port", m_port, 1000,
		  "Port at which to connect to probe station", true);
  conf["general"].addBool("notify", m_notify, false,
		  "Suss docs: true if this client wants to receive notifications", true);
  conf.reset();
}
void SuessPixProber::connectPS()
{
#ifdef WIN32
  char ipadd[50];
  sprintf_s(ipadd,"%s",m_myIP.c_str());
  int retval = SNI_Connect (ipadd, m_port, "PixLib", (int)m_notify);
  if (retval != 0){
    std::stringstream a;
    a << retval;
    throw PixProberExc(PixProberExc::ERROR, "SuessPixProber::connectPS : Probestation Connection Error = "+a.str());
  }
#else
  throw PixProberExc(PixProberExc::FATAL, "SuessPixProber::connectPS : only implemented for Windows");
#endif
}
void SuessPixProber::disconnectPS()
{
#ifdef WIN32
  int retval = SNI_Disconnect();
  if (retval != 0){
    std::stringstream a;
    a << retval;
    throw PixProberExc(PixProberExc::ERROR, "SuessPixProber::disconnectPS : Probestation Disconnection Error = "+a.str());
  }
#else
  throw PixProberExc(PixProberExc::FATAL, "SuessPixProber::connectPS : only implemented for Windows");
#endif
}
#ifdef WIN32
void SuessPixProber::runProberCommand(const char *in_command, char *response){
  char command[256];
  strncpy_s(command, in_command, 255);
  command[255]='0';
  int retval = SNI_DoProberCommand(command, response);
  if(retval!=0){
    disconnectPS();
    std::stringstream a;
    a << response;
    throw PixProberExc(PixProberExc::ERROR, "SuessPixProber::runProberCommand : Probestation Command Error "+a.str());
  }
#else
void SuessPixProber::runProberCommand(const char *, char *){
  throw PixProberExc(PixProberExc::FATAL, "SuessPixProber::runProberCommand : only implemented for Windows");
#endif
}
void SuessPixProber::chuckGotoFirst(){
  char Response[255];
  connectPS();
  runProberCommand("StepFirstDie", Response);
  disconnectPS();
}
void SuessPixProber::chuckContact(){
  char Response[255];
  connectPS();
  runProberCommand("MoveChuckContact", Response);
  disconnectPS();
}
void SuessPixProber::chuckSeparate(){
  char Response[255];
  connectPS();
  runProberCommand("MoveChuckSeparation", Response);
  disconnectPS();
}
bool SuessPixProber::chuckNextDie(){
  char Response[255];
  connectPS();
  try{
    runProberCommand("StepNextDie", Response);
  }catch(...){
    if(strcmp(Response,"End of wafer")==0) return true;
    else{
      std::stringstream a;
      a << Response;
      throw PixProberExc(PixProberExc::ERROR, "SuessPixProber::runProberCommand : Probestation Command Error "+a.str());
    }
  }
  disconnectPS();
  return false;
}
void SuessPixProber::chuckGotoPosition(int col, int row, int type){
  char Command[255];
  char Response[255];

  //last chance error handling
  if (col < -2 || col > 5 || row < 0 || row > 8
	|| (row == 0 && col == -2) || (row == 0 && col == -1) || (row == 0 && col == 4) || (row == 0 && col == 5)
	|| (row == 1 && col == -2) || (row == 1 && col == 5)
	|| (row == 7 && col == -2) || (row == 7 && col == 5)
	|| (row == 8 && col == -2) || (row == 8 && col == -1) || (row == 8 && col == 4) || (row == 8 && col == 5)
  ){
	  std::stringstream tFatal;
	  tFatal<<"chuckGotoPosition(col="<<col<<",row="<<row<<") is not on FE-I4 map";
	  throw (PixProberExc(PixProberExc::FATAL, tFatal.str()));
	  return;
  }

#ifdef WIN32
  sprintf_s(Command,"StepNextDie %i %i %i", col, row, type);
#else
  sprintf(Command,"StepNextDie %i %i %i", col, row, type);
#endif
  connectPS();
  runProberCommand(Command, Response);
  disconnectPS();
}
void SuessPixProber::chuckGetPosition(int &col, int &row, int &type){
  char Response[255];
  int Dummy;
  float xAbs, yAbs;
  
  connectPS();
  runProberCommand("ReadMapPosition", Response);
  disconnectPS();
  
#ifdef WIN32
  sscanf_s (Response, "%d %d %f %f %d %d", &col, &row, &xAbs, &yAbs, &type, &Dummy);
#else
  sscanf (Response, "%d %d %f %f %d %d", &col, &row, &xAbs, &yAbs, &type, &Dummy);
#endif
}
void SuessPixProber::chuckGetIndex(int &index){
	int col = 0;
	int row = 0;
	int type = 0;
	chuckGetPosition(col,row,type);
	switch(col)
	{
	case -2:
		switch(row)
		{
		case 6:
			index = 1;
			break;
		case 5:
			index = 2;
			break;
		case 4:
			index = 3;
			break;
		case 3:
			index = 4;
			break;
		case 2:
			index = 5;
			break;
		default:
			index = 999;
			break;
		}
		break;

	case -1:
		switch(row)
		{
		case 7:
			index = 6;
			break;
		case 6:
			index = 7;
			break;
		case 5:
			index = 8;
			break;
		case 4:
			index = 9;
			break;
		case 3:
			index = 10;
			break;
		case 2:
			index = 11;
			break;
		case 1:
			index = 12;
			break;
		default:
			index = 999;
			break;
		}
		break;

	case 0:
		switch(row)
		{
		case 8:
			index = 13;
			break;
		case 7:
			index = 14;
			break;
		case 6:
			index = 15;
			break;
		case 5:
			index = 16;
			break;
		case 4:
			index = 17;
			break;
		case 3:
			index = 18;
			break;
		case 2:
			index = 19;
			break;
		case 1:
			index = 20;
			break;
		case 0:
			index = 21;
			break;
		default:
			index = 999;
			break;
		}
		break;

	case 1:
		switch(row)
		{
		case 8:
			index = 22;
			break;
		case 7:
			index = 23;
			break;
		case 6:
			index = 24;
			break;
		case 5:
			index = 25;
			break;
		case 4:
			index = 26;
			break;
		case 3:
			index = 27;
			break;
		case 2:
			index = 28;
			break;
		case 1:
			index = 29;
			break;
		case 0:
			index = 30;
			break;
		default:
			index = 999;
			break;
		}
		break;

	case 2:
		switch(row)
		{
		case 8:
			index = 31;
			break;
		case 7:
			index = 32;
			break;
		case 6:
			index = 33;
			break;
		case 5:
			index = 34;
			break;
		case 4:
			index = 35;
			break;
		case 3:
			index = 36;
			break;
		case 2:
			index = 37;
			break;
		case 1:
			index = 38;
			break;
		case 0:
			index = 39;
			break;
		default:
			index = 999;
			break;
		}
		break;

	case 3:
		switch(row)
		{
		case 8:
			index = 40;
			break;
		case 7:
			index = 41;
			break;
		case 6:
			index = 42;
			break;
		case 5:
			index = 43;
			break;
		case 4:
			index = 44;
			break;
		case 3:
			index = 45;
			break;
		case 2:
			index = 46;
			break;
		case 1:
			index = 47;
			break;
		case 0:
			index = 48;
			break;
		default:
			index = 999;
			break;
		}
		break;

	case 4:
		switch(row)
		{
		case 7:
			index = 49;
			break;
		case 6:
			index = 50;
			break;
		case 5:
			index = 51;
			break;
		case 4:
			index = 52;
			break;
		case 3:
			index = 53;
			break;
		case 2:
			index = 54;
			break;
		case 1:
			index = 55;
			break;
		default:
			index = 999;
			break;
		}
		break;

	case 5:
		switch(row)
		{
		case 6:
			index = 56;
			break;
		case 5:
			index = 57;
			break;
		case 4:
			index = 58;
			break;
		case 3:
			index = 59;
			break;
		case 2:
			index = 60;
			break;
		default:
			index = 999;
			break;
		}
		break;
	}
	if (index == 999){
		std::stringstream tError;
		tError<<"chuckGetPosition: col="<<col<<",row="<<row<<" are not on FE-I4 map";
		throw (PixProberExc(PixProberExc::ERROR, tError.str()));
	}
}
void SuessPixProber::chuckGotoIndex(int index){
	int col = 0;
	int row = 0;
	int type = 0;

	if(1 <= index && index <= 5)
		col = -2;
	else if (6 <= index && index <= 12)
		col = -1;
	else if (13 <= index && index <= 21)
		col = 0;
	else if (22 <= index && index <= 30)
		col = 1;
	else if (31 <= index && index <= 39)
		col = 2;
	else if (40 <= index && index <= 48)
		col = 3;
	else if (49 <= index && index <= 55)
		col = 4;
	else if (56 <= index && index <= 60)
		col = 5;

	if (index==21 || index==30 || index==39 || index==48)
		row = 0;
	else if (index==12 || index==20 || index==29 || index==38 || index==47 || index==55)
		row = 1;
	else if (index==5 || index==11 || index==19 || index==28 || index==37 || index==46 || index==54 || index==60)
		row = 2;
	else if (index==4 || index==10 || index==18 || index==27 || index==36 || index==45 || index==53 || index==59)
		row = 3;
	else if (index==3 || index==9 || index==17 || index==26 || index==35 || index==44 || index==52 || index==58)
		row = 4;
	else if (index==2 || index==8 || index==16 || index==25 || index==34 || index==43 || index==51 || index==57)
		row = 5;
	else if (index==1 || index==7 || index==15 || index==24 || index==33 || index==42 || index==50 || index==56)
		row = 6;
	else if (index==6 || index==14 || index==23 || index==32 || index==41 || index==49)
		row = 7;
	else if (index==13 || index==22 || index==31 || index==40)
		row = 8;
	chuckGotoPosition(col, row, type);
}
