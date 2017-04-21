/////////////////////////////////////////////////////////////////////
// ConfGroup.h 
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

#ifndef PIXLIB_CONF_GROUP
#define PIXLIB_CONF_GROUP

#include <vector>
#include <map>
#include <string>
#include "Config/ConfMask.h"
#include "Config/ConfObj.h"


namespace PixLib {

class DBInquire;
class DbRecord;

class ConfGroup {
public:
  ConfGroup(std::string name) { m_groupName = name; };    // Constructor
  ~ConfGroup();                                           // Destructor

  void copy(const ConfGroup &grp);                        // Copy values to another ConfGrp
  void copyRef(const ConfGroup &grp);                     // Copy entire structure to another ConfGrp, include var. references

  void addInt(std::string name, int &val, int def, std::string comm, bool vis);                                   // Adds an integer
  void addInt(std::string name, unsigned int &val, unsigned int def, std::string comm, bool vis);                 // Adds an integer
  void addInt(std::string name, short int &val, short int def, std::string comm, bool vis);                       // Adds an integer
  void addInt(std::string name, unsigned short int &val, unsigned short int def, std::string comm, bool vis);     // Adds an integer
  void addInt(std::string name, char &val, char def, std::string comm, bool vis);                                 // Adds an integer
  void addInt(std::string name, unsigned char &val, unsigned char def, std::string comm, bool vis);               // Adds an integer
  void addFloat(std::string name, float &val, float def, std::string comm, bool vis);                             // Adds a float
  void addVector(std::string name, std::vector<int> &val, std::vector<int> def, std::string comm, bool vis);      // Adds a vector
  void addVector(std::string name, std::vector<unsigned int> &val, std::vector<unsigned int> def, std::string comm, bool vis);      // Adds a vector
  void addVector(std::string name, std::vector<float> &val, std::vector<float> def, std::string comm, bool vis);  // Adds a vector
  void addMatrix(std::string name, ConfMask<unsigned short int> &val, ConfMask<unsigned short int> def, std::string comm, bool vis); // Adds a matrix
  void addMatrix(std::string name, ConfMask<bool> &val, ConfMask<bool> def, std::string comm, bool vis);          // Adds a matrix
  void addMatrix(std::string name, ConfMask<float> &val, ConfMask<float> def, std::string comm, bool vis);          // Adds a matrix
  void addList(std::string name, int &val, int def, std::map<std::string, int> symb, std::string comm, bool vis); // Adds a list
  void addList(std::string name, unsigned int &val, unsigned int def, std::map<std::string, int> symb, std::string comm, bool vis); // Adds a list
  void addList(std::string name, short int &val, short int def, std::map<std::string, int> symb, std::string comm, bool vis); // Adds a list
  void addList(std::string name, unsigned short int &val, unsigned short int def, std::map<std::string, int> symb, std::string comm, bool vis); // Adds a list
  void addList(std::string name, char &val, char def, std::map<std::string, int> symb, std::string comm, bool vis); // Adds a list
  void addList(std::string name, unsigned char &val, unsigned char def, std::map<std::string, int> symb, std::string comm, bool vis); // Adds a list
  void addBool(std::string name, bool &val, bool def, std::string comm, bool vis);                                // Adds a bool
  void addBool(std::string name, bool &val, bool def, std::string y, std::string n, std::string comm, bool vis);  // Adds a bool
  void addString(std::string name, std::string &val, std::string def, std::string comm, bool vis, int type=0);    // Adds a string

  bool read(DBInquire *dbi);                              // Reads from DB
  bool write(DBInquire *dbi);                             // Writes to DB
  bool read(DbRecord *dbr);                               // Reads from DB
  bool write(DbRecord *dbr);                              // Writes to DB
  void dump(std::ostream &out);                           // Dump group content
  void reset();                                           // Set all parameters to default
  std::string name() { return m_groupName; };             // Returns group name
  int size() { return m_param.size(); };                  // Returns the size
  ConfObj &operator[](int i) { return *m_param[i]; };
  ConfObj &operator[](std::string name);

  //private: 
  std::vector<ConfObj*> m_param;
  std::string m_groupName;
};

}

#endif
