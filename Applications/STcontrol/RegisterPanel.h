#ifndef REGISTERPANEL
#define REGISTERPANEL

#include "ui_RegisterPanel.h"
#include "STCdefines.h"
#include <QWidget>
#include <QLabel>
#include <vector>


class QHBoxLayout; 
class QLabel;
class QPushButton;
class QAction;

class STControlEngine;
namespace PixLib{
class PixModule;
}

class RegisterItem : public QWidget{
  Q_OBJECT

 public:
  RegisterItem( QWidget* parent = 0, QString name="", Qt::WindowFlags fl = 0);
  ~RegisterItem();
  QHBoxLayout* m_layout;

 public slots:
  void detailsPressed();
  void setStatus(StatusTag status=tunknown, QString infoText="");
  void hide();
  void show();

 private:
  QLabel* m_label;
  QPushButton* m_button;
  QString m_infoText;

};

class RegisterPanel : public QWidget, public Ui::RegisterPanel{

  Q_OBJECT

 public:
  RegisterPanel( STControlEngine &engine_in, QWidget* parent = 0, Qt::WindowFlags fl = 0);
  ~RegisterPanel();

 public slots:
  void clearInfo();
  void clearAll();
  void testAll();
  void testCurrent();
  void testList0(){testList(0, false);};
  void testList1(){testList(1, false);};
  void testList2(){testList(2, false);};
  void testListCurrent0(){testList(0, true);};
  void testListCurrent1(){testList(1, true);};
  void testListCurrent2(){testList(2, true);};
  void testList(int, bool);
  void setModule(int crateID, int grpID, int modID, const char* name);
  void setModule(int crateID, int grpID, int modID ){setModule(crateID, grpID, modID, 0);};
  void setTextMessg(const char *msg);
  void setActions(bool);
  void setActionsEnabled(){setActions(true);};
  void setActionsDisabled(){setActions(false);};
  void setCurrModBlock(bool block){m_unblocked = !block;};

  static void setLED(QLabel *label, int state);

 private:
  STControlEngine &m_engine;
  RegisterItem *m_regMod;
  RegisterItem *m_regMCC;
  std::vector<RegisterItem*> m_regFE;
  bool m_unblocked;
  int m_crateID, m_grpID, m_modID;
  std::vector<QAction*> m_currActions, m_allActions;

};

#endif // REGISTERPANEL
