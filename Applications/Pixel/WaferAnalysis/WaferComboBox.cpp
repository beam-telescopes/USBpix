#include "WaferComboBox.h"
#include <iostream>
WaferComboBox::WaferComboBox(unsigned int pChipIndex, std::string pResultName, QString pValue, int pState, QWidget* parent) : QComboBox(parent)
{
	_chipIndex = pChipIndex;
	_resultName = pResultName;
	_value=pValue;

	this->addItem(pValue);
	this->addItem(pValue);
	this->addItem(pValue);
	this->addItem(pValue);

	switch (pState){
		case 5:
			this->setStyleSheet("QComboBox { background-color: lightgreen; }");
			break;
		case 6:
			this->setStyleSheet("QComboBox { background-color: yellow; }");
			break;
		case 7:
			this->setStyleSheet("QComboBox { background-color: tomato; }");
			break;
		default:
			this->setStyleSheet("QComboBox { background-color: lightblue; }");
	}

	this->setEditable(true);
	this->lineEdit()->setAlignment(Qt::AlignHCenter);
	this->lineEdit()->setReadOnly(true);
	this->setItemData(0, QBrush("lightblue"), Qt::BackgroundRole);
	this->setItemData(1, QBrush("tomato"), Qt::BackgroundRole);
	this->setItemData(2, QBrush("yellow"), Qt::BackgroundRole);
	this->setItemData(3, QBrush("lightgreen"), Qt::BackgroundRole);
	for (int i = 0; i < this->count(); ++i)
		this->setItemData(i, Qt::AlignHCenter, Qt::TextAlignmentRole);
	QAbstractItemView *view = this->view();
	QAbstractItemModel *model = view->model();
	model->setData(model->index(0, 0), QVariant(Qt::AlignCenter), Qt::TextAlignmentRole);
	model->setData(model->index(1, 0), QVariant(Qt::AlignCenter), Qt::TextAlignmentRole);
	model->setData(model->index(2, 0), QVariant(Qt::AlignCenter), Qt::TextAlignmentRole);
	model->setData(model->index(3, 0), QVariant(Qt::AlignCenter), Qt::TextAlignmentRole);
	QObject::connect(this, SIGNAL(activated(int)), this, SLOT(setState(int)));

}

WaferComboBox::~WaferComboBox()
{
}

unsigned int WaferComboBox::getChipIndex()
{
	return _chipIndex;
}

void WaferComboBox::setState(int pState)
{
	_resultState = pState;
//	this->setStyleSheet("QComboBox { background-color: lightblue; }");
//	if (pState == 0) this->setStyleSheet("QComboBox { background-color: lightblue; }");
//	if (pState == 1){
//		this->setStyleSheet("QComboBox { background-color: tomato; }");
//		std::cout<<"!! "<<std::endl;
//	}
//	if (pState == 2) this->setStyleSheet("QComboBox { background-color: yellow; }");
//	if (pState == 3) this->setStyleSheet("QComboBox { background-color: lightgreen; }");
//	std::cout<<"!! state "<<pState<<std::endl;
	emit getState(_chipIndex, _resultName, _resultState, _value);
}
