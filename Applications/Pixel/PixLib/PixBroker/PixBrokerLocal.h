/////////////////////////////////////////////////////////////////////
// PixBrokerLocal.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 03/04/06  Version 1.0 (CS)
//           Initial release
//

//! Class for the Pixel local resource broker

#ifndef _PIXLIB_BROKERLOCAL
#define _PIXLIB_BROKERLOCAL

#include "PixBroker/PixBroker.h"



namespace PixLib {

  // Forward declarations
  class PixActions;
  class DBInquire;
    

  class PixBrokerLocal : public PixBroker {
    
  public:
    
    // Constructor
    PixBrokerLocal(SctPixelRod::VmeInterface *vme, PixConfDBInterface *confDB, DBInquire *crateInquire);
    
    // Destructor
    virtual ~PixBrokerLocal();
    
    // Resource listing
    virtual std::list<std::string> listSubBrokers();

    virtual std::list<PixActions::PixActionsDescriptor> listActions(bool checkAvailability = true,
								    PixActions::Type type = PixActions::ANY_TYPE,
								    PixActions::Assignment assignment=PixActions::ANY_ASSIGNMENT,
								    std::string brokerName="");

    // Resource allocation
    virtual PixActions* allocateActions(PixActions::PixActionsDescriptor actionName);
    virtual PixActions* allocateActions(std::list<PixActions::PixActionsDescriptor> actionDescrVect, 
					PixActions::Type type = PixActions::ANY_TYPE);

    // Resource management
    virtual void createActions();
    virtual void destroyActions();

  private:
    
    // SW interfaces
    SctPixelRod::VmeInterface  *m_vme;
    PixConfDBInterface *m_confDB;
    
    // PixLib objects
    DBInquire *m_crateInquire;

    // Actions vectors
    std::vector<PixActions*> m_actions;
  };
  
};

#endif // _PIXLIB_BROKERLOCAL
