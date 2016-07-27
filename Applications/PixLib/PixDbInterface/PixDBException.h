#include "BaseException.h"
#include <iostream>
#include <string>

#ifndef _PIXDBEXCEPTION
#define _PIXDBEXCEPTION

namespace PixLib{
  
  class PixDBException : public SctPixelRod::BaseException{
  public:
    std::string message;
    virtual void what(std::ostream& os){ os << message;} // Dump the exception message to message
    // Constructors
    PixDBException(std::string txt) : BaseException(""), message(txt) {}
  };
};

#endif
