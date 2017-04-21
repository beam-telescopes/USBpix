/////////////////////////////////////////////////////////////////////
// PixActionsMulti.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 18/04/06  Version 1.0 (CS)
//           Initial release - Single threaded version
//

//! Class for the Pixel multiple actions

#ifndef _PIXLIB_ACTIONSMULTI
#define _PIXLIB_ACTIONSMULTI

#include "PixActions/PixActions.h"



// Forward declarations
namespace SctPixelRod {
  class VmeInterface;
};


namespace PixLib{
  
  // Forward declarations
  class PixConfDBInterface;
  
  
  class PixActionsMulti : public PixActions {
    
  public:
    
    // Constructor
    PixActionsMulti(std::string brokerName, SctPixelRod::VmeInterface *vme, PixConfDBInterface *confDB,
		    std::vector<PixActions*> subActions);
    
    // Destructor
    virtual ~PixActionsMulti() {}
    
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
    
    // Basic hardware operations
    virtual void reset();
    
    // Configuration handling
    virtual void readConfig();
    virtual void loadConfig();
    virtual void sendConfig();
    
    // Resource allocation
    virtual void allocate();
    virtual void deallocate();
    virtual void assign(Assignment assignment);
    
  private:
    
    // SW interfaces
    SctPixelRod::VmeInterface *m_vme;
    PixConfDBInterface *m_confDB;
    
    // PixLib objects
    std::vector<PixActions*> m_subActions;
    
  };
  
};


#endif // _PIXRCD_ACTIONSMULTI
