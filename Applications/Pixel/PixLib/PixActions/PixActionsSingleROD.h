/////////////////////////////////////////////////////////////////////
// PixActionsSingleROD.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 31/03/06  Version 1.0 (CS)
//           Initial release
//

//! Class for the Pixel single ROD actions

#ifndef _PIXLIB_ACTIONSSINGLEROD
#define _PIXLIB_ACTIONSSINGLEROD

#include "PixActions/PixActions.h"



// Forward declarations
namespace SctPixelRod {
  class VmeInterface;
};


namespace PixLib{
  
  // Forward declarations
  class DBInquire;
  class PixConfDBInterface;
  class PixModuleGroup;
  
  
  class PixActionsSingleROD : public PixActions {
    
  public:
    
    // Constructor
    PixActionsSingleROD(std::string brokerName, SctPixelRod::VmeInterface *vme, PixConfDBInterface *confDB,
			DBInquire *moduleGroupInquire);
    
    // Destructor
    virtual ~PixActionsSingleROD() {}
    
    // State transitions
    virtual void load();
    virtual void configure();
    virtual void prepareForRun();
    virtual void startTrigger();
    virtual void stopTrigger();
    virtual void stopFE();
    virtual void pause();
    virtual void resume();
    virtual void unconfigure();
    virtual void unload();
    
    // Detailed state transitions
    void load_ini();
    void load_run();
    void load_end();
    void configure_ini();
    void configure_run();
    void configure_end();
    
    // Basic hardware operations
    virtual void reset();
    
    // Configuration handling
    virtual void readConfig();
    virtual void loadConfig();
    virtual void sendConfig();
    
  private:
    
    // SW interfaces
    SctPixelRod::VmeInterface  *m_vme;
    PixConfDBInterface *m_confDB;
    
    // PixLib objects
    DBInquire      *m_moduleGroupInquire;
    PixModuleGroup *m_moduleGroup;
    
  };
  
};


#endif // _PIXRCD_ACTIONSSINGLEROD
