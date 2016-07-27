/////////////////////////////////////////////////////////////////////
// Bits.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 24/03/03  Version 1.0 (PM)
//           Initial release
//

// Bits class store and manage bit sequences

#include "Bits.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <assert.h>

using namespace PixLib;

Bits::Bits() { // Default constructor
  bits.clear();
  nbits = 0;
} 

Bits::Bits(const Bits& b) : nbits(b.nbits) { // copy constructor
  bits.clear();
  if (nbits != 0){
    for(int i = 0; i < nbits; i++) bits.push_back(b.bits[i]);
  }
} 

Bits::Bits(const Bits *b) { // Construction from a pointer
  bits.clear();
  nbits = 0;
  if (b != NULL) {
    nbits = b->nbits; 
    if (nbits != 0) {
      for (int i = 0; i < nbits; i++) bits.push_back(b->bits[i]);
    }
  }
}

Bits::Bits(int n, int d){ // contruct from a array of ints
  bits.clear();
  nbits = 0;
  if (n > 0 && n <=32) {
    nbits = n;
    for (int i=0; i<n; i++) bits.push_back((d>>(n-i-1))&0x1);
  }
}

Bits::Bits(int n) : nbits(n) { // contruct and put to 0 n bits
  bits.clear();
  if (nbits != 0) {
    for (int i=0; i<n; i++) bits.push_back(false);
  }
}

Bits::Bits(std::string d)  { // contruct from a string
  int bitsFirstChar;
  unsigned int ich;
  if (d[1]=='x' || d[1] == 'X') {
    ich = 2;
    std::istringstream(d.substr(0,1)) >> std::hex >> bitsFirstChar;
    bitsFirstChar = bitsFirstChar%4;
  } else {
    ich = 0;
    bitsFirstChar = 0;
  }
  if (bitsFirstChar == 0) bitsFirstChar = 4;
  nbits = 0;
  bits.clear();
  int ichar;
  for (; ich<d.size(); ich++) {
    int ib = 3; 
    if (nbits == 0) ib = bitsFirstChar - 1;
    std::istringstream(d.substr(ich,1)) >> std::hex >> ichar;
    for (; ib>=0; ib--) {
      bits.push_back((ichar >> ib) & 0x1);
      nbits++;
    }
  }
}

void Bits::clear() {   // Set all the bits to zero
  if (nbits > 0) {
    bits.clear();
    for (int i=0;i<nbits;i++) bits.push_back(false);
  }
}

void Bits::resize(int n) {   // Change the dimension to n bits
  int i;
  if (n == nbits) return;
  if (n > nbits) {
    for (i=nbits; i<n; i++) bits.push_back(false);
    nbits = n;
  } else if (n > 0){
    std::vector<bool> old = bits;
    bits.clear();
    nbits = n;
    for (int i=0; i<nbits; i++) bits[i] = old[i];
  } else {
    nbits = 0;
    bits.clear();
  }
}

bool Bits::divide(int nd) { 
  assert (nd > 0);

  std::vector<bool> divided;
  int i=0;
  while (i < nbits) {
    bool bit = bits[i++];
    divided.push_back(bit);
    for (int j=1; j<nd; j++) {
      if (i<nbits) {
        if (bits[i] != bit) return false; 
      }
      i++;
    }
  }
  nbits = divided.size();
  bits = divided;
  return true;
}

Bits Bits::operator + (const Bits& b) const { // Sum operator
  Bits ret = *this;
  for (int i=0; i<b.nbits; i++) ret.bits.push_back(b.bits[i]);
  ret.nbits += b.nbits;
  return ret;
}

Bits& Bits::operator += (const Bits& b) { // Increment operator
  for (int i=0; i<b.nbits; i++) bits.push_back(b.bits[i]);
  nbits += b.nbits;
  return *this;
}

Bits Bits::operator | (const Bits& b) const { // Or
  Bits ret;
  ret.nbits = nbits;
  if (b.nbits > ret.nbits) ret.nbits = b.nbits;
  for (int i=0; i<ret.nbits; i++) {
    bool b1 = false, b2 = false;
    if (i < nbits) b1 = bits[i];
    if (i < b.nbits) b2 = b.bits[i];
    ret.bits.push_back(b1|b2);
  }
  return ret;
}

Bits& Bits::operator |= (const Bits& b) { // Self or
  if (b.nbits > nbits) resize(b.nbits);
  for (int i=0; i<b.nbits; i++) {
    bits[i] = bits[i] | b.bits[i];
  }
  return *this;
}

Bits Bits::operator & (const Bits& b) const { // And
  Bits ret;
  ret.nbits = nbits;
  if (b.nbits > ret.nbits) ret.nbits = b.nbits;
  for (int i=0; i<ret.nbits; i++) {
    bool b1 = false, b2 = false;
    if (i < nbits) b1 = bits[i];
    if (i < b.nbits) b2 = b.bits[i];
    ret.bits.push_back(b1&b2);
  }
  return ret;
}

Bits& Bits::operator &= (const Bits& b) { // Self and
  if (b.nbits > nbits) resize(b.nbits);
  for (int i=0; i<b.nbits; i++) {
    bits[i] = bits[i] & b.bits[i];
  }
  return *this;
}

Bits& Bits::operator ~ () { // Complement
  for (int i=0; i<nbits; i++) bits[i] = !bits[i];
  return *this;
}

Bits& Bits::operator = (const Bits& b) { // assignement
  if (this != &b) {
    nbits = b.nbits;
    bits.clear();
    for (int i=0; i<nbits; i++) bits.push_back(b.bits[i]);
  }
  return *this;
}

bool Bits::operator==(const Bits &b) const { // Comparison
  if (nbits != b.nbits) return false;
  for (int i=0; i<nbits; i++) {
    if (bits[i] != b.bits[i]) return false;
  }
  return true;
}

bool Bits::operator!=(const Bits &b) const { // Comparison
  return !(b==*this);
}

int Bits::size() const {
  return nbits;
}

Bits Bits::range(int start) const { // Extract a sub set of bits
  return range(start, nbits-1);
}

Bits Bits::range(int start, int end) const { // Extract a sub set of bits
  assert(start>= 0 && start < nbits);
  assert(end>= 0 && end < nbits);
  assert(start <= end);

  Bits temp;
  if (start>= 0 && start < nbits) return temp;
  if (end>= 0 && end < nbits) return temp;
  if (start <= end) return temp;

  temp.nbits = end - start + 1;
  for (int ib=start; ib<=end; ib++) {
    temp.bits.push_back(bits[ib]);
  }
  return temp;
}

#ifdef WIN32
bool& Bits::operator[](int i) { //! Access to bit i
  static bool tmpret;
  if (i>=0 && i<nbits) {
    tmpret = bits[i];
    return tmpret;
#else
std::_Bit_reference Bits::operator[](int i) { //! Access to bit i
  if (i>=0 && i<nbits) {
    return bits[i];
#endif
  } else {
    throw ("an exception");
  }
}

 void Bits::set(int i, bool val){
  if (i>=0 && i<nbits) {
    bits[i] = val;
  } else {
    throw ("an exception");
  }
}
 
unsigned int Bits::pack(int i){ //! returns 32 bits packed in one int
  int npack = nbits/32;
  if (npack*32 != nbits) npack++;
  if (i>=0 && i<npack) {
    unsigned int out = 0;
    for (int j=0; j<32; j++) {
      if (i*32+j<nbits) {
        if (bits[i*32+j]) out |= (0x1 << (32-j-1));
      } else {
        break;
      }
    }
    return out;
  } else {
    return 0;
  }
}

int Bits::get(int start, int end) const { // Extract a sub set of bits
  assert(start>= 0 && start < nbits);
  assert(end>= 0 && end < nbits);
  assert(start <= end);
  assert(end - start + 1 < 32);

  int temp=0;
  for (int ib=start; ib<=end; ib++) {
    if (bits[ib]) temp |= (0x1 << (end-ib));
  }
  return temp;
}

std::ostream& Bits::dump(std::ostream& output_stream) const {

  int pad = nbits%4 - 1;
  if (pad < 0) pad = 3;
  int hexDigit = 0;

  output_stream << nbits%4 << "x";
  for (int ib=0; ib<nbits; ib++) {
    if (bits[ib]) hexDigit |= (0x1 << pad);
    if (--pad < 0) {
      output_stream << std::hex << std::setw(1) << hexDigit; 
      hexDigit = 0;
      pad = 3;
    }
  }
  output_stream << std::dec;
  return output_stream;   
}

void Bits::dump(std::string& output_stream) const {
  dump(output_stream, 0, "");
}

void Bits::dump(std::string& output_stream, int n, std::string fmt) const {
  int ib,ip1,ip2,dat,pad;
  int count;

  pad = nbits%4 - 1;
  if (pad < 0) pad = 3;
  dat = 0;

  std::ostringstream sout(output_stream);  
  sout << std::hex << std::setw(1) << nbits%4 << "x";
  count = 2;
  for (ib=0; ib<nbits; ib++) {
    ip1 = ib/32; // int 
    ip2 = 31 - ib%32; // modulus
    dat |= ((bits[ip1] >> ip2) & 0x1) << pad--;
    if (pad < 0) {
      sout << std::setw(1) << dat; count++;
      if (count == n) {
        count = 0;
        sout << fmt;
      }
      pad = 3;
      dat = 0;
    }
  }
}

std::ostream& PixLib::operator<< (std::ostream& s, const Bits& b)
{
	b.dump(s);
	return s;
}

