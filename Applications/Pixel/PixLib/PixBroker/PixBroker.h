/////////////////////////////////////////////////////////////////////
// PixBroker.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 03/04/06  Version 1.0 (CS)
//           Initial release
//

//! Base class for the Pixel resource broker

#ifndef _PIXLIB_BROKER
#define _PIXLIB_BROKER


#include <string>
#include <list>
#include "PixActions/PixActions.h"



namespace PixLib{

  // Forward declarations
  class DBInquire;


  class PixBroker {

  public:
    
    // Destructor
    virtual ~PixBroker() {}
    
    // Resource listing
    virtual std::list<std::string> listSubBrokers() = 0;

    virtual std::list<PixActions::PixActionsDescriptor> listActions(bool checkAvailability = true,
								    PixActions::Type type = PixActions::ANY_TYPE,
								    PixActions::Assignment assignment=PixActions::ANY_ASSIGNMENT,
								    std::string brokerName="") = 0;

    // Resource allocation
    virtual PixActions* allocateActions(PixActions::PixActionsDescriptor actionName) = 0;
    virtual PixActions* allocateActions(std::list<PixActions::PixActionsDescriptor> actionDescrVect, 
					PixActions::Type type = PixActions::ANY_TYPE) = 0;

    // Resource management
    virtual void createActions() = 0;
    virtual void destroyActions() = 0;

    // Name
    std::string name() {return m_name;}

  protected:

    // Broker name
    std::string m_name;    
  };
  
};

#endif // _PIXLIB_BROKER
