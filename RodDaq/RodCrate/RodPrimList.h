// File: RodPrimList.h

#ifndef SCTPIXELROD_RODPRIMLIST_H
#define SCTPIXELROD_RODPRIMLIST_H

#include <list>
#include <string>
#include <fstream> 
#include <iostream>
#include "RodPrimitive.h"
#include "BaseException.h"
#include "dsp_types.h"
#ifdef IBL
#include "iblPrimXface.h"
#endif
//#else
#include "primXface.h"
//#endif

namespace SctPixelRod {

/*!
 * @class PrimListException
 *
 * @brief This is a class to handle exceptions in the PrimList Class.
 *
 * @author Tom Meyer (meyer@iastate.edu) - originator
 */

class PrimListException : public BaseException {
public:
  PrimListException( std::string descriptor, unsigned long data1, unsigned long data2);
  unsigned long getData1() {return m_data1;};
  unsigned long getData2() {return m_data2;};
  virtual std::ostream& what(std::ostream&, PrimListException&);
  
private:
  unsigned long m_data1;    // First data value returned
  unsigned long m_data2;    // Second data value returned
};                                                                            

/*!
 * @class RodPrimList
 *
 * @brief This is a list of RodPrimitives to be sent to the RodModule.
 *
 * This class is a list of the primitives that control ROD behavior.  
 * It is sent to the RodModule via its Send function.  As it is derived 
 * from the STL list, it inherits all the usual list functionality.
 *
 * @author Tom Meyer (meyer@iastate.edu) - originator
 */

class RodPrimList : public std::list<RodPrimitive> {
  private:
    //! Buffer to send
    unsigned long *m_buffer;
    //! Buffer size
    unsigned long m_bufferSize;
    //! List header
    MsgListHead m_listhead;
    //! List tail
    MsgListTail m_listtail; 

  public:
    enum PrimState {IDLE, LOADED, EXECUTING, WAITING, PAUSED};

    //! Constructor with specified index
    RodPrimList(unsigned long theIndex) : 
      std::list<RodPrimitive>(), m_buffer(0), m_bufferSize(0) 
      {m_listhead.index = theIndex;};
    //! Default constructor
    RodPrimList() : 
      std::list<RodPrimitive>(), m_buffer(0), m_bufferSize(0) 
      {m_listhead.index = 0;};
    //! Copy constructor 
    RodPrimList(const RodPrimList &rhs); 
    //! Overloaded assignment
    RodPrimList &operator=(const RodPrimList&);
    //! Destructor
    ~RodPrimList();

    //! Compute list checksum
    unsigned long checkSum();   
    //! Compute number of words in list
    unsigned long numWords();   

    //! Create the buffer to send
    void bufferBuild() throw(PrimListException &);
    
    //! Write the buffer to an XML file. Buffer must already be built when this is called.
    void writeToXml(std::string& xmlFile) throw(PrimListException &);

    //! Create the buffer from an XML file
    void buildFromXml(std::string& xmlFile) throw(PrimListException &);
    
    //! Return the buffer lingth.
    long getBufferLength() { return m_buffer[0]; };

    //! Return the buffer.
    unsigned long *getBuffer() { return m_buffer; };

    //! Print the primitive list
    void print();

    //! Clear the primitive list
    void clear();

    //! Set the list identifier index
    void setIndex(unsigned long i) { m_listhead.index = i; }  
    //! Get the list identifier index
    unsigned long getIndex() const { return m_listhead.index; } ;    

    // Set the version number
    // void setVersion(unsigned long version) { m_listhead.index=version; }  
    // Get the version number
    // unsigned long getVersion() const { return m_version; } ;    
};

} // End namespace SctPixelRod

  std::ostream& operator<<(std::ostream& os, SctPixelRod::PrimListException& primExcept);

#endif // SCTPIXELROD_RODPRIMLIST_H
