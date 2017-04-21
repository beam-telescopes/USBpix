#ifndef DACUTPANEL_H
#define DACUTPANEL_H

#include <qvariant.h>
#include <qwidget.h>
#include <string.h>
//#include <TH2.h>
#include "cutpanelbase.h"
#include "Module.h"
#include <PixConfDBInterface.h>
#include "CutItem.h"



class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QPushButton;
class QSpinBox;
class QFrame;
class QLineEdit;
class QListView;
class QListViewItem;
class QString;
class TH2F;
class TH1F;
class Moditem;
class QListBox;
class QMenuBar;
class CutItem;
class DAEngine;

// Hauptwindow zur Kontrolle der Analyse

class cutPanel : public cutPanelBase  
{
    Q_OBJECT

public:
    cutPanel( DAEngine *eng, QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~cutPanel();
    
    void getCuts(); //load cuts from engine


public slots:
  void newCut();
  void accept_cuts();
  void deletecut();
  void editcut(QListViewItem *lvitem);
  void loadCuts();

private slots:
  void saveCuts(); //speichern von cuts


private:
  QMenuBar *m_menu;
  DAEngine *m_engine;

 
};

#endif
