#ifndef REGPANEL_H
#define REGPANEL_H

#include "ui_RegPanelBase.h"
#include <qstring.h>

class ModItem;
class QPushButton;
class QLineEdit;

class RegPanel : public QDialog, public Ui::RegPanelBase
{
    Q_OBJECT

 public:
    RegPanel( ModItem *item,  QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~RegPanel();

 public slots:
    void updateResult(int chip);
    QString bitToString(unsigned int bits, bool &allOK, int maxBits=-1);

 private:
    ModItem *m_item;
    QLineEdit *m_CPedit[9];
    QPushButton *m_CPbutton[9];

};

#endif // REGPANEL_H
