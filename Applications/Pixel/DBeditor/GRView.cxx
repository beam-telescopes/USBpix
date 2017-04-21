#include "GRView.h"
#include <QStringList>
#include <iostream>

GRView::GRView(QString text, QWidget* parent, Qt::WindowFlags fl, int type) : 
  QDialog(parent, fl){

  setupUi(this);

  QTableWidget *myTabWid = tableWidget;

  switch(type){

  case 0: // GR table
  default:{
    myTabWid->setColumnWidth(0,150);
    
    QStringList list = text.split("\n", QString::SkipEmptyParts);
    bool GRstart = false;
    int FEid=-1;
    for(QStringList::iterator it=list.begin(); it!=list.end(); it++){
      if((*it).left(6)=="*** FE"){
	FEid++;
	GRstart = false;
	if(FEid>0) {
	  QWidget *tab = new QWidget();
	  QVBoxLayout *verticalLayout = new QVBoxLayout(tab);
	  verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
	  QTableWidget *newTableWidget = new QTableWidget(tab);
	  if (newTableWidget->columnCount() < 3)
            newTableWidget->setColumnCount(3);
	  QTableWidgetItem *qtablewidgetitem0 = new QTableWidgetItem();
	  QTableWidgetItem *qtablewidgetitem1 = new QTableWidgetItem();
	  QTableWidgetItem *qtablewidgetitem2 = new QTableWidgetItem();
	  qtablewidgetitem0->setText("Register");
	  qtablewidgetitem1->setText("Sent value");
	  qtablewidgetitem2->setText("Read value");
	  newTableWidget->setHorizontalHeaderItem(0, qtablewidgetitem0);
	  newTableWidget->setHorizontalHeaderItem(1, qtablewidgetitem1);
	  newTableWidget->setHorizontalHeaderItem(2, qtablewidgetitem2);
	  newTableWidget->setObjectName(QString::fromUtf8("newTableWidget"));
	  verticalLayout->addWidget(newTableWidget);
	  tabWidget->addTab(tab, "FE "+QString::number(FEid));
	  myTabWid = newTableWidget;
	  myTabWid->setColumnWidth(0,150);
	}
      }
      if(GRstart) {
	myTabWid->setRowCount(myTabWid->rowCount()+1);
	QStringList txitems = (*it).split("\t", QString::SkipEmptyParts);
	int ncol=3;
	if(txitems.size()<3) ncol = txitems.size();
	for(int i=0;i<ncol;i++)
	  myTabWid->setItem(myTabWid->rowCount()-1,i,new QTableWidgetItem(txitems.at(i)));
	if(ncol==3){
	  int sent = txitems.at(1).toInt();
	  int read = txitems.at(2).toInt();
	  if(sent!=read){
	    for(int i=0;i<ncol;i++) myTabWid->item(myTabWid->rowCount()-1,i)->setBackground(QBrush(QColor("red")));
	  }
	}
      }
      if((*it)=="Register\tSent\tRead") GRstart=true;
    }
    break;}
  case 1: {// SC test
    setWindowTitle("Scn Chain Test Viewer");
    resize(1100, 600);
    myTabWid->setColumnWidth(0,120);
    myTabWid->setColumnWidth(1,750);
    myTabWid->setColumnWidth(2,100);
    QStringList hLabels;
    hLabels << "Pattern";
    hLabels << "Bits";
    hLabels << "Current";
    myTabWid->setHorizontalHeaderLabels(hLabels);

    QStringList list = text.split("\n", QString::SkipEmptyParts);
    QString bits1, bits2, currb, curra;
    bool SCstart = false;
    int ipat=0;
    const int bitmax=120;
    for(QStringList::iterator it=list.begin(); it!=list.end(); it++){
      if(SCstart) {
	// expected bit pattern
	bits1 = it->mid(6,it->length()-5);
	it++; // read bit pattern
	bits2 = it->mid(6,it->length()-5);
	it++; // current before
	currb = it->mid(14,it->length()-13);
	it++; // current after
	curra = it->mid(13,it->length()-12);
	// create 2*npart rows for segments of bitstreams
	int currRow = myTabWid->rowCount();
	int nparts = bits1.length()/bitmax;
	if((bits1.length()%bitmax)!=0) nparts++;
	myTabWid->setRowCount(currRow+2*nparts);

	for(int ip=0;ip<nparts;ip++){
	  QString bitp1 = bits1.mid(ip*bitmax, bitmax);
	  QString bitp2 = bits2.mid(ip*bitmax, bitmax);
	  QString label1 = QString::number(ipat);
	  if(nparts>1) label1 += " - p. " + QString::number(ip);
	  label1 += " - Expected";
	  QString label2 = QString::number(ipat);
	  if(nparts>1) label2 += " - p. " + QString::number(ip);
	  label2 += " - Read";
	  myTabWid->setItem(currRow+2*ip,  0,new QTableWidgetItem(label1));
	  myTabWid->setItem(currRow+2*ip+1,0,new QTableWidgetItem(label2));
	  myTabWid->setItem(currRow+2*ip,  1,new QTableWidgetItem(bitp1));
	  myTabWid->setItem(currRow+2*ip+1,1,new QTableWidgetItem(bitp2));
	  if(ip==0){
	    myTabWid->setItem(currRow,  2,new QTableWidgetItem(currb));
	    myTabWid->setItem(currRow+1,2,new QTableWidgetItem(curra));
	  } else{
	    myTabWid->setItem(currRow+2*ip,  2,new QTableWidgetItem(""));
	    myTabWid->setItem(currRow+2*ip+1,2,new QTableWidgetItem(""));
	  }
	  bool passed=true;
	  for(int ib=0;ib<bitp1.size();ib++)
	    if(bitp1.mid(ib,1)!=bitp2.mid(ib,1) && bitp1.mid(ib,1)!="X" && bitp1.mid(ib,1)!="x") passed=false;
	  if(!passed)
	    for(int i=0;i<3;i++) myTabWid->item(currRow+2*ip+1,i)->setBackground(QBrush(QColor("red")));
	}
	
	SCstart=false;
	ipat++;
      }
      if(it->left(7)=="Pattern") SCstart=true;
    }

    break;}
  }

}
GRView::~GRView(){
}
