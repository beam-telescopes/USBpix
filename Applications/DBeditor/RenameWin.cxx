#include <RenameWin.h>
RenameWin::RenameWin( QWidget* parent, Qt::WindowFlags fl) : QDialog(parent,fl){
  setupUi(this);
  optBox->hide();
  QObject::connect(Name, SIGNAL(returnPressed()), this, SLOT(accept()));
}
RenameWin::~RenameWin(){}
