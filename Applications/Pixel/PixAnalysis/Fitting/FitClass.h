#ifndef FITCLASS_H
#define FITCLASS_H

#include <string>
#include <map>
#include <vector>

/*! This class provides all information about functions used for fitting 
 *  Pixel DAQ data such as S-curves, ToT calibration or Gauss as well as 
 *  an actual routine running a fit on arays of x,y values and errors
 * 
 * @authors Joern Grosse-Knetter <joern.grosse-knetter@uni-bonn.de>
 */

class FitClass {

 public:
  FitClass();
  ~FitClass(){};

  /*! returns the number of parameters associated with fit function of "funcID"
   */
  int getNPar(int funcID);
  /*! returns true if the function expects four extra parameters to be set
   *  to the VCAL -> charge conversion factors
   */
  bool extraVcalPar();
  /*! returns the brief name associated with fit function of "funcID"
   */
  std::string getFuncName(int funcID){return m_funcNames[funcID];};
  /*! returns the exntensive name associated with fit function of "funcID"
   */
  std::string getFuncTitle(int funcID){return m_funcFullNames[funcID];};
  /*! returns the ID of the functions with title as returned by
      getFuncTitle or -1 if fucntion sis not ofund
  */
  int getFuncID(std::string func_title);
  /*! returns a vector with the names of all paramerst (not VCAL) associated 
   *  with fit function of "funcID"
   */
  std::vector<std::string> getParNames(int funcID);
  /*! returns true, if the last four parameters of this function are supposed to be
   *  VCAL->charge(e) conversion factors, like e.g. for S-curve fits
   */
  bool hasVCAL(int funcID);
  /*! tell the fitting method: minuit (false) or Virzi (true)
   */
  int runVirzi(int);
  /*! returns a pointer to the actual function with "funcID"
   *  can e.g. be used for creating a ROOT TF1 object when pointer
   *  is converted to (double (*)(double *, double *))
   */
  void* getFunc(int funcID);
  /*! main fitting routine    <br>
   *  NB: this is currently still based in ROOT's TGraphErrors::Fit()
   *      function which must be changed in the future
   */
  double runFit(int npts, double *x, double *y, double *xerr, double *yerr, double *par, bool *fix_par, int funcID, double xmin, double xmax, bool show=false);
  // helper function for runFit
  double getChi2(int npts, double *x, double *y, double *yerr, double *par, int funcID);

  bool guessPar(int funcID);
  bool guessPar(int funcID, double *x, double *y, int npts, double *par);
  void guesstot(double *x, double *y, int npts, double *par);

  // static functions - explicit declaration of fit functions
  // use getFunc(int) to get access
  static double erfcfun(double *, double *);
  static double newtotfun(double *, double *);
  static double totfun(double *, double *);
  static double constfun(double *, double *);
  static double linfun(double *, double *);
  static double parbfun(double *, double *);
  static double cubfun(double *, double *);
  static double expfun(double *, double *);
  static double cplexpfun(double *, double *);
  static double dblexpfun(double *, double *);
  static double gausfun(double *, double *);
  static double boxfun(double *, double *);
  static double TOT_FE_I4_pol2(double *, double *);

 private:
  std::map<int,std::string> m_funcNames, m_funcFullNames;
  std::map<int,std::vector<std::string> > m_parNames;

};

#endif // FITCLASS_H
