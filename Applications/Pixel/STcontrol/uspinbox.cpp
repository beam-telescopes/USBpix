/***************************************************************************
                          kuintspinbox.cpp  -  description
                             -------------------
    begin                : Sun Apr 13 2003
    copyright            : (C) 2003 by German Martinez
    email                : german.martinez@cern.ch
 ***************************************************************************/
//#include <config.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <math.h>

#include "uspinbox.h"
#include <QString>
#include <QValidator>

 USpinBox::USpinBox(QWidget *parent)
 : QSpinBox (parent)
 {
   m_valBase = 16;
   setMinimum(-2147483648);
   setMaximum(2147483647);
   setSingleStep(4);
   setUValue(0);
 }

 USpinBox::~USpinBox()
 {
 }

 USpinBox::USpinBox(unsigned long lower, unsigned long upper, int step, unsigned long value, int base,
                          QWidget* parent)
     : QSpinBox (parent)
 {
   m_valBase = base;
   setMinimum(UtoI(lower));
   setMaximum(UtoI(upper));
   setSingleStep(step);
   setUValue(value);
 }

 void USpinBox::setBase(int base)
 {
     m_valBase = base;
 }
 
 
 int USpinBox::base() const
 {
     return m_valBase;
 }
 
 
 // -----------------------------------------------------------------------------

 QString  USpinBox::textFromValue(int val) const
 {
   return QString::number(ItoU(val), m_valBase);
 }


 // -----------------------------------------------------------------------------

 int USpinBox::valueFromText(const QString &text) const
{
  bool ok;
  int retVal= UtoI(text.toUInt(&ok, m_valBase));
  if(ok)
    return retVal;
  else
    return 0;
}

unsigned long USpinBox::UValue()
{
   m_val=ItoU(value());
   return m_val;
}

void USpinBox::setUValue(unsigned long v)
{
  m_val=v;
  setValue(UtoI(m_val));
}

QValidator::State USpinBox::validate ( QString & input, int &) const
{
  bool ok;
  input.toUInt(&ok, m_valBase);
  return  ok?QValidator::Acceptable:QValidator::Invalid;
}
