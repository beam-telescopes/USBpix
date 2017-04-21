#ifndef SELECT_PANEL
#define SELECT_PANEL

#include <vector>
#include <qcheckbox.h>
#include "ui_SelectionPanelBase.h"

class SelectionPanel : public QDialog, public Ui::SelectionPanelBase{

    Q_OBJECT

 public:
  SelectionPanel( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
  ~SelectionPanel();
  
 public slots:
  void OKpressed();
  void CancPressed();
  void SelectAll(){SetAll(true);};
  void DeselectAll(){SetAll(false);};
  void SetAll(bool doset);
  void AddCheckBox(const char *name);
  void andorLabel(bool isON);

 private:
  std::vector<QCheckBox*> cutCheck;

};

#endif // SELECT_PANEL
