#include <qprinter.h>
#if QT_VERSION >= 0x040000
#include <qprintdialog.h>
#endif
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>
#include "HitmapPlot.h"

class MyZoomer: public QwtPlotZoomer
{
public:
    MyZoomer(QwtPlotCanvas *canvas):
        QwtPlotZoomer(canvas)
    {
        setTrackerMode(AlwaysOn);
    }

    virtual QwtText trackerText(const QwtDoublePoint &pos) const
    {
        QColor bg(Qt::white);
#if QT_VERSION >= 0x040300
        bg.setAlpha(200);
#endif

        QwtText text = QwtPlotZoomer::trackerText(pos);
        text.setBackgroundBrush( QBrush( bg ));
        return text;
    }
};



HitmapPlot::HitmapPlot(QWidget *parent):
    QwtPlot(parent)
{
    d_spectrogram = new QwtPlotSpectrogram();

	setAxisScale(QwtPlot::xBottom, 0, 17, 3);
	setAxisScale(QwtPlot::yLeft, 0, 159, 20);

	QwtLinearColorMap colorMap(Qt::darkRed, Qt::yellow);
	colorMap.addColorStop(0.5, Qt::red);
    colorMap.addColorStop(1, Qt::yellow);
    d_spectrogram->setColorMap(colorMap);

    d_spectrogram->attach(this);

    QwtValueList contourLevels;
    for ( double level = 0.5; level < 10.0; level += 1.0 )
        contourLevels += level;
    d_spectrogram->setContourLevels(contourLevels);

    // A color bar on the right axis
    rightAxis = axisWidget(QwtPlot::yRight);
	rightAxis->setTitle("# hits");
    rightAxis->setColorBarEnabled(true);
    rightAxis->setColorMap(d_spectrogram->data().range(),
	        d_spectrogram->colorMap());

    setAxisScale(QwtPlot::yRight,
        d_spectrogram->data().range().minValue(),
        d_spectrogram->data().range().maxValue() );
    enableAxis(QwtPlot::yRight);

	//showContour(true);


    plotLayout()->setAlignCanvasToScales(true);
    replot();

    // LeftButton for the zooming
    // MidButton for the panning
    // RightButton: zoom out by 1
    // Ctrl+RighButton: zoom out to full size

	

    QwtPlotZoomer* zoomer = new MyZoomer(canvas());
#if QT_VERSION < 0x040000
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
        Qt::RightButton, Qt::ControlButton);
#else
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
        Qt::RightButton, Qt::ControlModifier);
#endif
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
        Qt::RightButton);

    QwtPlotPanner *panner = new QwtPlotPanner(canvas());
    panner->setAxisEnabled(QwtPlot::yRight, false);
    panner->setMouseButton(Qt::MidButton);

    // Avoid jumping when labels with more/less digits
    // appear/disappear when scrolling vertically

    const QFontMetrics fm(axisWidget(QwtPlot::yLeft)->font());
    QwtScaleDraw *sd = axisScaleDraw(QwtPlot::yLeft);
    sd->setMinimumExtent( fm.width("100.00") );

    const QColor c(Qt::darkBlue);
    zoomer->setRubberBandPen(c);
    zoomer->setTrackerPen(c);
	
}

void HitmapPlot::showContour(bool on)
{
    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, on);
    replot();
}

void HitmapPlot::showSpectrogram(bool on)
{
    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ImageMode, on);
    d_spectrogram->setDefaultContourPen(on ? QPen() : QPen(Qt::NoPen));
    replot();
}

void HitmapPlot::printPlot()
{
	QPrinter printer;
	printer.setOrientation(QPrinter::Landscape);
#if QT_VERSION < 0x040000
	printer.setColorMode(QPrinter::Color);
#if 0
	printer.setOutputFileName("/tmp/spectrogram.ps");
#endif
	if (printer.setup())
#else
#if 0
	printer.setOutputFileName("/tmp/spectrogram.pdf");
#endif
	QPrintDialog dialog(&printer);
	if ( dialog.exec() )
#endif
	{
		print(printer);
	}
}

void HitmapPlot::setData(const QwtRasterData &data)
{
	d_spectrogram->setData(data);
}

void HitmapPlot::reAdjustColorMap()
{
	rightAxis->setColorMap(d_spectrogram->data().range(),
	        d_spectrogram->colorMap());

	setAxisScale(QwtPlot::yRight,
		d_spectrogram->data().range().minValue(),
		d_spectrogram->data().range().maxValue() );
}

void HitmapPlot::setData(double * Array)
{
	d_spectrogram->setData(HitmapData(Array));
}