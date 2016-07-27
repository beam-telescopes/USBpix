#ifndef WAFERCOMBOBOX_H
#define WAFERCOMBOBOX_H

#include <string>

#include <QComboBox>
#include <QPalette>
#include <QLineEdit>
#include <QAbstractItemView>
#include <QAbstractItemModel>

class WaferComboBox : public QComboBox {
	Q_OBJECT

public:
	WaferComboBox(unsigned int pChipIndex, std::string pResultName, QString pValue, int pState = 3, QWidget* parent = 0);
	~WaferComboBox();
unsigned int getChipIndex();

public slots:
	void setState(int pState);

signals:
	void getState(unsigned int, std::string, int, QString);

private:
	unsigned int _chipIndex;
	int _resultState;
	QString _value;
	std::string _resultName;
};

#endif // WAFERCOMBOBOX_H
