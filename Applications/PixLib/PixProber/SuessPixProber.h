/////////////////////////////////////////////////////////////////////
// SuessPixprober.h
// version 1.0
// Joern Grosse-Knetter, Kevin Kroeninger, University of Goettingen
/////////////////////////////////////////////////////////////////////

//! Class for Suess probe stations

#ifndef _PIXLIB_SUESSPIXPROBER
#define _PIXLIB_SUESSPIXPROBER

#include "PixProber.h"

namespace PixLib {
  
  class SuessPixProber : public PixProber {

  public:
    
    /** The default constructor. */ 
    SuessPixProber(DBInquire *dbInquire=0); 
    
    /** The default destructor. */     
    ~SuessPixProber(); 
    
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

  private:
    void configInit();   //! Init configuration structure

    // internal communication with the nclient functions
    void connectPS();
    void disconnectPS();
    void runProberCommand(const char *command, char *response);

    std::string m_myIP;
    int m_port;
    bool m_notify;
    bool m_wasInit;

  };

}

#endif

