#include "image.h"

image::image(QWidget *parent)
{
   setFixedSize(880, 672);
   //setFixedSize(300,800);
   setWindowTitle("FE-I4 representation. Grey - off. Light Blue - on. Green - hit (darker=stronger)");
}

image::~image(void)
{
}

void image::paintEvent(QPaintEvent *)
{
   QPainter painter(this);
   painter.drawPixmap(0, 0, plot);
}