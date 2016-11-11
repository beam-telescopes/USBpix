/////////////////////////////////////////////////////////////////////
// OpenSuessPixProber.cxx
/////////////////////////////////////////////////////////////////////

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdlib.h>
#include <QTcpSocket>
#include "Suess.h"

using namespace Suess;

const std::string ProbeStation::eol   = "\r\n";
const char        ProbeStation::delim = ':';

const bool OpenSuessPixProber_Debug = false;

ProbeStation::ProbeStation(std::string host, int port, 
    std::string name, bool notify, int client_id): 
  m_client_id(client_id)
{
  sock.reset(new QTcpSocket);
  sock->connectToHost(host.data(), port);
  if (!sock->waitForConnected(suess_tcp_timeout))
  {
    throw Exception( 
        "SuessProbeStation::SuessProbeStation: TCP Connection Timeout.");
  }
  
  if (sock->state() != QAbstractSocket::ConnectedState)
  {
    throw Exception("SuessProbeStation::send: Not connected.");
  }
  send_name(name);
  send_notify(notify);
}

ProbeStation::~ProbeStation()
{
}

void ProbeStation::send_name(std::string name)
{
  std::stringstream ss;
  ss << "Name=" << name << eol;
  send(ss.str());
}
void ProbeStation::send_notify(bool notify)
{
  std::stringstream ss;
  ss << "Notify=" << notify << eol;
  send(ss.str());
}
      
void ProbeStation::send(std::string out)
{
  if (sock->state() != QAbstractSocket::ConnectedState)
  {
    
  }
  if (OpenSuessPixProber_Debug)
  {
    std::cerr << " >>SPB>> '" << out.substr(0, out.length() - eol.length()) << "'" << std::endl;
  }

  QByteArray block(out.data(), out.length());
  sock->write(block);
  sock->waitForBytesWritten(suess_tcp_timeout);
}

std::string ProbeStation::clientCommand(std::string cmd, std::string data)
{
  std::stringstream buf;
  buf << "Cmd="
      << m_client_id
      << delim
      << cmd
      << delim
      << data
      << eol;
  send(buf.str());
  return recv();
}

void ProbeStation::MoveChuckContact(float velocity)
{
  std::stringstream data;
  std::string cmd = "MoveChuckContact";
  if (velocity > 0)
  {
    if (velocity > 1)
    {
      velocity = 1;
    }
    data << int(velocity * 100);
  }

  Response r(clientCommand(cmd, data.str()));

  r.validate(m_client_id);
}
/**
 * \brief move to an arbitrary position or an arbitray distance
 * \param x			X position or distance
 * \param Y			Y position or distance
 * \param PosRef	refer to... 'H'=Home, 'Z'=Zero, 'C'=Center, 'R'=current ...position
 * \param Unit		Size of steps: 'Y'=Micron, 'I'= Mils, 'X'=Index, 'J'=Jog
 * \param velocity	speed of movement in percent
 * \param comp		comp. level: 'D'=default, 'N'=None, 'P'=Prober, 'T'=Technology
 */

void ProbeStation::MoveChuck(float x,float y, char PosRef, char Unit, 
							 float velocity, char comp){
  std::stringstream data;
  std::string cmd = "MoveChuck";
      data<<std::fixed<<std::showpoint<<std::setprecision(2);
      data << x;
	  data << " ";
      data << y;
	  data << " ";

	if (PosRef=='H'||PosRef=='Z'||PosRef=='C'||PosRef=='R')
    {
      data << PosRef; 
	  data << " ";
    } 
	else throw Exception("Suess::ProbeStation::MoveChuck: Wrong reference point.");

	if (Unit=='Y'||Unit=='I'||Unit=='X'||Unit=='J')
    {
      data << Unit;  
	  data << " ";
    } 
	else throw Exception("Suess::ProbeStation::MoveChuck: Wrong unit.");
   data<<std::resetiosflags(std::ios::fixed )<<std::resetiosflags(std::ios::showpoint )<<std::setprecision(0);
	if (velocity > 1||velocity<0)
    {
      velocity = 1;
    } 
	  data << int(velocity * 100);
	  data << " ";

    if (comp=='D'||comp=='N'||comp=='P'||comp=='T')
    {
      data << comp;
    } 
	else throw Exception("Suess::ProbeStation::MoveChuck: Wrong comp. level.");

  Response r(clientCommand(cmd, data.str()));

  r.validate(m_client_id);
  
}

ReadChuckPositionResponse ProbeStation::ReadChuckPosition(char Unit, 
	char PosRef, char comp)
{

	std::stringstream data;
	std::string cmd = "ReadChuckPosition";

	if (Unit=='Y'||Unit=='I'||Unit=='X')
    {
      data << Unit;  
	  data << " ";
    } 
	else throw Exception("Suess::ProbeStation::ReadChuckPosition: Wrong unit.");

	if (PosRef=='H'||PosRef=='Z'||PosRef=='C')
    {
      data << PosRef; 
	  data << " ";
    }
	else throw Exception("Suess::ProbeStation::ReadChuckPosition: Wrong reference point.");

	if (comp=='D'||comp=='N'||comp=='P'||comp=='T')
    {
      data << comp;
    }
	else throw Exception("Suess::ProbeStation::ReadChuckPosition: Wrong comp. Mode.");

	ReadChuckPositionResponse r(clientCommand(cmd, data.str()));

  r.validate(m_client_id);
  return r;
}

void ProbeStation::MoveChuckZ(float z, char PosRef, char Unit, 
							 float velocity, char comp){
  std::stringstream data;
  std::string cmd = "MoveChuckZ";
	data<<std::fixed<<std::showpoint<<std::setprecision(2);
      data << z;
	  data << " ";

	if (PosRef=='H'||PosRef=='Z'||PosRef=='R')
    {
      data << PosRef; 
	  data << " ";
    } 
	else throw Exception("Suess::ProbeStation::MoveChuck: Wrong reference point.");

	if (Unit=='Y'||Unit=='I'||Unit=='J')
    {
      data << Unit;  
	  data << " ";
    } 
	else throw Exception("Suess::ProbeStation::MoveChuck: Wrong unit.");
 data<<std::resetiosflags(std::ios::fixed )<<std::resetiosflags(std::ios::showpoint )<<std::setprecision(0);
   if (velocity<0||velocity>1)
    {
      velocity=1;
    } 
    data << Unit;  
	  data << " ";

    if (comp=='D'||comp=='N'||comp=='P'||comp=='T')
    {
      data << comp;
    } 
	else throw Exception("Suess::ProbeStation::MoveChuck: Wrong comp. level.");

  Response r(clientCommand(cmd, data.str()));

  r.validate(m_client_id);
}

void ProbeStation::MoveScope(float x, float y, char PosRef, char Unit, float velocity, char comp){
  std::stringstream data;
  std::string cmd = "MoveScope";
      data<<std::fixed<<std::showpoint<<std::setprecision(2);
      data << x;
	  data << " ";
      data << y;
	  data << " ";

	if (PosRef=='H'||PosRef=='Z'||PosRef=='C'||PosRef=='R')
    {
      data << PosRef; 
	  data << " ";
    } 
	else throw Exception("Suess::ProbeStation::MoveScope: Wrong reference point.");

	if (Unit=='Y'||Unit=='I'||Unit=='X'||Unit=='J')
    {
      data << Unit;  
	  data << " ";
    } 
	else throw Exception("Suess::ProbeStation::MoveScope: Wrong unit.");
   data<<std::resetiosflags(std::ios::fixed )<<std::resetiosflags(std::ios::showpoint )<<std::setprecision(0);
	if (velocity > 1||velocity<0)
    {
      velocity = 1;
    } 
	  data << int(velocity * 100);
	  data << " ";

    if (comp=='D'||comp=='N'||comp=='P'||comp=='T')
    {
      data << comp;
    } 
	else throw Exception("Suess::ProbeStation::MoveScope: Wrong comp. level.");

  Response r(clientCommand(cmd, data.str()));

  r.validate(m_client_id);
  
}
ReadScopePositionResponse ProbeStation::ReadScopePosition(char Unit, char PosRef, char comp){
	std::stringstream data;
	std::string cmd = "ReadScopePosition";

	if (Unit=='Y'||Unit=='I'||Unit=='X')
    {
      data << Unit;  
	  data << " ";
    } 
	else throw Exception("Suess::ProbeStation::ReadScopePosition: Wrong unit.");

	if (PosRef=='H'||PosRef=='Z'||PosRef=='C')
    {
      data << PosRef; 
	  data << " ";
    }
	else throw Exception("Suess::ProbeStation::ReadScopePosition: Wrong reference point.");

	if (comp=='D'||comp=='N'||comp=='P'||comp=='T')
    {
      data << comp;
    }
	else throw Exception("Suess::ProbeStation::ReadScopePosition: Wrong comp. Mode.");

	ReadScopePositionResponse r(clientCommand(cmd, data.str()));

  r.validate(m_client_id);
  return r;
}

void ProbeStation::MoveScopeZ(float /*z*/, char /*PosRef*/, char /*Unit*/,float /*velocity*/, char /*comp*/){
  throw Exception("Suess::ProbeStation::MoveScopeZ: Currently disabled until cause for malfunctioning is identified.");
//  std::stringstream data;
//   std::string cmd = "MoveScopeZ";
//       data<<std::fixed<<std::showpoint<<std::setprecision(2);
//       data << z;
// 	  data << " ";

// 	if (PosRef=='H'||PosRef=='Z'||PosRef=='R')
//     {
//       data << PosRef; 
// 	  data << " ";
//     } 
// 	else throw Exception("Suess::ProbeStation::MoveScopeZ: Wrong reference point.");

// 	if (Unit=='Y'||Unit=='I'||Unit=='J')
//     {
//       data << Unit;  
// 	  data << " ";
//     } 
// 	else throw Exception("Suess::ProbeStation::MoveScopeZ: Wrong unit.");

//    if (velocity<0||velocity>1)
//     {
//       velocity=1;
//     } 
//     data << Unit;  
// 	  data << " ";

//     if (comp=='D'||comp=='N'||comp=='P'||comp=='T')
//     {
//       data << comp;
//     } 
// 	else throw Exception("Suess::ProbeStation::MoveScope: Wrong comp. level.");

//   Response r(clientCommand(cmd, data.str()));

//   r.validate(m_client_id);
}

void ProbeStation::MoveChuckSeparation(float velocity)
{
  std::stringstream data;
  std::string cmd = "MoveChuckSeparation";
  if (velocity > 0)
  {
    if (velocity > 1)
    {
      velocity = 1;
    }
    data << int(velocity * 100);
  }

  Response r(clientCommand(cmd, data.str()));

  r.validate(m_client_id);
}
        
StepDieResponse ProbeStation::StepFirstDie(bool ClearBins, bool RecalcRoute)
{
  std::stringstream data;
  std::string cmd = "StepFirstDie";
  data << ClearBins;
  data << " ";
  data << RecalcRoute;

  StepDieResponse r(clientCommand(cmd, data.str()));

  r.validate(m_client_id);
  return r;
}

StepDieResponse ProbeStation::StepNextDie(
    int column, int row, int subdie)
{
  std::stringstream data;
  std::string cmd = "StepNextDie";

  if ((column != -1) && (row != -1) && (subdie != -1))
  {
    data << column;
    data << " ";
    data << row;
    data << " ";
    data << subdie;
  }

  StepDieResponse r(clientCommand(cmd, data.str()));

  r.validate(m_client_id);
  return r;
}

ReadMapPositionResponse ProbeStation::ReadMapPosition(
    int pos, char from_pos)
{
  std::stringstream data;
  std::string cmd = "ReadMapPosition";
  data << pos;
  data << " ";
  data << from_pos;

  ReadMapPositionResponse r(clientCommand(cmd, data.str()));

  r.validate(m_client_id);
  return r;
}

std::string ProbeStation::recv()
{
  if (sock->state() != QAbstractSocket::ConnectedState)
  {
    throw Exception("SuessProbeStation::send: Not connected.");
  }
  QByteArray resp;
  int tries = 0;
  while((!resp.contains('\n')) && (tries++ < 600))
  {
    sock->waitForReadyRead(100);
    resp += sock->readAll();
    if (sock->state() != QAbstractSocket::ConnectedState)
    {
      throw Exception("SuessProbeStation::send: Not connected.");
    }
  }
  
  int idx = resp.indexOf('\n');
  if (idx < 1)
    return std::string("");
  else
    return std::string(resp.constData(), idx - 1);
}

Response::Response(const Response & r)
{
  client_id = r.client_id;
  error_code = r.error_code;
  return_data = r.return_data;
}
        
Response::Response(std::string r)
{
  if (OpenSuessPixProber_Debug)
  {
    std::cerr << "Response::Response(" << r << ")" << std::endl;
  }
  
  if (r.substr(0, 4) != "Rsp=")
  {
    throw Exception("Response::Response: Parser error: \"" + r + "\"");
  }
  std::istringstream ss(r.substr(4));
  std::string x;

  if (ss.eof())
  {
    throw Exception("Response::Response: Parser error: \"" + r + "\"");
  }
  std::getline(ss, x, ProbeStation::delim);
  client_id = atoi(x.data());

  if (ss.eof())
  {
    throw Exception("Response::Response: Parser error: \"" + r + "\"");
  }
  std::getline(ss, x, ProbeStation::delim);
  error_code = atoi(x.data());

  if (ss.eof())
  {
    throw Exception("Response::Response: Parser error: \"" + r + "\"");
  }
  std::getline(ss, return_data, ProbeStation::delim);
  data_iss.reset(new std::istringstream(return_data));
}
        
std::string Response::parseString()
{
  if (data_iss->eof())
  {
    throw Exception("Response::Response: Data parser error: \"" + return_data 
        + "\"");
  }
  std::string r;
  std::getline(*data_iss, r, ' ');
  return r;
}

int Response::parseInt()
{
  return atoi(parseString().data());
}
        
float Response::parseFloat()
{
  return atof(parseString().data());
}
        
void Response::validate(int client_id)
{
  if (OpenSuessPixProber_Debug)
  {
    dump();
  }

  if (client_id != this->client_id)
  {
    throw Exception("Response::validate: Client id mismatch.");
  }
}

void Response::dump()
{
  std::cerr << "Response: client id = " << client_id << ", error_code = " << error_code << ", data = " << return_data << std::endl;
}

StepDieResponse::StepDieResponse(std::string d): Response(d),
  r_column(0), r_row(0), r_subdie(0), r_totalsubdies(0)
{
  if (error_code == 0)
  {
    r_column       = parseInt();
    r_row          = parseInt();
    r_subdie       = parseInt();
    r_totalsubdies = parseInt();
  }
}
        
StepDieResponse::StepDieResponse(const StepDieResponse & r):
  Response(r)
{
  r_column = r.r_column;
  r_row    = r.r_row;
  r_subdie = r.r_subdie;
  r_totalsubdies = r.r_totalsubdies;
}

ReadChuckPositionResponse::ReadChuckPositionResponse(std::string d): Response(d), //???
  r_x(0), r_y(0), r_z(0)
{
  if (error_code == 0)
  {
    r_x = parseFloat();
    r_y = parseFloat();
    r_z = parseFloat();
  }
}
        
ReadChuckPositionResponse::ReadChuckPositionResponse(const ReadChuckPositionResponse & r):
  Response(r)
{
  r_x = r.r_x;
  r_y = r.r_y;
  r_z = r.r_z;

}

ReadScopePositionResponse::ReadScopePositionResponse(std::string d): Response(d), //???
  r_x(0), r_y(0), r_z(0)
{
  if (error_code == 0)
  {
    r_x = parseFloat();
    r_y = parseFloat();
    r_z = parseFloat();
  }
}
        
ReadScopePositionResponse::ReadScopePositionResponse(const ReadScopePositionResponse & r):
  Response(r)
{
  r_x = r.r_x;
  r_y = r.r_y;
  r_z = r.r_z;

}

ReadMapPositionResponse::ReadMapPositionResponse(std::string d): 
  Response(d)
{
  r_column = 0;
  r_row = 0;
  r_x_position = 0;
  r_y_position = 0;
  r_current_subdie = 0;
  r_total_subdies = 0;
  r_current_die = 0;
  r_total_dies = 0;
  r_current_cluster = 0;
  r_total_clusters = 0;

  if (error_code == 0)
  {
    r_column = parseInt();
    r_row = parseInt();
    r_x_position = parseFloat();
    r_y_position = parseFloat();
    r_current_subdie = parseInt();
    r_total_subdies = parseInt();
    r_current_die = parseInt();
    r_total_dies = parseInt();
    r_current_cluster = parseInt();
    r_total_clusters = parseInt();
  }
}

void ReadMapPositionResponse::dump()
{
  Response::dump();
  std::cerr << " + StepDieResponse: " 
    << " r_column = " << r_column << ", "
    << " r_row = " << r_row << ", "
    << " r_x_position = " << r_x_position << ", "
    << " r_y_position = " << r_y_position << ", "
    << " r_current_subdie = " << r_current_subdie << ", "
    << " r_total_subdies = " << r_total_subdies << ", "
    << " r_current_die = " << r_current_die << ", "
    << " r_total_dies = " << r_total_dies << ", "
    << " r_current_cluster = " << r_current_cluster << ", "
    << " r_total_clusters = " << r_total_clusters << std::endl;
}

ReadMapPositionResponse::ReadMapPositionResponse(
    const ReadMapPositionResponse & r):
  Response(r)
{
  r_column          = r.r_column;
  r_row             = r.r_row;
  r_x_position      = r.r_x_position;
  r_y_position      = r.r_y_position;
  r_current_subdie  = r.r_current_subdie;
  r_total_subdies   = r.r_total_subdies;
  r_current_die     = r.r_current_die;
  r_total_dies      = r.r_total_dies;
  r_current_cluster = r.r_current_cluster;
  r_total_clusters  = r.r_total_clusters;
}

void StepDieResponse::dump()
{
  Response::dump();
  std::cerr << " + StepDieResponse: " 
    << " r_column = " << r_column << ", "
    << " r_row = "    << r_row << ", "
    << " r_subdie = " << r_subdie << ", "
    << " r_totalsubdies = " << r_totalsubdies << std::endl;
}

void ReadChuckPositionResponse::dump()
{
  Response::dump();
  std::cerr << " + ReadChuckPositionResponse: " 
    << " r_x = " << r_x << ", "
    << " r_y = " << r_y << ", "
    << " r_z = " << r_z << std::endl;
}
void ReadScopePositionResponse::dump()
{
  Response::dump();
  std::cerr << " + ReadScopePositionResponse: " 
    << " r_x = " << r_x << ", "
    << " r_y = " << r_y << ", "
    << " r_z = " << r_z << std::endl;
}
