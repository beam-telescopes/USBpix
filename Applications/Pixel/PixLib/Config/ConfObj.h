/////////////////////////////////////////////////////////////////////
// ConfObj.h 
// version 4.0
/////////////////////////////////////////////////////////////////////
//
// 26/09/99  Version 0.1 (PM)
//  7/10/99  Version 0.2 (PM)
//  4/11/99  Version 0.3 (PM)
// 25/07/01  Version 2.1.0 (PM)
// 26/03/04  Version 3.0 - Imported from SimPix (PM)
// 20/07/06  Version 4.0 - Interface with PixDbInterface (PM)
//

#ifndef PIXLIB_CONF_CF_OBJ
#define PIXLIB_CONF_CF_OBJ

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#pragma warning(disable: 4100)
#endif

#include "BaseException.h"

#include <string>
#include <map>
#include <vector>

namespace PixLib {
  
  class DBInquire;
  class DbRecord;

  template<class T> class ConfMask;
  
  //! Config Exception class
  class ConfigExc : public SctPixelRod::BaseException{
  public:
    //! Constructor
    ConfigExc(std::string name) : BaseException(name) {}; 
    //! Destructor
    virtual ~ConfigExc() {};

    //! Dump the error
    virtual void dump(std::ostream &out) {
     out << "Config " << getDescriptor() << std::endl; 
    }
  private:
  };

  class ConfObj {
  public:
    enum types { INT, FLOAT, VECTOR, MATRIX, LIST, BOOL, STRING, VOID };
    
    // Constructors distructors and copy
    ConfObj(std::string nam, std::string comm, bool vis, types typ);
    virtual ~ConfObj();

    virtual void copy(const ConfObj &) { };

    // Methods
    virtual void reset() { };
    virtual bool read(DBInquire *) { return false; };
    virtual bool write(DBInquire *) { return false; };
    virtual bool read(DbRecord *) { return false; };
    virtual bool write(DbRecord *) { return false; };
    virtual void dump(std::ostream &) {};
    virtual std::string name() { return m_name; };
    virtual bool visible() { return m_visible; };
    virtual void setVisible(bool vis) { m_visible = vis; };
    virtual std::string comment() { return m_comment; };
    virtual types type() { return m_type; };

    virtual int& valueInt() { throw ConfigExc("Incompatible value type"); };
    virtual unsigned int& valueUInt() { throw ConfigExc("Incompatible value type"); };
    virtual float& valueFloat() { throw ConfigExc("Incompatible value type"); };
    virtual bool& valueBool() { throw ConfigExc("Incompatible value type"); };
    virtual std::string& valueString() { throw ConfigExc("Incompatible value type"); };

    // Data
    std::string m_name;
    bool m_visible;
    std::string m_comment;
    types m_type;
  };
  
  class ConfInt : public ConfObj {
  public:
    enum subtypes { S32, U32, S16, U16, S8, U8 };
    
    // Constructors and distructors
    ConfInt(std::string nam, int &val, int def, std::string comm, bool vis);
    ConfInt(std::string nam, int &val, int def, std::string comm, bool vis, types typ);
    ConfInt(std::string nam, unsigned int &val, unsigned int def, std::string comm, bool vis);
    ConfInt(std::string nam, unsigned int &val, unsigned int def, std::string comm, bool vis, types typ);
    ConfInt(std::string nam, short int &val, short int def, std::string comm, bool vis);
    ConfInt(std::string nam, short int &val, short int def, std::string comm, bool vis, types typ);
    ConfInt(std::string nam, unsigned short int &val, unsigned short int def, std::string comm, bool vis);
    ConfInt(std::string nam, unsigned short int &val, unsigned short int def, std::string comm, bool vis, types typ);
    ConfInt(std::string nam, char &val, char def, std::string comm, bool vis);
    ConfInt(std::string nam, char &val, char def, std::string comm, bool vis, types typ);
    ConfInt(std::string nam, unsigned char &val, unsigned char def, std::string comm, bool vis);
    ConfInt(std::string nam, unsigned char &val, unsigned char def, std::string comm, bool vis, types typ);
    virtual ~ConfInt();

    virtual void copy(const ConfObj &obj);
    
    // Methods
    virtual void reset();
    virtual bool read(DBInquire *dbi);
    virtual bool write(DBInquire *dbi);
    virtual bool read(DbRecord *dbr);
    virtual bool write(DbRecord *dbr);
    virtual void dump(std::ostream &out);
    int getValue();
    void setValue(int val);
    int &value() { return *((int *)m_value); };
    int &valueS32() { return *((int *)m_value); };
    unsigned int &valueU32() { return *((unsigned int *)m_value); };
    short int &valueS16() { return *((short int *)m_value); };
    unsigned short int &valueU16() { return *((unsigned short int *)m_value); };
    char &valueS8() { return *((char *)m_value); };
    unsigned char &valueU8() { return *((unsigned char *)m_value); };
    subtypes subtype() { return m_subtype; };
    virtual int& valueInt() { 
      if (m_subtype == S32) {
        return *((int *)m_value); 
      } else {
        throw ConfigExc("Incompatible value type");
      }
    };
    virtual unsigned int& valueUInt() {
      if (m_subtype == U32) {
        return *((unsigned int *)m_value);
      } else {
        throw ConfigExc("Incompatible value type");
      }
    };    
    // Data
    void *m_value;
    int m_defS32;
    unsigned int m_defU32;
    short int m_defS16;
    unsigned short int m_defU16;
    char m_defS8;
    unsigned char m_defU8;
    subtypes m_subtype;
  };
  
  class ConfFloat : public ConfObj {
  public:
    // Constructors and distructors
    ConfFloat(std::string nam, float &val, float def, std::string comm, bool vis);
    virtual ~ConfFloat();
    virtual void copy(const ConfObj &obj);
    
    // Methods
    virtual void reset() { m_value = m_defval; };
    virtual bool read(DBInquire *dbi);
    virtual bool write(DBInquire *dbi);
    virtual bool read(DbRecord *dbr);
    virtual bool write(DbRecord *dbr);
    virtual void dump(std::ostream &out);
    float &value() { return m_value; };
    virtual float& valueFloat() { return m_value; };
    
    // Data
    float &m_value;
    float m_defval;
  };
  
  class ConfVector : public ConfObj {
  public:
    enum subtypes { INT, UINT, FLOAT };
    
    // Constructors and distructors
    ConfVector(std::string nam, std::vector<int> &val, std::vector<int> def, std::string comm, bool vis);
    ConfVector(std::string nam, std::vector<unsigned int> &val, std::vector<unsigned int> def, std::string comm, bool vis);
    ConfVector(std::string nam, std::vector<float> &val, std::vector<float> def, std::string comm, bool vis);
    virtual ~ConfVector();
    virtual void copy(const ConfObj &obj);
    
    // Methods
    virtual void reset();
    virtual bool read(DBInquire *dbi);
    virtual bool write(DBInquire *dbi);
    virtual bool read(DbRecord *dbr);
    virtual bool write(DbRecord *dbr);
    virtual void dump(std::ostream &out);
    
    std::vector<int> &valueVInt() { return *((std::vector<int> *)m_value); };
    std::vector<unsigned int> &valueVUint() { return *((std::vector<unsigned int> *)m_value); };
    std::vector<float> &valueVFloat() { return *((std::vector<float> *)m_value); };
    
    subtypes subtype() { return m_subtype; };
    
    // Data
    void *m_value;
    std::vector<int> m_defInt;
    std::vector<unsigned int> m_defUint;
    std::vector<float> m_defFloat;
    subtypes m_subtype;
  };
  
  class ConfMatrix : public ConfObj {
  public:
    enum subtypes { U16, U1, FLOAT };
    
    // Constructors and distructors
    ConfMatrix(std::string nam, ConfMask<unsigned short int> &val, ConfMask<unsigned short int> def, std::string comm, bool vis);
    ConfMatrix(std::string nam, ConfMask<bool> &val, ConfMask<bool> def, std::string comm, bool vis);
    ConfMatrix(std::string nam, ConfMask<float> &val, ConfMask<float> def, std::string comm, bool vis);
    virtual ~ConfMatrix();
    virtual void copy(const ConfObj &obj);

    // Methods
    virtual void reset();
    virtual bool read(DBInquire *dbi);
    virtual bool write(DBInquire *dbi);
    virtual bool read(DbRecord *dbr);
    virtual bool write(DbRecord *dbr);
    virtual void dump(std::ostream &out);
    
    ConfMask<unsigned short int> &valueU16() { return *((ConfMask<unsigned short int> *)m_value); };
    ConfMask<bool> &valueU1() { return *((ConfMask<bool> *)m_value); };
    
    subtypes subtype() { return m_subtype; };
    
    // Data
    void *m_value;
    ConfMask<unsigned short int> m_defU16;
    ConfMask<bool> m_defU1;
    ConfMask<float> m_defFloat;
    subtypes m_subtype;
  };
  
  class ConfList : public ConfInt {
  public:
    // Constructors and distructors
    ConfList(std::string nam, int &val, int def, std::map<std::string, int>symb, std::string comm, bool vis);
    ConfList(std::string nam, unsigned int &val, unsigned int def, std::map<std::string, int>symb, std::string comm, bool vis);
    ConfList(std::string nam, short int &val, short int def, std::map<std::string, int>symb, std::string comm, bool vis);
    ConfList(std::string nam, unsigned short int &val, unsigned short int def, std::map<std::string, int>symb, std::string comm, bool vis);
    ConfList(std::string nam, char &val, char def, std::map<std::string, int>symb, std::string comm, bool vis);
    ConfList(std::string nam, unsigned char &val, unsigned char def, std::map<std::string, int>symb, std::string comm, bool vis);
    virtual ~ConfList();
    virtual void copy(const ConfObj &obj);

    // Methods
    virtual bool read(DBInquire *dbi);
    virtual bool write(DBInquire *dbi);
    virtual bool read(DbRecord *dbr);
    virtual bool write(DbRecord *dbr);
    virtual void dump(std::ostream &out);
    virtual std::string sValue();
    virtual std::map<std::string, int>& symbols() { return m_symbols; };
     
    // Data
    std::map<std::string, int> m_symbols;
  };
  
  class ConfBool : public ConfObj {
  public:
    // Constructors and distructors
    ConfBool(std::string nam, bool &val, bool def, std::string comm, bool vis);
    ConfBool(std::string nam, bool &val, bool def, std::string y, std::string n, std::string comm, bool vis);
    virtual ~ConfBool();
    virtual void copy(const ConfObj &obj);
    
    // Methods
    virtual void reset() { m_value = m_defval; };
    virtual bool read(DBInquire *dbi);
    virtual bool write(DBInquire *dbi);
    virtual bool read(DbRecord *dbr);
    virtual bool write(DbRecord *dbr);
    virtual void dump(std::ostream &out);
    bool &value() { return m_value; };
    virtual bool& valueBool() { return m_value; };
    
    // Data
    bool &m_value;
    bool m_defval;
    std::string m_yes;
    std::string m_no;
  };
  
  class ConfString : public ConfObj {
  public:
    // Constructors and distructors
    ConfString(std::string nam, std::string &val, std::string def, std::string comm, bool vis, int type);
    virtual ~ConfString();
    virtual void copy(const ConfObj &obj);
    
    // Methods
    virtual void reset() { m_value = m_defval; };
    virtual bool read(DBInquire *dbi);
    virtual bool write(DBInquire *dbi);
    virtual bool read(DbRecord *dbr);
    virtual bool write(DbRecord *dbr);
    virtual void dump(std::ostream &out);
    std::string &value() { return m_value; };
    virtual std::string& valueString() { return m_value; };
    int getType(){return m_type;};
    
    // Data
    std::string &m_value;
    std::string m_defval;
    int m_type; // 0: arb. string, 1: file path, 2: dir. path, 3: file name
  };
  
}

#endif
