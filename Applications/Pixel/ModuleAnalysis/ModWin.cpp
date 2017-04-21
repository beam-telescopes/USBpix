#include <ModWin.h>
#include <QDialog>

ModWin::ModWin( QWidget* parent,  const char* , bool , Qt::WFlags fl )
  : QDialog(parent, fl){
  setupUi(this);
}
ModWin::~ModWin(){}
