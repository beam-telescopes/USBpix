#ifndef MODSTATUS_H
#define MODSTATUS_H

#include <qvariant.h>
#include <qwidget.h>
#include <string.h>
#include <modstatusbase.h>
#include "cutpanelbase.h"
#include "Module.h"
#include <PixConfDBInterface.h>
#include "CutItem.h"





// Hauptwindow zur Kontrolle der Analyse

class ModStatus : public ModStatusBase  
{
    Q_OBJECT

public:
    ModStatus(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
    ~ModStatus(){};
 
public slots:
    virtual void getHisto(const QString &);
 
signals:
     void s_startMA(const char *,const char *, const char *);

};

#endif
