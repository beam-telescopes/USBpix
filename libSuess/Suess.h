/////////////////////////////////////////////////////////////////////
// SuessPixprober.h
// version 1.0
// Joern Grosse-Knetter, Kevin Kroeninger, University of Goettingen
/////////////////////////////////////////////////////////////////////

//! Class for Suess probe stations

#ifndef _SUESS_PROBESTATION_H 
#define _SUESS_PROBESTATION_H 

#include <memory>
#include <string>
#include "dllexport.h"

class QTcpSocket;

namespace Suess {
  enum Errors
  {
    None = 0,
    END_OF_WAFER = 703,
  };

  class DllExport ProbeStation;
  class DllExport Response {
    public:
      Response(std::string);
      Response(const Response & r);
      int client_id;
      int error_code;
      std::string return_data;

      virtual void dump();
      void validate(int client_id);

    protected:
      std::auto_ptr<std::istringstream> data_iss;
      std::string parseString();
      int parseInt();
      float parseFloat();
  };

  class DllExport StepDieResponse: public Response {
    public:
      StepDieResponse(std::string);
      StepDieResponse(const StepDieResponse & r);

      virtual void dump();

      int r_column;
      int r_row;
      int r_subdie;
      int r_totalsubdies;
  };

  class DllExport ReadChuckPositionResponse: public Response {
    public:
      ReadChuckPositionResponse(std::string);
      ReadChuckPositionResponse(const ReadChuckPositionResponse & r);

      virtual void dump();

      int r_x;
      int r_y;
      int r_z;
  };

  class DllExport ReadScopePositionResponse: public Response {
    public:
      ReadScopePositionResponse(std::string);
      ReadScopePositionResponse(const ReadScopePositionResponse & r);

      virtual void dump();

      int r_x;
      int r_y;
      int r_z;
  };

  class DllExport ReadMapPositionResponse: public Response {
    public:
      ReadMapPositionResponse(std::string);
      ReadMapPositionResponse(const ReadMapPositionResponse & r);

      virtual void dump();

      int r_column;
      int r_row;
      int r_x_position;
      int r_y_position;
      int r_current_subdie;
      int r_total_subdies;
      int r_current_die;
      int r_total_dies;
      int r_current_cluster;
      int r_total_clusters;
  };

  class DllExport ProbeStation {
    public:
      ProbeStation(std::string host, int port, 
          std::string name, bool notify, int client_id);
      ~ProbeStation();

      std::string clientCommand(std::string cmd, std::string data);

      void MoveChuckContact(float velocity = -1);
      void MoveChuckSeparation(float velocity = -1);
      Suess::StepDieResponse StepFirstDie(
          bool ClearBins = true, bool RecalcRoute = true);
      Suess::StepDieResponse StepNextDie(
          int column = -1, int row = -1, int subdie = -1);
      Suess::ReadMapPositionResponse ReadMapPosition(
          int pos = 0, char from_pos = 'R');

	  void MoveChuck(float x, float y, char PosRef='H', char Unit='Y', float velocity=1, char comp='D');
	  ReadChuckPositionResponse ReadChuckPosition(char Unit='Y', char PosRef='H', char comp='D');
          void MoveChuckZ(float z, char PosRef='H', char Unit='Y',float velocity=1, char comp='D');

	  void MoveScope(float x, float y, char PosRef='H', char Unit='Y', float velocity=1, char comp='D');
	  ReadScopePositionResponse ReadScopePosition(char Unit='Y', char PosRef='H', char comp='D');
          void MoveScopeZ(float z, char PosRef='H', char Unit='Y',float velocity=1, char comp='D');


    private:
      void send(std::string);
      std::string recv();

      void send_name(std::string name);
      void send_notify(bool notify);

      int m_client_id;
      std::auto_ptr<QTcpSocket> sock;
      const static int suess_tcp_timeout = 5000;

    public:
      const static char delim;
      const static std::string eol;	  
  };

  class DllExport Exception {
    public:
      Exception(std::string text):
        text(text)
      {
      }
      std::string text;
  };

}
#endif

