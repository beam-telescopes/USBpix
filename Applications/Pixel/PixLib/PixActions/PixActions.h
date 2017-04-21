/////////////////////////////////////////////////////////////////////
// PixActions.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 31/03/06  Version 1.0 (CS)
//           Initial release
//

//! Base class for the Pixel actions

#ifndef _PIXLIB_ACTIONS
#define _PIXLIB_ACTIONS



namespace PixLib {
  
  class PixActions {

  public:
    
    enum Type {ANY_TYPE, SINGLE_ROD, SINGLE_TIM, MULTI, MULTI_IPC, MULTI_REMOTE};

    enum Assignment {ANY_ASSIGNMENT, RCD, EXPERT};

    struct PixActionsDescriptor {
      std::string name;
      bool available;
      Type type;
      Assignment assignment;
      std::string brokerName;

      friend bool operator==(PixActionsDescriptor &d1, PixActionsDescriptor &d2) {
	return(d1.name==d2.name &&
	       d1.available==d2.available &&
	       d1.type==d2.type &&
	       d1.assignment==d2.assignment && 
	       d1.brokerName==d2.brokerName);
      }

      friend bool operator<(PixActionsDescriptor &d1, PixActionsDescriptor &d2) {
	return(d1.name<d2.name);
      }
    };
    
  public:
    
    // Destructor
    virtual ~PixActions() {}
    
    // State transitions
    virtual void load() = 0;
    virtual void configure() = 0;
    virtual void prepareForRun() = 0;
    virtual void startTrigger() = 0;
    virtual void stopTrigger() = 0;
    virtual void stopFE() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void unconfigure() = 0;
    virtual void unload() = 0;
    
    // Basic hardware operations
    virtual void reset() = 0;
    
    // Configuration handling
    virtual void readConfig() = 0;
    virtual void loadConfig() = 0;
    virtual void sendConfig() = 0;
    
    // Resource allocation
    virtual void allocate() {m_descriptor.available=false;}
    virtual void deallocate() {m_descriptor.available=true;}
    virtual void assign(Assignment assignment) {m_descriptor.assignment=assignment;}
    virtual void rename(std::string name) {m_descriptor.name=name;} // CMQL - must disappear... for test only
    
    // Descriptor
    virtual struct PixActionsDescriptor descriptor() {return m_descriptor;}

    virtual std::string name() {return m_descriptor.name;}
    virtual std::string brokerName() {return m_descriptor.brokerName;}
    virtual bool available() {return m_descriptor.available;}
    virtual Type type() {return m_descriptor.type;}
    virtual Assignment assignment() {return m_descriptor.assignment;}

  protected:

    // Actions descriptor
    struct PixActionsDescriptor m_descriptor;
    
  };
  
};

#endif // _PIXLIB_ACTIONS
