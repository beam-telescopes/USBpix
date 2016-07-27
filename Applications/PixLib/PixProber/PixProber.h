/////////////////////////////////////////////////////////////////////
// Pixprober.h
// version 1.0
// Joern Grosse-Knetter, Kevin Kroeninger, University of Goettingen
/////////////////////////////////////////////////////////////////////

//! Abstract class for probe stations

#ifndef _PIXLIB_PIXPROBER
#define _PIXLIB_PIXPROBER

#include <string>
#include <vector>
#include "BaseException.h"

namespace PixLib {

  class DBInquire;
  class Config;
  
  //! PixDcs Exception class; an object of this type is thrown in case of a Dcs communication or setting error
  class PixProberExc : public SctPixelRod::BaseException{
  public:
    enum ErrorLevel{INFO, WARNING, ERROR, FATAL};
    PixProberExc(ErrorLevel el, std::string descr) : BaseException(descr), m_errorLevel(el), m_descr(descr) {}; 
      virtual ~PixProberExc() {};

      //! Dump the error
      virtual void dump(std::ostream &out) {
	out << dumpLevel() << ":" << m_descr << std::endl; 
      }
      std::string dumpLevel() {
	switch (m_errorLevel) {
	case INFO : 
	  return "INFO";
	case WARNING :
	  return "WARNING";
	case ERROR :
	  return "ERROR";
	case FATAL :
	  return "FATAL";
	default :
	  return "UNKNOWN";
	}
      }
      //! m_errorLevel accessor
      ErrorLevel getErrorLevel() { return m_errorLevel; };
      std::string getDescr() { return m_descr; };

  private:
      ErrorLevel m_errorLevel;
      std::string m_descr;
  };

  class PixProber {
  friend class PixProberDcs;
  public:

    // generate different types of probestations
    static PixProber* make(DBInquire *dbInquire, std::string type);    //! Factory
    static void listTypes(std::vector<std::string> &list);             //! List available types

    /** The default constructor. */ 
    PixProber(DBInquire *dbInquire); 
    
    /** The default destructor. */     
    virtual ~PixProber(){}; 
    
    virtual void initHW() = 0; // initialise hardware - first thing to do

    // movement functions
    virtual void chuckGotoFirst()=0;
    virtual void chuckContact()=0;
    virtual void chuckSeparate()=0;
    virtual bool chuckNextDie()=0;
    virtual void chuckGotoPosition(int col, int row, int type)=0;
    virtual void chuckGetPosition(int &col, int &row, int &type)=0;
    virtual void chuckGetIndex(int &index)=0;
    virtual void chuckGotoIndex(int index)=0;
   
    virtual float returnPosX(){return m_posX;}
    virtual float returnPosY(){return m_posY;}
    virtual float returnPosZ(){return m_posZ;}
    
    virtual void MoveChuck(float x, float y);
    virtual void MoveChuckZ(float z);
    virtual void ReadChuckPosition(float &x, float &y, float &z);

    virtual void MoveScope(float x, float y);
    virtual void MoveScopeZ(float z);
    virtual void ReadScopePosition(float &x, float &y, float &z);
    // general functions
    std::string name() { return m_name; };               //! name accessor
    std::string decName() { return m_decName; };         //! decorated name (of DB record) accessor
    Config &config() { return *m_conf; };                //! Configuration object accessor
    
  protected:
    virtual void configInit() = 0;   //! Init configuration structure
    
    Config *m_conf;                  //! Configuration object
    DBInquire *m_dbInquire;
    std::string m_name, m_decName;
    float m_posX, m_posY, m_posZ;
    


  };

  class DummyPixProber : public PixProber {

  public:
    
    /** The default constructor. */ 
    DummyPixProber(DBInquire *dbInquire=0); 
    
    /** The default destructor. */     
    ~DummyPixProber(); 
    
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

    void ReadChuckPosition(float &x, float &y, float &z);
    void MoveChuck(float x, float y);
    void MoveChuckZ(float z);
  
    void ReadScopePosition(float &x, float &y, float &z);
    void MoveScope(float x, float y);
    void MoveScopeZ(float z);

private:
    void configInit();   //! Init configuration structure

    int m_dummyCfgVar;
    bool m_wasInit;
    int m_col, m_row, m_index;
    float m_x, m_y, m_z;

  };
}

#endif

