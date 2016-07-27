/////////////////////////////////////////////////////////////////////
// Histo.h 
// version 1.0
/////////////////////////////////////////////////////////////////////
//
//  5/04/04  Version 1.0 (PM)
//

#ifndef PIXLIB_HISTO
#define PIXLIB_HISTO

#include <string>

namespace PixLib {

class Histo {
public:
  Histo();                                                                                    //! Default constructor ("0-Dim histogram")
  Histo(std::string name, std::string title, unsigned int nbin, double xmin, double xmax);    //! Constructor 1D
  Histo(std::string name, std::string title, unsigned int nbin1, double xmin1, double xmax1,
                                             unsigned int nbin2, double xmin2, double xmax2); //! Constructor 2D
  Histo(std::string name, std::string title, unsigned int size, unsigned int nbin1, double xmin1, double xmax1,
                                             unsigned int nbin2, double xmin2, double xmax2); //! Constructor 2D
  Histo(const Histo &h);                                                                      //! Copy contructor
  ~Histo();                                                                                   //! Dstructor

  Histo &operator=(const Histo &h);                     //! Assignement operator
  double operator()(unsigned int i) const ;                    //! Bin read access 1d
  double operator()(unsigned int i, unsigned int j) const;    //! Bin read access 2d
  void set(unsigned int i, double val);                 //! Bin write access 1d
  void set(unsigned int i, unsigned int j, double val); //! Bin write access 2d
  void clear();                                         //! Reset histo - NO MEMORY DISCARD 
//  void reset();                                         //! 

  //! Accessors
  std::string name() const { return m_name; };
  std::string title() const { return m_title; };
  int nDim() const { return m_ndim; };
  int nBin(int d) const { if (d >=0 && d <= 1) return m_dim[d]; else return 0; };
  double min(int d) const { if (d >=0 && d <= 1) return m_lim[d][0]; else return 0; };
  double max(int d) const { if (d >=0 && d <= 1) return m_lim[d][1]; else return 0; };

private:
  int m_ndim;             //! Number of dimensions (1 or 2)
  std::string m_name;     //! Histogram name
  std::string m_title;    //! Histogram title
  unsigned int  m_dim[2]; //! Dimensions
  double m_lim[2][2];     //! Limits
  void** m_histo;         //! Histogram
  unsigned int m_size;    //! Word size
  double m_trash;         //! Trash bin
};

}

#endif

