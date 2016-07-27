//File BaseException.h

#ifndef SCTPIXELROD_BASEEXCEPTION_H
#define SCTPIXELROD_BASEEXCEPTION_H

#include <iostream>
#include <string>

namespace SctPixelRod {

enum exceptType {BASE, VME, NOIMAGEFILE, HPI, ROD, PRIMLIST, TIM, BOC};

//------------------------------baseException------------------------------ 

/*! @class baseException
 *
 *  @brief This is a general exception base class for ROD software.
 *
 *  Classes inherited from this class are thrown if an error in a ROD 
 *  operation is detected. In general, this class supports a string message
 *  and inherited classes can then add additional parameters to report as
 *  needed.  All classes should support overloading the insertion operator
 *  (<<) so that users can print a message to cout or cerr simply by inserting
 *  the exception object into the desired ostream object.
 *
 *  @author Tom Meyer (meyer@iastate.edu) - originator
 */

class BaseException {
public:
  BaseException(std::string descriptor);
  std::string getDescriptor() const {return m_descriptor;}
  unsigned long getNumData() const {return m_numData;}
  void setType(exceptType type) {m_type = type; return;}
  exceptType getType() const {return m_type;}
  virtual void what(std::ostream&) const;
  virtual ~BaseException(){}
  
private:
  std::string m_descriptor;    //!< A text description of the error
  unsigned long m_numData;  //!< Number of data values returned
  exceptType m_type;              //!!< Type of exception (filled by derived classes)
}; 
                                                                         
};

//---------------------------- Overload operator<< -----------------------------
/* This overloaded operator lets us use cout and cerr to print the sexception
*  information.
*/
  std::ostream& operator<<(std::ostream& os, const SctPixelRod::BaseException& e);

#endif // SCTPIXELROD_BASEEXCEPTION_H
