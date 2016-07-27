/////////////////////////////////////////////////////////////////////
// ConfMask.h 
// version 0.1
/////////////////////////////////////////////////////////////////////
//
// 13/04/04  Version 0.1 (CS)
//

#ifndef PIXLIB_CONFMASK
#define PIXLIB_CONFMASK

#include <vector>

namespace PixLib {

template<class T> class ConfMask {
public:
  ConfMask() : m_maxValue(0) {} // Default constructor
  ConfMask(int nCol, int nRow, T maxValue); // Constructor (all entries are set to zero)
  ConfMask(int nCol, int nRow, T maxValue, T defValue); // Constructor (all entries are set to defValue)
  ConfMask(const ConfMask& c); //! Copy constructor
  ~ConfMask(); // Destructor

  //! Assignment operator
  ConfMask& operator = (const ConfMask& c);

  //! Entire mask operations
  void enableAll();       // Set all entries to m_maxValue
  void disableAll();      // Set all entries to zero
  void setAll(T value); // Set all entries to value
  
  //! Single column operations
  void enableCol(int col);         // Set an entire column to m_maxValue
  void disableCol(int col);        // Set an entire column to zero
  void setCol(int col, T value); // Set an entire column to value

  //! Single row operations
  void enableRow(int row);         // Set an entire row to m_maxValue
  void disableRow(int row);        // Set an entire row to zero
  void setRow(int row, T value); // Set an entire row to value

  //! Single entry operations
  void enable(int col, int row);         // Set entry to m_maxValue
  void disable(int col, int row);        // Set entry to zero
  void set(int col, int row, T value); // Set entry to value

  //! Conversion to and from vector methods
  void set(std::vector<T> &value);
  void get(std::vector<T> &output);
  int ncol() { return m_ncol; };
  int nrow() { return m_nrow; };

  //! Accessor methods
  std::vector< std::vector<T> >& get();
  std::vector<T>& operator[](int col);
  T& get(int col, int row);

private: 
  std::vector< std::vector<T> > m_mask;
  T m_maxValue;
  int m_nrow;
  int m_ncol;
};

}


#include "Config/ConfMask.cxx"


#endif

