#ifndef CUT_H
#define CUT_H

#include <vector>
#include <qlistview.h>
#include <string>
//#include "cutpanel.h"
//#include "CutItem.h"

#define UNDEFMIN 1.234567e-15
#define UNDEFMAX 9.876543e15

class Cut{
 
 private:
  //member
  std::string m_cutname,
    m_testtype,
    m_histotype,
    m_actiontype,
    m_pixeltype;
  
  double m_min, m_max;


 public:
  //methoden
  Cut(const char *cutname,const char *min,const char *max,const char *Test,const char *Histo,const char *action, const char *pixel);
  Cut(const char *cutname, double min, double max,const char *Test,const char *Histo,const char *action, const char *pixel);
  ~Cut();
  
  std::string getCutName()  {return m_cutname;};
  double getMin()           {return m_min;};
  double getMax()           {return m_max;};
  // std::string getStringMin();    
  // std::string getStringMax();     
  std::string getTestType() {return m_testtype;};
  std::string getHistoType(){return m_histotype;};
  std::string getActType()  {return m_actiontype;};
  std::string getPixType()  {return m_pixeltype;};

  void setCutName(const char *n)   {m_cutname=n;};
  void setMin(double min)          {m_min = min;};
  void setMin(const char *);
  void setMax(double max)          {m_max = max;};
  void setMax(const char *);
  void setTestType(const char *n)  {m_testtype=n;};
  void setHistoType(const char *n) {m_histotype=n;};
  void setActType(const char *n)   {m_actiontype=n;};
  void setPixType(const char *n)   {m_pixeltype=n;};



};


class BadPixCut : public Cut
{
 public:
  BadPixCut(const char *cutname,const char *min,const char *max,const char *Test,const char *Histo,const char *action,const char *pixel, const char *badmin,const char *badmax);
  BadPixCut(const char *cutname, double min, double max,const char *Test,const char *Histo,const char *action,const char *pixel, double badmin, double badmax);
  ~BadPixCut(){};

  double getBadPixMin(){return m_badpixmin;};
  double getBadPixMax(){return m_badpixmax;};
  void   setBadPixMin(const char *);
  void   setBadPixMax(const char *);
  void   setBadPixMin(double min){m_badpixmin=min;};
  void   setBadPixMax(double max){m_badpixmax=max;};

 private:
  double m_badpixmin;
  double m_badpixmax;
};

#endif
