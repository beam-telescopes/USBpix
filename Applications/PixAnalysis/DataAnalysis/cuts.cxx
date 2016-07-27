
#include <vector>
#include <qlistview.h>
#include <qstring.h>

#include <TMath.h>

#include "cuts.h"
#include <iostream>
#include <sstream>

//using namespace std; 


Cut::Cut(const char *cutname,const  char *min,const  char *max,const  char *Test,const char *Histo,const char *action, const char *pixel)
{
  m_cutname = cutname;
  m_testtype = Test;
  m_histotype = Histo;
  m_actiontype = action;
  m_pixeltype =pixel;

  if( sscanf(min,"%lf",&m_min) == 0)//if text(1) is not a number -> Cut is undefined
    m_min= UNDEFMIN;
  //if(TMath::Abs(m_min)<1e-35) 
  // m_min=0;
  
  if( sscanf(max,"%lf",&m_max) == 0)//if text(1) is not a number -> Cut is undefined
    m_max= UNDEFMAX;
  if(TMath::Abs(m_max)<1e-35) 
    m_max=0;
}
 

Cut::Cut(const char *cutname, double min, double max,const  char *Test,const char *Histo,const char *action, const char *pixel)
{
  m_cutname = cutname;
  m_min = min;
  m_max = max;
  m_testtype = Test;
  m_histotype = Histo;
  m_actiontype = action;
  m_pixeltype =pixel;
}


Cut::~Cut(){}


void Cut::setMin(const char *min)
{
  if( sscanf(min,"%lf",&m_min) == 0)//if text(1) is not a number -> Cut is undefined
    m_min= UNDEFMIN;
  // if(TMath::Abs(m_min)<1e-35) 
  // m_min=0;
}


void Cut::setMax(const char *max)
{
 if( sscanf(max,"%lf",&m_max) == 0)//if text(1) is not a number -> Cut is undefined
    m_max= UNDEFMAX;
 if(TMath::Abs(m_max)<1e-35) 
    m_max=0;
}


BadPixCut::BadPixCut(const char *cutname,const char *min,const char *max,const char *Test,const char *Histo,const char *action, const char *pixel,const char *badmin,const char *badmax)
  :Cut(cutname,min,max,Test,Histo,action,pixel) 
{
  if( sscanf(badmin,"%lf",&m_badpixmin ) == 0)
    m_badpixmin= UNDEFMIN;

  if( sscanf(badmax,"%lf",&m_badpixmax ) == 0)
    m_badpixmax= UNDEFMAX;
}

BadPixCut::BadPixCut(const char *cutname, double min, double max,const char *Test,const char *Histo,const char *action, const char *pixel, double badmin, double badmax)
:Cut(cutname,min,max,Test,Histo,action,pixel) 
{
  m_badpixmin = badmin;
  m_badpixmax = badmax;
}


void BadPixCut::setBadPixMin(const char *min)
{
  if( sscanf(min,"%lf",&m_badpixmin) == 0)
    m_badpixmin= UNDEFMIN; 
}


void BadPixCut::setBadPixMax(const char *max)
{
  if( sscanf(max,"%lf",&m_badpixmax) == 0)
    m_badpixmax= UNDEFMAX;
}
