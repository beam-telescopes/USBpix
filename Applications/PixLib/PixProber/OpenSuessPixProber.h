/////////////////////////////////////////////////////////////////////
// SuessPixprober.h
// version 1.0
// Joern Grosse-Knetter, Kevin Kroeninger, University of Goettingen
/////////////////////////////////////////////////////////////////////

//! Class for Suess probe stations

#ifndef _PIXLIB_OPENSUESSPIXPROBER
#define _PIXLIB_OPENSUESSPIXPROBER

#include "PixProber.h"
#include <memory>
#include <Suess.h>

namespace PixLib {

  class OpenSuessPixProber : public PixProber {

  public:
    
    /** The default constructor. */ 
    OpenSuessPixProber(DBInquire *dbInquire=0); 
    
    /** The default destructor. */     
    ~OpenSuessPixProber(); 
    
    void initHW();

    // movement functions
    void chuckGotoFirst();
    void chuckContact();
    void chuckSeparate();
    bool chuckNextDie();
    void chuckGotoPosition(int col, int row, int type);
    void chuckGetPosition(int &col, int &row, int &type);
    void chuckGetIndex(int &index);
    void chuckGotoIndex(int index);

    void MoveChuck(float x, float y);
    void MoveChuckZ(float z);
    void ReadChuckPosition(float &x, float &y, float &z);

    void MoveScope(float x, float y);
    void MoveScopeZ(float z);
    void ReadScopePosition(float &x, float &y, float &z);
    

  private:
    void configInit();   //! Init configuration structure

    void connectPS();
    void disconnectPS();
    void runProberCommand(char *command, char *response);

    std::string m_myIP;
    int m_port;
    bool m_notify;
    bool m_wasInit;
    
    
 
    std::auto_ptr<Suess::ProbeStation> m_ps;
  };

}

#endif

