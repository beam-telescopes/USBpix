
#include <vector>
#include <qlistview.h>
#include <qstring.h>

#include <TMath.h>

#include "CutItem.h"
#include <iostream>
#include <sstream>

using namespace std; 


CutItem::CutItem(QListView *parent, QString l1, QString l2, QString l3,QString l4,QString l5,QString l6)
  :QListViewItem(parent, l1,l2,l3,l4,l5,l6)
{

}
 
CutItem::~CutItem(){}
  
std::string CutItem::getCutName()
{
  return (text(0).latin1());
}


std::string  CutItem::getMin()
{
 //  double min;
  
//   if( sscanf(text(1).latin1(),"%lf",&min) == 0)
//     {
//       return -10000000.0;
//     }
//   if(TMath::Abs(min)<1e-35) min=0;
//   return min;
  return (text(1).latin1());
}


std::string  CutItem::getMax()
{
 //  double max;
//   if( sscanf(text(2).latin1(),"%lf",&max) == 0)
//     {
//       return 10000000.0;
//     }
//   if(TMath::Abs(max)<1e-35) max=0;
//   return max;
  return (text(2).latin1());
}


std::string CutItem::getTestType()
{
  return (text(3).latin1());
}


std::string CutItem::getHistoType()
{
  return (text(4).latin1());
}
std::string CutItem::getActType()
{
  return (text(5).latin1());
}
