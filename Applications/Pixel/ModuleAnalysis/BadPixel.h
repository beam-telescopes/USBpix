#ifndef BADPIXEL
#define BADPIXEL

#include "ui_BadPixelBase.h"
#include <q3table.h>

class QString;
class ModItem;
class TopWin;

/*! table-panel for listing pixels identified bad somewhere
 * @authors Giorgio Cardarelli <giorgio@pcteor1.mi.infn.it>
 *  Joern Grosse-Knetter <jgrosse@physik.uni-bonn.de>
 */

class BadPixel : public QDialog, public Ui::BadPixelBase{

  Q_OBJECT

 public:
  BadPixel( ModItem *item=NULL, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
  ~BadPixel();
  void Clear();
  void AddTabLine(int xval, int yval, float val);
  void AddTabLine(QString *text);
  void Init(const char *name, const char *title);
  int  nRows(){return PixelList->numRows();};

public slots:
    void WriteToFile();
    void CreateMask();
    void copyToCB();

 private:
    ModItem *m_item;

};


#endif // BADPIXEL
