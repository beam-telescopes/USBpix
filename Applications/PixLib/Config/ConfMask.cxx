/////////////////////////////////////////////////////////////////////
// ConfMask.cxx 
// version 0.1
/////////////////////////////////////////////////////////////////////
//
// 13/04/04  Version 0.1 (CS)
//

#include "Config/ConfMask.h"

using namespace PixLib;


//! Constructor (all entries are set to zero)
template<class T> ConfMask<T>::ConfMask(int nCol, int nRow, T maxValue) : m_maxValue(maxValue) {
  m_ncol = nCol;
  m_nrow = nRow;
  // Check column and row numbers
  if(nCol>0 && nRow>0) {
    // Create mask
    m_mask = std::vector< std::vector<T> >(nCol, std::vector<T>(nRow, 0));
  }
}

//! Constructor (all entries are set to defValue)
template<class T> ConfMask<T>::ConfMask(int nCol, int nRow, T maxValue, T defValue) : m_maxValue(maxValue) {
  m_ncol = nCol;
  m_nrow = nRow;
  // Check column and row numbers
  if(nCol>0 && nRow>0) {
    // Create mask
    T defaultValue;
    if(defValue<m_maxValue) defaultValue = defValue; else defaultValue = m_maxValue;
    m_mask = std::vector< std::vector<T> >(nCol, std::vector<T>(nRow, defaultValue));
  }
}

//! Copy constructor
template<class T> ConfMask<T>::ConfMask(const ConfMask<T> &c) {
  // Copy mask
  m_mask = c.m_mask;

  // Copy max value
  m_maxValue = c.m_maxValue;
  m_ncol = c.m_ncol;
  m_nrow = c.m_nrow;
}

//! Destructor
template<class T> ConfMask<T>::~ConfMask() {
  // Clear columns
  for(unsigned int col=0; col<m_mask.size(); col++) m_mask[col].clear();

  // Clear mask
  m_mask.clear();
}


//! Assignment operator
template<class T> ConfMask<T>& ConfMask<T>::operator = (const ConfMask<T>& c) {
  // Copy mask
  m_mask = c.m_mask;

  // Copy max value
  m_maxValue = c.m_maxValue;
  m_ncol = c.m_ncol;
  m_nrow = c.m_nrow;

  return *this;
}


//! Set all entries to m_maxValue
template<class T> void ConfMask<T>::enableAll() {
  // Enable mask
  m_mask = std::vector< std::vector<T> >(m_mask.size(), std::vector<T>(m_mask.front().size(), m_maxValue));
}

//! Set all entries to zero
template<class T> void ConfMask<T>::disableAll() {
  // Disable mask
  m_mask = std::vector< std::vector<T> >(m_mask.size(), std::vector<T>(m_mask.front().size(), 0));
}

//! Set all entries to value
template<class T> void ConfMask<T>::setAll(T value) {
  // Set mask
  T val;
  if(value<m_maxValue) val = value; else val = m_maxValue;
  m_mask = std::vector< std::vector<T> >(m_mask.size(), std::vector<T>(m_mask.front().size(), val));
}
  

//! Set an entire column to m_maxValue
template<class T> void ConfMask<T>::enableCol(int col) {
  // Check column number
  if(col<0 || col>=(int)m_mask.size()) return;

  // Enable column
  m_mask[col] = std::vector<T>(m_mask.front().size(), m_maxValue);
}

//! Set an entire column to zero
template<class T> void ConfMask<T>::disableCol(int col) {
  // Check column number
  if(col<0 || col>=(int)m_mask.size()) return;

  // Disable column
  m_mask[col] = std::vector<T>(m_mask.front().size(), 0);
}

//! Set an entire column to value
template<class T> void ConfMask<T>::setCol(int col, T value) {
  // Check column number
  if(col<0 || col>=(int)m_mask.size()) return;

  // Set column to value
  T val;
  if(value<m_maxValue) val = value; else val = m_maxValue;
  m_mask[col] = std::vector<T>(m_mask.front().size(), val);
}


//! Set an entire row to m_maxValue
template<class T> void ConfMask<T>::enableRow(int row) {
  // Check row number
  if(row<0 || row>=(int)m_mask.front().size()) return;

  // Enable row
  for(unsigned int i=0; i<m_mask.size(); i++)
    m_mask[i][row] = m_maxValue;
}

//! Set an entire row to zero
template<class T> void ConfMask<T>::disableRow(int row) {
  // Check row number
  if(row<0 || row>=(int)m_mask.front().size()) return;

  // Disable row
  for(unsigned int i=0; i<m_mask.size(); i++)
    m_mask[i][row] = 0;
}

//! Set an entire row to value
template<class T> void ConfMask<T>::setRow(int row, T value) {
  // Check row number
  if(row<0 || row>=(int)m_mask.front().size()) return;

  // Set row to value
  T val;
  if(value<m_maxValue) val = value; else val = m_maxValue;
  for(unsigned int i=0; i<m_mask.size(); i++)
    m_mask[i][row] = val;
}


//! Set entry to m_maxValue
template<class T> void ConfMask<T>::enable(int col, int row) {
  // Check row and column number
  if(col<0 || col>=(int)m_mask.size()) return;
  if(row<0 || row>=(int)m_mask.front().size()) return;

  // Enable entry
  m_mask[col][row] = m_maxValue;
}

//! Set entry to m_maxValue
template<class T> void ConfMask<T>::disable(int col, int row) {
  // Check row and column number
  if(col<0 || col>=(int)m_mask.size()) return;
  if(row<0 || row>=(int)m_mask.front().size()) return;

  // Disable entry
  m_mask[col][row] = 0;
}

//! Set entry to m_maxValue
template<class T> void ConfMask<T>::set(int col, int row, T value) {
  // Check row and column number
  if(col<0 || col>=(int)m_mask.size()) return;
  if(row<0 || row>=(int)m_mask.front().size()) return;

  // Set entry to value
  T val;
  if(value<m_maxValue) val = value; else val = m_maxValue;
  m_mask[col][row] = val;
}


//! Initialization from vector of T
template<class T> void ConfMask<T>::set(std::vector<T> &value) {
  // Get row and column number
  unsigned int nCol = m_mask.size();
  unsigned int nRow = m_mask.front().size();
  unsigned int col=0, row=0;

  // Set entries
  for(unsigned int i=0; i<value.size() && col<nCol && row<nRow; i++) {
    m_mask[col][row] = value[i];
    col++; if(col==nCol) {row++; col=0;}
  }
}

//! Output to vector of T
template<class T> void ConfMask<T>::get(std::vector<T> &output) {
  // Get row and column number
  unsigned int nCol = m_mask.size();
  unsigned int nRow = m_mask.front().size();

  // Get entries
  output.clear();
  for(unsigned int row=0; row<nRow; row++)
    for(unsigned int col=0; col<nCol; col++)
      output.push_back(m_mask[col][row]);
}


//! Mask accessor method
template<class T> std::vector< std::vector<T> >& ConfMask<T>::get() {
  return m_mask;
}

//! Column accessor method
template<class T> std::vector<T>& ConfMask<T>::operator[](int col) {
  return m_mask[col];
}

//! Mask accessor method
template<class T> T& ConfMask<T>::get(int col, int row) {
  return m_mask[col][row];
}


