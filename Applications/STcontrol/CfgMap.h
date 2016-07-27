#ifndef CFGMAP_H
#define CFGMAP_H

#include "ui_CfgMap.h"
#include <QLabel>

class QMouseEvent;
class QImage;
class RefWin;
class QString;

class CfgPixMap : public QLabel
{
 Q_OBJECT

 public:
  CfgPixMap( QWidget * parent = 0);
  ~CfgPixMap(){};

 public slots:
  void mouseDoubleClickEvent (QMouseEvent *);
  void mouseReleaseEvent (QMouseEvent *);

 signals:
  void singleMapClick(uint x, uint y);
  void doubleMapClick(uint x, uint y);
};

class CfgMap : public QDialog, public Ui::CfgMap
{

 Q_OBJECT

 public:
  CfgMap(uint **data, uint maxval, uint ncol, uint nrow, bool flipped=false, QWidget* parent = 0, Qt::WindowFlags fl = 0 , 
	 const char *gname=0, const char *mname=0, int chip=0, uint sx=20, uint sy=4);
  ~CfgMap();

  uint **m_extData;

 public slots:
  void singleMapClick(uint x, uint y);
  void doubleMapClick(uint x, uint y);
  void readPixVal();
  void setPixVal();
  void setROI();
  void setColVal();
  void setRowVal();
  void setGangVal();
  void setIntgdVal();
  void setLongVal();
  void setAllVal();
  void maskMaker();
  void maskMakerHistoFill(const QString&);
  void save();
  uint nRow(){return m_nrow;};
  uint nCol(){return m_ncol;};

 private:
  void mapValueToPixelColor(unsigned int pX, unsigned int pY, double pValue);	//translates a value to a pixel color
  uint **m_data, m_maxval, m_ncol, m_nrow, m_scaleX, m_scaleY;
  int m_chip;
  RefWin *m_sclist;
  QString m_maskFile, m_gname, m_mname;
  QImage *m_img;
  bool m_flipped;

};
#endif // CFGMAP_H
