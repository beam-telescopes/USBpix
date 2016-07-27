#ifndef WAFERBUTTON_H
#define WAFERBUTTON_H

#include <QPushButton>

class WaferButton : public QPushButton {
	Q_OBJECT

	public:
		WaferButton(QString path, QWidget* parent = 0);
		~WaferButton();

	public slots:
		void startViewer();

	signals:
		void showViewer(QString);

	private:
		QString m_path;
};

#endif // WAFERBUTTON_H
