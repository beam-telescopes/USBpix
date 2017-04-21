#include <RenameWin.h>
RenameWin::RenameWin( QWidget* parent, const char* , bool , Qt::WFlags fl) : QDialog(parent,fl){
  setupUi(this);
  optBox->hide();
  QObject::connect(Name, SIGNAL(returnPressed()), this, SLOT(accept()));
}
RenameWin::~RenameWin(){}
