#include "WaferButton.h"
#include <stdio.h>

WaferButton::WaferButton(QString path, QWidget* parent) : QPushButton("View", parent), m_path(path)
{
	QObject::connect(this, SIGNAL(clicked()), this, SLOT(startViewer()));
}
WaferButton::~WaferButton()
{
	;
}
void WaferButton::startViewer()
{
	emit showViewer(m_path);
}
