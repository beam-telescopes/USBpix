#if !defined(COLPAIRSWITCH_H)
#define COLPAIRSWITCH_H

#include "ui_ColPairSwitch.h"
#include <QPushButton>
#include <vector>

class ColourButton : public QPushButton {
 Q_OBJECT

 public:

  ColourButton(const QString &text, QWidget *parent=0);
  ~ColourButton();

 public slots:
  void setState();

};

class ColPairSwitch : public QDialog, public Ui::ColPairSwitch{

 Q_OBJECT

 public:
  ColPairSwitch(QWidget* parent = 0, Qt::WindowFlags fl = 0 , int nButtons=9);
  ~ColPairSwitch();

 public slots:
  void allOff();
  void allOn();

 public:
  std::vector<ColourButton*>& getButtons(){return m_buttons;}

 private:
  int m_nButtons;
  std::vector<ColourButton*> m_buttons;

};

#endif
