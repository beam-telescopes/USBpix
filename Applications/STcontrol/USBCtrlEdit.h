#ifndef USBCTRLEDIT_H
#define USBCTRLEDIT_H
#include "ui_USBCtrlEdit.h"

namespace PixLib{
 class Config;
 class ConfGroup;
}

class USBCtrlEdit : public QDialog, public Ui::USBCtrlEdit {

 Q_OBJECT

 public:
  USBCtrlEdit(PixLib::Config &in_cfg, QWidget* parent = 0, Qt::WindowFlags fl=0);
  ~USBCtrlEdit();

  bool recfgNeeded(){return m_recfgNeeded;};

  public slots:
   void save();
   void browseFPGA();
   void browseMicroCtrl();
   void calcAuxFreq(int);
   void AdapterCardFlavourComboBoxStateChanged();

 private:
  PixLib::Config &m_config;
  bool m_recfgNeeded;
  bool m_overrideEnableDemux;

  bool check_setup();

  void updateReadoutChannelComboboxes(); 
};

#endif // USBCTRLEDIT
