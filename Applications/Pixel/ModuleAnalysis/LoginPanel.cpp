#include "LoginPanel.h"
LoginPanel::LoginPanel(QWidget* parent, const char* , bool , Qt::WindowFlags fl) : QDialog(parent, fl){
  setupUi(this);
  QObject::connect(Password, SIGNAL(returnPressed()), this, SLOT(accept()));
}
LoginPanel::~LoginPanel(){}
