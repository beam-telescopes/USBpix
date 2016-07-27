// File: RodPrimList.cxx
// $Header$

#include "RodPrimList.h"
#include <string.h>

namespace SctPixelRod {

//***************************Class PrimListException**************************
//
// Description:
//   This is a class to handle exceptions in the PrimList Class.
//
//
// @author Tom Meyer (meyer@iastate.edu) - originator
//

PrimListException::PrimListException( std::string descriptor, unsigned long data1,
                            unsigned long data2) : BaseException(descriptor) {
  m_data1 = data1;
  m_data2 = data2;
  setType(PRIMLIST);
  }

std::ostream& PrimListException::what(std::ostream& os, PrimListException& primEx) {
    os << "PrimListException: " << primEx.getDescriptor() << std::endl;
    os << "Data1:" << primEx.getData1() << std::endl;
    os << "Data2:" << primEx.getData2() << std::endl;
   return os;
}  
  
// Description:
//  Class RodPrimList is a list of the primitives that control ROD behavior.  
// It is sent to the RodModule via its Send function.  As it is derived 
// from the STL list, it inherits all the usual list functionality.
//
// @author Tom Meyer (meyer@iastate.edu) - originator
//
RodPrimList::RodPrimList(const RodPrimList &rhs) :
  std::list<RodPrimitive>(rhs) {
  m_listhead.index=rhs.getIndex();
}

RodPrimList &RodPrimList::operator=(const RodPrimList &rhs) {
  if (this == &rhs)
    return *this;
  std::list<RodPrimitive>::operator=(rhs);
  m_listhead.index=rhs.getIndex();
  if (rhs.m_buffer) {
    m_bufferSize=rhs.m_bufferSize;
    std::copy(rhs.m_buffer, rhs.m_buffer+rhs.m_bufferSize, m_buffer);
  } else {
    m_bufferSize=0;
    m_buffer=0;
  }
  return *this;
}

RodPrimList::~RodPrimList() {
  if (m_buffer) 
    delete [] m_buffer;
}

unsigned long RodPrimList::numWords() {
  // List head and tail 
  unsigned long n = (sizeof(MsgListHead)+sizeof(MsgListTail))/sizeof(UINT32); 
  RodPrimList::iterator i; 
  for (i = begin(); i != end(); i++)
    n += i->getLength();
  return n;
}

unsigned long RodPrimList::checkSum() {
  unsigned long c = m_buffer[0];; 
  for (unsigned long i = 1; i < m_bufferSize-1; i++) {
    c ^= m_buffer[i];
  }
  return c;
}

void RodPrimList::bufferBuild() throw(PrimListException &) {
  long unsigned int bufPos = 0;
  long unsigned int bufPosSave = 0;

  m_bufferSize = numWords();
  m_buffer = new unsigned long[m_bufferSize]; // call clear() to delete m_buffer
  if (0 == m_buffer) throw PrimListException(
      "Bufferbuild unable to allocate buffer, bufferSize, index:", m_bufferSize, 
      m_listhead.index);

  m_listhead.nMsgs  = this->size();
  m_listhead.length = m_bufferSize;

  // Copy List header
  memcpy(m_buffer, &m_listhead, sizeof(m_listhead));

  bufPos += sizeof(m_listhead)/sizeof(UINT32);

  //Debug
  // for(unsigned int j=0; j<bufPos; j++){
  //   std::cout << "Header copy: " << m_buffer[j] << std::endl;
  // }

  long primLength;

  bufPosSave = bufPos;
  for (RodPrimList::iterator i = this->begin(); i != this->end(); i++) {
    primLength = i->copyToBuffer(&m_buffer[bufPos]);
    bufPos += primLength;
  } 
  //  for(int j=bufPosSave; j<bufPos; j++){
  //    std::cout << "Prim copy: " << m_buffer[j] << std::endl;
  //  }

  m_listtail.length = m_bufferSize;
  m_listtail.crc    = checkSum();

  // Copy List trailer
  memcpy(&m_buffer[bufPos], &m_listtail, sizeof(m_listtail));
  bufPos += sizeof(m_listtail)/sizeof(UINT32);

}

void RodPrimList::writeToXml(std::string& xmlFile) throw(PrimListException &) {
  long numPrims, primLength, bufPos;
  unsigned long i;
  long j;
  std::string currentLine;
  std::ofstream fout;
  
// Open output file
  fout.open(xmlFile.c_str(), std::ios::out);
  if (!fout.is_open()) throw PrimListException("Unable to open XML file for output ", 0, 0);
  fout << "<?xml version=\"1.0\"?>" << std::endl;
  fout << "<!DOCTYPE PRIMLIST SYSTEM \"PrimList.dtd\">" << std::endl;
  fout << "<?xml-stylesheet type=\"text/css\" href=\"PrimList.css\"?>" << std::endl;
  fout << "<PRIMLIST>" << std::endl;
  fout << "  <LISTSIZE> " << m_bufferSize << "  </LISTSIZE>" << std::endl;
  fout << "  <LISTINDEX> " << m_listhead.index << "  </LISTINDEX>" << std::endl;
  numPrims = this->size();
  fout << "  <NUMPRIMS> " << numPrims << "  </NUMPRIMS>" << std::endl;
  //  fout << "  <LISTVERSION> " << m_version << "  </LISTVERSION>" << std::endl;
  bufPos += sizeof(m_listhead)/4;
  for (i=0; i<m_buffer[2]; i++) {
    fout << "  <PRIMITIVE>" << std::endl;
    primLength = m_buffer[bufPos++];
    fout << "    <PRIMLENGTH>  " << primLength << "  </PRIMLENGTH>" << std::endl;
    fout << "    <PRIMINDEX>  " << m_buffer[bufPos++] << "  </PRIMINDEX>" << std::endl;
    fout << "    <PRIMNAME>  " << "undefined" << "  </PRIMNAME>" << std::endl;
    fout << "    <PRIMID>  " << m_buffer[bufPos++] << "  </PRIMID>" << std::endl;
    fout << "    <PRIMVERSION>  " << m_buffer[bufPos++] << "  </PRIMVERSION>" << std::endl;
    fout << "    <PRIMDATA TYPE=""HEX"">" << std::endl;
    for (j=0; j<primLength-4; j++) {
      fout << "      " << std::hex << m_buffer[bufPos++] << std::endl;
    }
    fout << "    </PRIMDATA>" << std::endl;
    fout << "  </PRIMITIVE>" << std::endl;
  }
  fout << "</PRIMLIST>" << std::endl;
}

void RodPrimList::buildFromXml(std::string& xmlFile) throw(PrimListException &) {
  //long bufPos = 0;
  long index, numPrims, primLength;
  std::string currentLine;
  std::ifstream fin;
  
// Open input file, test that first line is XML tag and throw away stylesheet line
  fin.open(xmlFile.c_str());
  if (!fin.is_open()) throw PrimListException("Unable to open XML file for input.", 0, 0);
  getline(fin, currentLine);
  if (currentLine.find("<?xml version=")==std::string::npos) throw PrimListException(
          "First line of file does not have XML tag", 0, 0);
  getline(fin, currentLine);
  if (currentLine.find("<?xml-stylesheet")<std::string::npos) {
    fin.ignore();
    getline(fin, currentLine);
  }
  
// read buffer size and create buffer
  if (currentLine.find("PRIMLIST")==std::string::npos) throw
              PrimListException("PrimList not found.", 0, 0);
  getline(fin, currentLine, '>');
  if (currentLine.find("LISTSIZE")==std::string::npos) throw PrimListException("PrimList size not found.", 0, 0);
  fin >> m_bufferSize;
  m_buffer = new unsigned long[m_bufferSize];
  m_buffer[0] = m_bufferSize;
  fin.ignore(256, '\n');           // Ignore the rest of the line
  index = 1;
  
// Get rest of the PrimList header values
  getline(fin, currentLine, '>');
  if (currentLine.find("LISTINDEX")==std::string::npos) throw PrimListException("PrimList index not found.", 0, 0);
  fin >> m_listhead.index;
  m_buffer[index++] = m_listhead.index;
  fin.ignore(256, '\n');           // Ignore the rest of the line
  getline(fin, currentLine, '>');
  if (currentLine.find("NUMPRIMS")==std::string::npos) throw PrimListException("PrimList index not found.", 0, 0);
  fin >> numPrims;
  m_buffer[index++] = numPrims;
  fin.ignore(256, '\n');           // Ignore the rest of the line
  getline(fin, currentLine, '>');
  if (currentLine.find("LISTVERSION")==std::string::npos) throw
              PrimListException("PrimList version not found.", 0, 0);
  fin.ignore(256, '\n');           // Ignore the rest of the line
  
// Loop over primitives
  for (int iprim=0; iprim<numPrims; iprim++) {
    getline(fin, currentLine, '>');
    if (currentLine.find("PRIMITIVE")==std::string::npos) throw
              PrimListException("Primitive not found.", iprim, 0);
    getline(fin, currentLine, '>');
    if (currentLine.find("PRIMLENGTH")==std::string::npos) throw
              PrimListException("Primitive length not found.", 0, 0);
    fin >> primLength;
    m_buffer[index++] = primLength;
    fin.ignore(256, '\n');           // Ignore the rest of the line
    getline(fin, currentLine, '>');
    if (currentLine.find("PRIMINDEX")==std::string::npos) throw
              PrimListException("Primitive index not found.", 0, 0);
    fin >> m_buffer[index++];
    fin.ignore(256, '\n');           // Ignore the rest of the line
    getline(fin, currentLine, '>');
    if (currentLine.find("PRIMNAME")<std::string::npos) fin.ignore(256,'\n'); //Not used
    getline(fin, currentLine, '>');
    if (currentLine.find("PRIMID")==std::string::npos) throw
              PrimListException("Primitive ID not found.", 0, 0);
    fin >> m_buffer[index++];
    fin.ignore(256, '\n');           // Ignore the rest of the line
    getline(fin, currentLine, '>');
    if (currentLine.find("PRIMVERSION")==std::string::npos) throw
              PrimListException("Primitive version not found.", 0, 0);
    fin >> m_buffer[index++];
    fin.ignore(256, '\n');           // Ignore the rest of the line
    getline(fin, currentLine, '>');
    if (currentLine.find("REPLYLENGTH")<std::string::npos) fin.ignore(256, '\n');//Not used
    getline(fin, currentLine);
    if (currentLine.find("PRIMDATA")==std::string::npos) throw
              PrimListException("Primitive data not found.", 0, 0);
// Loop over data values, filling them in buffer
    long indata;
    if (currentLine.find("DEC") < std::string::npos) {
      for (int idata=0; idata< primLength-4; idata++) {
        fin >> indata;
        m_buffer[index++] = indata;
        dec(fin); 
      }
    }
    else {
      for (int idata=0; idata< primLength-4; idata++) {
        fin >> std::hex >> indata;
        m_buffer[index++] = indata;
        dec(fin); 
      }     
    }
    getline(fin, currentLine, '>');  
    if (currentLine.find("/PRIMDATA")==std::string::npos) throw
              PrimListException("Primitive data close not found.", 0, 0);
    fin.ignore(256, '\n');           // Ignore the rest of the line
    getline(fin, currentLine, '>');  
    if (currentLine.find("/PRIMITIVE")==std::string::npos) throw
              PrimListException("Primitive close not found.", 0, 0);
    fin.ignore(256, '\n');           // Ignore the rest of the line
  }      // End of loop over primitives
  
// Add list trailer
  m_buffer[index++] = m_bufferSize;
  m_buffer[index] = checkSum();
  
  fin.close();
}

void RodPrimList::clear() {
  this->erase(begin(), end());
  m_bufferSize=0;
  if (m_buffer)
    delete [] m_buffer;
    m_buffer = 0;
}

void RodPrimList::print() {
  if (!m_buffer)
    std::cerr << "Please call bufferBuild before printing.\n";
  for (unsigned int i=0; i<m_bufferSize; i++) 
    std::cout << m_buffer[i] << std::endl;
}
} // End namespace SctPixelRod

/* This overloaded operator lets us use cout to print the PrimListException information
*/
  std::ostream& operator<<(std::ostream& os, SctPixelRod::PrimListException& primEx) {
   return primEx.what(os, primEx);
  }

