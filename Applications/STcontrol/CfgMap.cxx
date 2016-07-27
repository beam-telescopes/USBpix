#include "CfgMap.h"

#include <RefWin.h>
#include <MaskMaker.h>
#include <PixConfDBInterface/RootDB.h>
#include <DataContainer/PixelDataContainer.h>
#include <DataContainer/PixDBData.h>
#include <PixController/PixScan.h>
#include <Histo/Histo.h>

#include <QMouseEvent>
#include <QFileDialog>
#include <QMessageBox>

CfgMap::CfgMap(uint **data, uint maxval, uint ncol, uint nrow, bool flipped, QWidget* parent, Qt::WindowFlags fl, 
	       const char *gname, const char *mname, int chip, uint sx, uint sy)
  : QDialog(parent, fl), m_extData(data), m_maxval(maxval), m_ncol(ncol), m_nrow(nrow), m_chip(chip), m_flipped(flipped) {

  setupUi(this);
  QObject::connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(AcceptButton, SIGNAL(clicked()), this, SLOT(save()));

  m_gname="";
  m_mname="";
  if(gname!=0) m_gname = gname;
  if(mname!=0) m_mname = mname;

  m_scaleX = sx;
  m_scaleY = sy;

  rowVal->setMaximum(m_nrow-1);
  colVal->setMaximum(m_ncol-1);

  colLimitLow->setMaximum(m_ncol-1);
  rowLimitLow->setMaximum(m_nrow-1);
  colLimitHigh->setMaximum(m_ncol-1);
  rowLimitHigh->setMaximum(m_nrow-1);


  pixelSetVal->setMaximum(m_maxval);

  delete TheMap;
  TheMap = new CfgPixMap(this);
  TheMap->setObjectName(QString::fromUtf8("TheMap"));
  TheMap->setScaledContents(true);
  TheMap->setWordWrap(false);
  QObject::connect(TheMap, SIGNAL(doubleMapClick(uint,uint)), this, SLOT(doubleMapClick(uint,uint)));
  QObject::connect(TheMap, SIGNAL(singleMapClick(uint,uint)), this, SLOT(singleMapClick(uint,uint)));
  QObject::connect(colVal, SIGNAL(valueChanged(int)), this, SLOT(readPixVal()));
  QObject::connect(rowVal, SIGNAL(valueChanged(int)), this, SLOT(readPixVal()));
  QObject::connect(SetButton, SIGNAL(clicked()), this, SLOT(setPixVal()));

  QObject::connect(SetROIButton, SIGNAL(clicked()), this, SLOT(setROI()));

  QObject::connect(setColButton, SIGNAL(clicked()), this, SLOT(setColVal()));
  QObject::connect(setRowButton, SIGNAL(clicked()), this, SLOT(setRowVal()));
  QObject::connect(setGangedButton, SIGNAL(clicked()), this, SLOT(setGangVal()));
  QObject::connect(setIntGangedButton, SIGNAL(clicked()), this, SLOT(setIntgdVal()));
  QObject::connect(setLongButton, SIGNAL(clicked()), this, SLOT(setLongVal()));  
  QObject::connect(setAllButton, SIGNAL(clicked()), this, SLOT(setAllVal()));  
  QObject::connect(maskMakerButton, SIGNAL(clicked()), this, SLOT(maskMaker())); 

  //  mapHorizontalLayout->addWidget(TheMap);
  mapVerticalLayout->addWidget(TheMap);
  
  m_img = new QImage((int)(m_ncol*m_scaleX),(int)(m_nrow*m_scaleY),QImage::Format_RGB32);

  m_data = new uint*[m_ncol];
  for(uint i=0;i<m_ncol; i++){
    m_data[i] = new uint[m_nrow];
    for(uint j=0;j<m_nrow; j++){
      m_data[i][j] = data[i][m_flipped?(m_nrow-1-j):j];
      for(uint pi=0;pi<m_scaleX;pi++){
	for(uint pj=0;pj<m_scaleY;pj++){
		mapValueToPixelColor(m_scaleX*i+pi,m_scaleY*j+pj,(double) m_data[i][j]);
	}
      }
    }
  }
  TheMap->setPixmap(QPixmap::fromImage(*m_img));
  readPixVal();

  // panel needed for mask maker
  m_sclist = new RefWin(this);
  m_sclist->FuncselLabel->setText("select a scan:");
  m_sclist->SelectorLabel->setText("select a histogram to be used for mask generation");
  m_sclist->setWindowTitle("Select scan / histogram for masking");
  m_maskFile = "";

}
CfgMap::~CfgMap(){
  for(uint i=0;i<m_ncol; i++)
    delete[] m_data[i];
  delete[] m_data;
  delete m_img;
}
void CfgMap::save(){
  for(uint i=0;i<m_ncol; i++){
    for(uint j=0;j<m_nrow; j++){
      m_extData[i][j] = m_data[i][m_flipped?(m_nrow-1-j):j];
    }
  }
  accept();
}
void CfgMap::readPixVal(){
  int row = m_flipped?rowVal->value():(m_nrow-1-rowVal->value());
  pixelVal->setText(QString::number(m_data[colVal->value()][row]));
}
void CfgMap::setPixVal(){
  int row = m_flipped?rowVal->value():(m_nrow-1-rowVal->value());
  m_data[colVal->value()][row] = (uint)pixelSetVal->value();
  // refresh image
  for(uint pi=0;pi<m_scaleX;pi++){
    for(uint pj=0;pj<m_scaleY;pj++){
    	mapValueToPixelColor(m_scaleX*colVal->value()+pi,m_scaleY*row+pj,(double)pixelSetVal->value());
    }
  }
  TheMap->setPixmap(QPixmap::fromImage(*m_img));
  readPixVal();
}


void CfgMap::setROI(){

// set all pixels to 1
	  for(unsigned int k=0;k<m_ncol;k++){
	    for(unsigned int i =0;i<m_nrow;i++){
	      m_data[k][i] = 1;
	      // refresh image
	      for(uint pi=0;pi<m_scaleX;pi++){
		  for(uint pj=0;pj<m_scaleY;pj++){
			  mapValueToPixelColor(m_scaleX*k+pi,m_scaleY*i+pj,(double)m_maxval);
		}
	      }
	    }
	  }
	  TheMap->setPixmap(QPixmap::fromImage(*m_img));
	  readPixVal();

// set selected rectangle (region of interest) of pixels to 0
	  for (uint ROIi = (uint)colLimitLow->value(); ROIi<(uint)colLimitHigh->value(); ROIi++){
		  for (uint ROIj = (uint)rowLimitLow->value(); ROIj<(uint)rowLimitHigh->value(); ROIj++){
		    int row = m_flipped?ROIj:(m_nrow-1-ROIj);
		    m_data[ROIi][row] = 0;

		  // refresh image
    	  for(uint pi=0;pi<m_scaleX;pi++){
    		  for(uint pj=0;pj<m_scaleY;pj++){
    			  mapValueToPixelColor(m_scaleX*ROIi+pi,m_scaleY*row+pj,(double)pixelSetVal->value());
    		  	  	  	  	  	  	  	  	  }
		  	  	  	  	  	  	  	  	  }
		  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	}
	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	 }
	  TheMap->setPixmap(QPixmap::fromImage(*m_img));
	  readPixVal();
}


void CfgMap::setColVal(){
  for(unsigned int i =0;i<m_nrow;i++){
    m_data[colVal->value()][i] = (uint)pixelSetVal->value();

    // refresh image
    for(uint pi=0;pi<m_scaleX;pi++){
      for(uint pj=0;pj<m_scaleY;pj++){
    	  mapValueToPixelColor(m_scaleX*colVal->value()+pi,m_scaleY*i+pj,(double)pixelSetVal->value());
      }
    }
  }
  TheMap->setPixmap(QPixmap::fromImage(*m_img));
  readPixVal();
}
void CfgMap::setRowVal(){
  for(unsigned int i =0;i<m_ncol;i++){
    int row = m_flipped?rowVal->value():(m_nrow-1-rowVal->value());
    m_data[i][row] = (uint)pixelSetVal->value();
    // refresh image
    for(uint pi=0;pi<m_scaleX;pi++){
      for(uint pj=0;pj<m_scaleY;pj++){
    	  mapValueToPixelColor(m_scaleX*i+pi,m_scaleY*row+pj,(double)pixelSetVal->value());
      }
    }
  }
  TheMap->setPixmap(QPixmap::fromImage(*m_img));
  readPixVal();
}
void CfgMap::setGangVal(){
  const unsigned int ngrow=4;
  unsigned int grow[ngrow]={153,155,157,159};
  for(unsigned int k=0;k<ngrow;k++){
    for(unsigned int i =0;i<m_ncol;i++){
      int row = m_flipped?grow[k]:(m_nrow-1-grow[k]);
      m_data[i][row] = (uint)pixelSetVal->value();
      // refresh image
      for(uint pi=0;pi<m_scaleX;pi++){
	for(uint pj=0;pj<m_scaleY;pj++){
		mapValueToPixelColor(m_scaleX*i+pi,m_scaleY*row+pj,(double)pixelSetVal->value());
	}
      }
    }
  }
  TheMap->setPixmap(QPixmap::fromImage(*m_img));
}
void CfgMap::setIntgdVal(){
  const unsigned int nigrow=3;
  unsigned int igrow[nigrow]={154,156,158};
  for(unsigned int k=0;k<nigrow;k++){
    for(unsigned int i =0;i<m_ncol;i++){
      int row = m_flipped?igrow[k]:(m_nrow-1-igrow[k]);
     m_data[i][row] = (uint)pixelSetVal->value();
      // refresh image
      for(uint pi=0;pi<m_scaleX;pi++){
	for(uint pj=0;pj<m_scaleY;pj++){
		mapValueToPixelColor(m_scaleX*i+pi,m_scaleY*row+pj,(double)pixelSetVal->value());
	}
      }
    }
  }
  TheMap->setPixmap(QPixmap::fromImage(*m_img));
  readPixVal();
}
void CfgMap::setLongVal(){
  const unsigned int nlcol=2;
  unsigned int lcol[nlcol]={0,17};
  for(unsigned int k=0;k<nlcol;k++){
    for(unsigned int i =0;i<m_nrow;i++){
      m_data[lcol[k]][i] = (uint)pixelSetVal->value();
      // refresh image
      for(uint pi=0;pi<m_scaleX;pi++){
	for(uint pj=0;pj<m_scaleY;pj++){
		mapValueToPixelColor(m_scaleX*lcol[k]+pi,m_scaleY*i+pj,(double)pixelSetVal->value());
	}
      }
    }
  }
  TheMap->setPixmap(QPixmap::fromImage(*m_img));
  readPixVal();
}
void CfgMap::setAllVal(){
  for(unsigned int k=0;k<m_ncol;k++){
    for(unsigned int i =0;i<m_nrow;i++){
      m_data[k][i] = (uint)pixelSetVal->value();

      // refresh image
      for(uint pi=0;pi<m_scaleX;pi++){
	for(uint pj=0;pj<m_scaleY;pj++){
		mapValueToPixelColor(m_scaleX*k+pi,m_scaleY*i+pj,(double)pixelSetVal->value());
	}
      }
    }
  }
  TheMap->setPixmap(QPixmap::fromImage(*m_img));
  readPixVal();
}
void CfgMap::singleMapClick(uint x, uint y){
  //  printf("release at %d %d\n",x/m_scaleX, y/m_scaleY);
  colVal->setValue(x/m_scaleX);
  rowVal->setValue(m_flipped?(y/m_scaleY):(m_nrow-y/m_scaleY-1));
  readPixVal();
}
void CfgMap::doubleMapClick(uint, uint){//uint x, uint y){
  //printf("double click at %d %d\n",x,y);
}
void CfgMap::maskMaker(){
  m_maskFile = "";
  QStringList filter;
  filter += "DB ROOT data file (*.root)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this, "Specify name of RootDB data-file to be used for mask making");
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif  
  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::ExistingFile);
  if(fdia.exec() == QDialog::Accepted){
    m_maskFile = fdia.selectedFiles().at(0);
    m_maskFile.replace("\\", "/");
    RootDB *dfile = new RootDB(m_maskFile.toLatin1().data());

    DBInquire *root = dfile->readRootRecord(1);
    
    disconnect( m_sclist->FuncSel, SIGNAL( activated ( const QString &) ), this, SLOT( maskMakerHistoFill( const QString& ) ) );
    m_sclist->FuncSel->clear();
    m_sclist->DatSel->clear();

    for(recordIterator it = root->recordBegin(); it!=root->recordEnd(); it++){
      if((*it)->getName()=="PixScanResult"){
	std::string label = (*it)->getDecName();
	// remnove trailing and heading "/"
	label.erase(label.length()-1,1);
	label.erase(0,1);
	m_sclist->FuncSel->addItem(label.c_str());
      }
    }
    delete dfile;
    maskMakerHistoFill( m_sclist->FuncSel->currentText() );
    connect( m_sclist->FuncSel, SIGNAL( activated ( const QString &) ), this, SLOT( maskMakerHistoFill( const QString& ) ) );

    if(m_sclist->FuncSel->count()>0){
      if(m_sclist->exec()){
	PixScan ps;
	int htype = ps.getHistoTypes()[m_sclist->DatSel->currentText().toLatin1().data()];
	QString path = m_maskFile + ":/"+ m_sclist->FuncSel->currentText() + "/" + m_gname;
	PixDBData *DataObj=0;
	try{
	  DataObj = new PixDBData("Name",path.toLatin1().data(),m_mname.toLatin1().data());
	} catch(...){
	  DataObj = 0;
	}
	if(DataObj==0)
	  QMessageBox::warning(this,"CfgMap::maskMaker","Couldn't find data set for scan " + m_sclist->FuncSel->currentText() +
			       " and module "+m_mname);
	MaskMaker mm(this);
 	mm.setHistoLabel(htype);
	if(mm.exec()==QDialog::Accepted){
	  //QApplication::setOverrideCursor(Qt::waitCursor);
	  PixLib::Histo *h = DataObj->GenMask((PixLib::PixScan::HistogramType)htype, 0, -1, mm.getMin(),mm.getMax());
	  if(h!=0 && h->nDim()==2){
	    // transfer result from histo to local map
	    for(uint i=0;i<m_ncol; i++){
	      for(uint j=0;j<m_nrow; j++){
		int ih, jh;
		DataObj->PixCCRtoBins(m_chip, (int)i, (int)j, ih, jh);
		ih--; jh--; // function was written for ROOT, which starts at 1, Histo starts at 0...
		if(ih<h->nBin(0) && jh<h->nBin(1))
		  m_data[i][j] = (unsigned int)(*h)(ih,jh);
		else
		  m_data[i][j] = 0;
		for(uint pi=0;pi<m_scaleX;pi++){
		  for(uint pj=0;pj<m_scaleY;pj++){
			  mapValueToPixelColor(m_scaleX*i+pi,m_scaleY*(m_nrow-j-1)+pj,(double)m_data[i][j]);
		  }
		}
	      }
	    }
	    TheMap->setPixmap(QPixmap::fromImage(*m_img));
	    readPixVal();
	  } else if(h!=0)
	    QMessageBox::warning(this,"CfgMap::maskMaker","Mask generated has wrong dimensions: "+QString::number(h->nDim()));
	  else
	    QMessageBox::warning(this,"CfgMap::maskMaker","Mask generated is NULL -> was module " + m_mname + " in this scan?");
	  delete h;
	}
	delete DataObj;
      }
    } else{
      QMessageBox::warning(this,"CfgMap::maskMaker","No scans found in file "+m_maskFile);
    }
  }  
}
void CfgMap::maskMakerHistoFill(const QString &scanLabel){
  QString path = m_maskFile + ":/"+ scanLabel.toLatin1().data() + "/" + m_gname;
  PixDBData *DataObj=0;
  m_sclist->DatSel->clear();
  try{
    DataObj = new PixDBData("Name",path.toLatin1().data(),m_mname.toLatin1().data());
    for(int ih=0;ih<(int)PixLib::PixScan::MAX_HISTO_TYPES; ih++){
      if(DataObj->haveHistoType(ih) && ih!=PixLib::PixScan::DCS_DATA) // DCS data are 1D type
	m_sclist->DatSel->addItem(DataObj->getHistoName((PixLib::PixScan::HistogramType)ih).c_str());
    }
  } catch(...){
    DataObj = 0;
  }
  delete DataObj;
  m_sclist->OKButt->setEnabled(m_sclist->DatSel->count()>0);
}

void CfgMap::mapValueToPixelColor(unsigned int pX, unsigned int pY, double pValue)
{
	double pRedValue = 0;
	double pGreenValue = 0;
	double pBlueValue = 0;

	if(m_maxval > 128){	//only use new mapping for higher range to prevent confusion
		if(pValue<(double)m_maxval/3.){
			pRedValue = 255. / m_maxval * pValue * 3.;
			pGreenValue = 0;
			pBlueValue = 0;
		}
		if(pValue>(double)m_maxval/3. && pValue<2.*(double)m_maxval/3.){
			pRedValue = 0;
			pGreenValue = 0;
			pBlueValue = 255. / m_maxval * pValue * 3.;
		}
		if(pValue>2.*(double)m_maxval/3.){
			pRedValue = 0;
			pGreenValue = 255. / m_maxval * pValue * 3.;
			pBlueValue = 0;
		}
	}
	else
		pRedValue = 255. / m_maxval * pValue;
	m_img->setPixel(pX, pY,qRgb((int)pRedValue,(int)pGreenValue,(int)pBlueValue));
}

// the actual pixel map
CfgPixMap::CfgPixMap( QWidget * parent) : QLabel(parent){
}
void CfgPixMap::mouseDoubleClickEvent (QMouseEvent *event){
  emit doubleMapClick(event->x(), event->y());
}
void CfgPixMap::mouseReleaseEvent (QMouseEvent *event){
  emit singleMapClick(event->x(), event->y());
}
