#pragma once

#include <QWidget>
#include <QPixmap>
#include <QPainter>

class image:
	public QWidget
{
public:
image(QWidget *parent = 0);
	~image(void);
	QPixmap plot;
protected:
	void paintEvent(QPaintEvent *event);

};