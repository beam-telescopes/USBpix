/////////////////////////////////////////////////////////////////////
// Bits.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 24/03/03  Version 1.0 (PM)
//           Initial release
//

//! Bits class store and manage bit sequences

#ifndef _PIXLIB_BITS
#define _PIXLIB_BITS

#include <vector>
#include <string>

namespace PixLib {

class Bits {
public:
  Bits();                     //! Default contructor	
  Bits(const Bits& b);        //! Copy constructor
  Bits(const Bits* b);        //! Constructor form a pointer
  Bits(int n, int d);         //! Contruct from an int
  Bits(int n);                //! Contruct and put to 0 n bits
  Bits(std::string d);        //! Construct form a string

  ~Bits(){};                  //! Destructor
	
  void clear();                         //! Set all the bits to zero
  void resize(int n);                   //! Change the dimension to n bits
  bool divide(int nd);                  //! Divide frequency by nd

  Bits operator + (const Bits& b) const;         //! Add operator
  Bits& operator += (const Bits& b);             //! Increment operator
  Bits operator | (const Bits &b) const;         //! Or operator
  Bits& operator |= (const Bits &b);             //! Self-Or operator
  Bits operator & (const Bits &b) const;         //! And operator
  Bits& operator &= (const Bits &b);             //! Self-And operator
  Bits& operator ~ ();                           //! Complement operator

  Bits& operator = (const Bits& b);              //! Assignement

  bool operator==(const Bits &b) const;    //! Comparison
  bool operator!=(const Bits &b) const;    //! Comparison

  int size() const;                           //! Return the number of bits
  Bits range(int start) const;                //! Extract a subset of bits
  Bits range(int start, int end) const;       //! Extract a subset of bits
#ifdef WIN32
  bool& operator[](int i);       //! Access to bit i
#else
  std::_Bit_reference operator[](int i);       //! Access to bit i
#endif
  unsigned int pack(int i);                   //! returns 32 bits packed in one int
  int  get(int start, int end) const;         //! Extract a subset of bits (max 32) to an int
  void set(int i, bool val);

  std::ostream& dump(std::ostream& output_stream) const;               //! Dump to ostream
  void dump(std::string& output_stream) const;                         //! Dump to String
  void dump(std::string& output_stream, int n, std::string fmt) const; //! Dump to String wrap every n chars

private:
  int nbits;               //! number of stored bits 
  std::vector<bool> bits;  //! array of int with stored bits	
};

 std::ostream& operator<< (std::ostream& s, const PixLib::Bits& b);
}


#endif
