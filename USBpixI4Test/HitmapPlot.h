//#include <qwt_plot.h>
//#include <qwt_plot_spectrogram.h>
//#include <QObject>

#ifndef HITMAPPLOT_H
#define HITMAPPLOT_H
 
#include <QtGui/QHBoxLayout>
#include <QtGui/QWidget>
 
#include <qwt_plot.h>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>

class HitmapData: public QwtRasterData
{
private:
	double * m_Array;
	double m_minValue;
	double m_maxValue;
 
	struct structMinMax{
		double min;
		double max;
	};
	structMinMax m_RangeX;
	structMinMax m_RangeY;
	struct structXY{
		double x;
		double y;
	};
	structXY m_DataSize;
	structXY m_RealToArray;
 
public:
	// Constructor giving back the QwtRasterData Constructor
    HitmapData(double * Array): QwtRasterData(QwtDoubleRect(0, 17, 0, 159))
    {
		m_Array = new double [2880];
		setRangeX(0, 17);
		setRangeY(0, 159);
		setData(Array);
		for (int col = 0; col < 18; col++)
		{
			for (int row = 0; row < 160; row++)
			{
				value(col, row);
			}
		}		
	}
 
    ~HitmapData()
    {
		if (m_Array != NULL)
			delete [] m_Array;
    }
 
    virtual QwtRasterData *copy() const
    {
        HitmapData *clone = new HitmapData(m_Array);
		clone->setRangeX(m_RangeX.min, m_RangeX.max);
		clone->setRangeY(m_RangeY.min, m_RangeY.max);
		clone->setBoundingRect(QwtDoubleRect(m_RangeX.min, m_RangeY.min, m_RangeX.max, m_RangeY.max));
		clone->setData(m_Array);
		return clone;
    }
 
    virtual QwtDoubleInterval range() const
    {
        return QwtDoubleInterval(m_minValue, m_maxValue);
    }
 
    double value(double x, double y) const
    {
		//int xpos = (int)((x - m_RangeX.min) / m_RealToArray.x);
		//int ypos = (int)((y - m_RangeY.min) / m_RealToArray.y);
		int pos = ArrPos(x, y);//xpos, ypos);
		double dvalue = m_Array[pos];
		return dvalue;
    }
 
	void setData(double * Array)
	{
		MinMaxArrayValue(Array, 2880, m_minValue, m_maxValue);
		if (m_Array != NULL)
			delete [] m_Array;
		m_Array = new double [2880];
		memcpy(m_Array, Array, 2880 * sizeof(double));
 
		m_RealToArray.x = (m_RangeX.max - m_RangeX.min) / (m_DataSize.x);
		m_RealToArray.y = (m_RangeY.max - m_RangeY.min) / (m_DataSize.y);
	}
 
	void setRangeX(const double min, const double max)
	{
		m_RangeX.min = min;
		m_RangeX.max = max;
	}
 
	void setRangeY(const double min, const double max)
	{
		m_RangeY.min = min;
		m_RangeY.max = max;		
	}
 
	int ArrPos(const int x, const int y) const
	{
		int pixnr = 0;
		if ((x >= 0) && (x < 18) && (y >= 0) && (y < 160))
		{
			if ((x % 2) == 0) // even
				pixnr = (x * 160) + y;
			else // odd
				pixnr = (x * 160) + (159 - y);
		}
		else
		{
			pixnr = -1;
		}
		return pixnr;
		//return y + m_DataSize.y * x;
	}
	
	void MinMaxArrayValue(double * Array, int size, double &m_minValue, double &m_maxValue)
	{
		double min = Array[0]; 
		double max = Array[0];
		for (int i = 0; i < size; i++)
		{
			if (Array[i] <= min)
				min = Array[i];
			if (Array[i] >= max)
				max = Array[i];
		}
			
		m_minValue = min;
		m_maxValue = max;
	}
};

class HitmapPlot: public QwtPlot
{
    Q_OBJECT

public:
    HitmapPlot(QWidget * = NULL);
	

public slots:
    void showContour(bool on);
    void showSpectrogram(bool on);
    void printPlot();
	void setData(const QwtRasterData &data);
	void reAdjustColorMap();
	void setData(double * Array);

private:
    QwtPlotSpectrogram *d_spectrogram;
	QwtScaleWidget *rightAxis;
};
#endif