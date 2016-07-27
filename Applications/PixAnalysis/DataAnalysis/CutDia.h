#ifndef CUTDIA_H
#define CUTDIA_H

#include <qvariant.h>
#include <qwidget.h>
#include <string.h>
//#include <TH2.h>
#include "cutpanelbase.h"
#include "Module.h"
#include <PixConfDBInterface.h>
#include "cutdiabase.h"





// Hauptwindow zur Kontrolle der Analyse

class CutDia : public cutdiabase
{
    Q_OBJECT

public:
     CutDia( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CutDia();
public slots:    
    virtual bool setBadPixEnable();
    virtual bool setTotalBadPixEnable();

private:
    void enablebadpix(bool ena);
    void enablebadpixtotal(bool ena);
/* public slots: */
/*   virtual void updatecuts(QListBox *); */
/*   virtual void newCut(); */
/*   virtual void accept_cuts(); */
/*   virtual void deletecut(); */
/*   virtual void editcut(QListViewItem *lvitem); */

/* private slots: */
/*   virtual void show_only_testype(); //makes not testtype items invisible */
/*   virtual void loadCuts(); //laden von cuts */
/*   virtual void saveCuts(); //speichern von cuts */


/* private: */
/*   QMenuBar *m_menu; */

 
};

#endif
