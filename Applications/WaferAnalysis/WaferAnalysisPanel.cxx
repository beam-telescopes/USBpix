#include <QFileDialog>
#include <QString>
#include <QDir>
#include <QTableWidget>
#include <QMessageBox>
#include <QTextEdit>
#include <QComboBox>

#include <QMimeData>
#include <QUrl>

#include <TSystem.h>

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>

#include <DataViewer.h>
#include "WaferButton.h"
#include "WaferComboBox.h"
#include "WaferAnalysisPanel.h"

#define _MULTITHREAD false

WaferAnalysisPanel::WaferAnalysisPanel(QWidget * parent, Qt::WindowFlags flags) : 
QMainWindow (parent, flags), _waferAnalysis()
{
	setupUi(this);
	setAcceptDrops(true);
	QObject::connect(actionAdd_Data, SIGNAL(triggered()), this, SLOT(addData()));
	QObject::connect(actionCreateXML, SIGNAL(triggered()), this, SLOT(exportResultsToXML()));
	QObject::connect(actionExport_data_textfile, SIGNAL(triggered()), this, SLOT(exportDataTextFile()));
	QObject::connect(actionWrite_Ranking_Data, SIGNAL(triggered()), this, SLOT(writeRankingData()));
	QObject::connect(actionClear_Data, SIGNAL(triggered()), this, SLOT(clearData()));
	QObject::connect(actionExit, SIGNAL(triggered()), this, SLOT(exitAll()));
	QObject::connect(actionOpen_Cuts, SIGNAL(triggered()), this, SLOT(browseCuts()));
	QObject::connect(actionOpen_Settings, SIGNAL(triggered()), this, SLOT(browseSettings()));
	QObject::connect(actionReload_Cuts, SIGNAL(triggered()), this, SLOT(refreshResultCuts()));
	QObject::connect(actionReload_Settings, SIGNAL(triggered()), this, SLOT(reloadSettings()));
	QObject::connect(pushButton_Analyze, SIGNAL(clicked()), this, SLOT(analyzeData()));
	QObject::connect(pushButton_resetStatus, SIGNAL(clicked()), this, SLOT(resetChipStatuse()));
	QObject::connect(actionCreate_Wafer_Plots, SIGNAL(triggered()), this, SLOT(createWaferResult()));
	QObject::connect(actionDebug, SIGNAL(triggered()), this, SLOT(setDebugOutput()));
	QObject::connect(actionInfo, SIGNAL(triggered()), this, SLOT(setInfoOutput()));
	QObject::connect(actionWarning, SIGNAL(triggered()), this, SLOT(setWarningOutput()));
	QObject::connect(actionError, SIGNAL(triggered()), this, SLOT(setErrorOutput()));
	QObject::connect(actionCreateBugReport, SIGNAL(triggered()), this, SLOT(setBugReport()));
	QObject::connect(actionCreate_Chip_Plots, SIGNAL(triggered()), this, SLOT(setChipPlotOutput()));
	QObject::connect(actionSave_Project, SIGNAL(triggered()), this, SLOT(saveProject()));
	QObject::connect(actionOpen_Project, SIGNAL(triggered()), this, SLOT(openProject()));
	QObject::connect(actionAdd_Project_Data, SIGNAL(triggered()), this, SLOT(addProjectData()));
	QObject::connect(spinBox_waferNr, SIGNAL(valueChanged(int)), this, SLOT(setWaferSerialNumber(int)));
	QObject::connect(actionInfo_2, SIGNAL(triggered()), this, SLOT(showInfoWindow()));

	m_dataView = new DataViewer(0, 0, true );

	setWindowTitle("WaferAnalysis");

	label_green->setStyleSheet("QLabel { background-color : lightgreen; color : black; }");
	label_yellow->setStyleSheet("QLabel { background-color : yellow; color : black; }");
	label_red->setStyleSheet("QLabel { background-color : tomato; color : black; }");
	label_blue->setStyleSheet("QLabel { background-color : lightblue; color : black; }");
	label_nocut->setStyleSheet("QLabel { background-color : lightgrey; color : black; }");

	progressBarSetting = 0;
	_twoChip = false;

	tryAutoOpenCutFile();
	tryAutoOpenSettingsFile();
}
WaferAnalysisPanel::~WaferAnalysisPanel()
{
  delete m_dataView;
}

void WaferAnalysisPanel::addData()
{
	QStringList filter;
	filter += "DB ROOT file (*.root)";
	filter += "Any file (*.*)";
	QFileDialog fdia(this,"Select any wafer or 1 chip data file");
#if defined(QT5_FIX_QDIALOG)
	fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
	fdia.setFileMode(QFileDialog::ExistingFile);
	if(fdia.exec() == QDialog::Accepted){
	  QString fname = fdia.selectedFiles().at(0);
		QApplication::setOverrideCursor(Qt::WaitCursor);
		if(_waferAnalysis._waferData.getNwafers() > 0){
			_waferAnalysis._waferData.addFileGroup(std::string(fname.toLatin1().data()));
			plotTable();
		}
		else{
			if(checkForSettingsCutsFile()){
				_waferAnalysis._waferData.openFileGroup(std::string(fname.toLatin1().data()));
				plotTable();
				waferName->setText(_waferAnalysis._waferData.getWaferName().c_str());
				label_chipNr->setText(QString::number(_waferAnalysis._waferData.getNchips()));
			}
		}
		QApplication::restoreOverrideCursor();
	}
}

void WaferAnalysisPanel::openProject()
{
	QStringList filter;
	filter += "wafer analysis project (*.waprj)";
	filter += "Any file (*.*)";
	QFileDialog fdia(this,"Select a project file");
#if defined(QT5_FIX_QDIALOG)
	fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
	fdia.setNameFilters(filter);
	fdia.setFileMode(QFileDialog::ExistingFile);
	if(fdia.exec() == QDialog::Accepted){
		clearData();
		QString fname = fdia.selectedFiles().at(0);
		QApplication::setOverrideCursor(Qt::WaitCursor);
		_waferAnalysis.openProject(std::string(fname.toLatin1().data()));
		waferName->setText(_waferAnalysis._waferData.getWaferName().c_str());
		label_chipNr->setText(QString::number(_waferAnalysis._waferData.getNchips()));
		std::string tCutFileName = _waferAnalysis._waferCuts.getCutFileName();
		tCutFileName = tCutFileName.substr(tCutFileName.find_last_of("/")+1,tCutFileName.size()-tCutFileName.find_last_of("/"));
		cutsFileName->setText(QString(tCutFileName.c_str()));
		std::string tSettingsFileName = _waferAnalysis._waferSettings.getSettingsFileName();
		tSettingsFileName = tSettingsFileName.substr(tSettingsFileName.find_last_of("/")+1,tSettingsFileName.size()-tSettingsFileName.find_last_of("/"));
		settingsFileName->setText(QString(tSettingsFileName.c_str()));
		_waferAnalysis._waferSettings.ReadSettingsFile();
		_waferAnalysis._waferCuts.ReadCutFile();
		label_cutversion->setText(QString::number(_waferAnalysis._waferCuts.getCutFileVersion()));
		plotTable();
	}
	QApplication::restoreOverrideCursor();
}

void WaferAnalysisPanel::addProjectData()
{
	if(_waferAnalysis._waferData.getNwafers() == 0){
		QMessageBox::warning(this,"No data already loaded", "Use open project/add data.");
		return;
	}
	QStringList filter;
	filter += "Wafer Analysis project (*.waprj)";
	filter += "Any file (*.*)";
	QFileDialog fdia(this,"Select any Wafer Analysis project file");
#if defined(QT5_FIX_QDIALOG)
	fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
	fdia.setFileMode(QFileDialog::ExistingFile);
	if(fdia.exec() == QDialog::Accepted){
	  QString fname = fdia.selectedFiles().at(0);
		QApplication::setOverrideCursor(Qt::WaitCursor);
		_waferAnalysis.addProjectData(std::string(fname.toLatin1().data()));
		plotTable();
		QApplication::restoreOverrideCursor();
	}
}

void WaferAnalysisPanel::openProject(std::string tProjectFileName)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	_waferAnalysis.openProject(tProjectFileName);
	waferName->setText(_waferAnalysis._waferData.getWaferName().c_str());
	label_chipNr->setText(QString::number(_waferAnalysis._waferData.getNchips()));
	std::string tCutFileName = _waferAnalysis._waferCuts.getCutFileName();
	tCutFileName = tCutFileName.substr(tCutFileName.find_last_of("/")+1,tCutFileName.size()-tCutFileName.find_last_of("/"));
	cutsFileName->setText(QString(tCutFileName.c_str()));
	std::string tSettingsFileName = _waferAnalysis._waferSettings.getSettingsFileName();
	tSettingsFileName = tSettingsFileName.substr(tSettingsFileName.find_last_of("/")+1,tSettingsFileName.size()-tSettingsFileName.find_last_of("/"));
	settingsFileName->setText(QString(tSettingsFileName.c_str()));
	_waferAnalysis._waferSettings.ReadSettingsFile();
	_waferAnalysis._waferCuts.ReadCutFile();
	label_cutversion->setText(QString::number(_waferAnalysis._waferCuts.getCutFileVersion()));
	plotTable();
	QApplication::restoreOverrideCursor();
}

void WaferAnalysisPanel::saveProject()
{
	QFileDialog tFileDialog(this);
#if defined(QT5_FIX_QDIALOG)
	tFileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
	QStringList tNameFilter;
	tNameFilter+="wafer analysis project (*.waprj)";
	tFileDialog.setDefaultSuffix("waprj");
	tFileDialog.setAcceptMode(QFileDialog::AcceptSave);//To save files,default is open mode
	QString tFilePath = tFileDialog.getSaveFileName(this);
	if(!tFilePath.isEmpty()){
		QApplication::setOverrideCursor(Qt::WaitCursor);
		_waferAnalysis.saveProject(std::string(tFilePath.toLatin1().data()));
	}
	QApplication::restoreOverrideCursor();
}

void WaferAnalysisPanel::exportResultsToXML()
{
	_waferAnalysis.exportResultsToXML();
}

void WaferAnalysisPanel::exportDataTextFile()
{
	QFileDialog tFileDialog(this);
#if defined(QT5_FIX_QDIALOG)
	tFileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
	QStringList tNameFilter;
	tNameFilter+="tabular seperated data file (*.txt)";
	tFileDialog.setAcceptMode(QFileDialog::AcceptSave);//To save files,default is open mode
	tFileDialog.setDefaultSuffix("txt");
	QString tFilePath = tFileDialog.getSaveFileName(this);
	if(!tFilePath.isEmpty()){
		QApplication::setOverrideCursor(Qt::WaitCursor);
		_waferAnalysis.exportOverviewData(std::string(tFilePath.toLatin1().data()));
	}
	QApplication::restoreOverrideCursor();
}

void WaferAnalysisPanel::writeRankingData()
{
	_waferAnalysis.writeRankingData();
}

void WaferAnalysisPanel::clearData()
{
	_waferAnalysis._waferData.clearAllData();
	resultsTable->clear();
	resultsTable->setColumnCount(0);
	resultsTable->setRowCount(0);
	resultsTable->setHorizontalHeaderLabels(QStringList(""));
	waferName->setText("(none)");
	label_chipNr->setText("-");
}

void WaferAnalysisPanel::clearAnalyzedData()
{
	_waferAnalysis._waferData.clearAllData();
	resultsTable->clear();
	resultsTable->setColumnCount(0);
	resultsTable->setRowCount(0);
	resultsTable->setHorizontalHeaderLabels(QStringList(""));
	waferName->setText("(none)");
	label_chipNr->setText("-");
}

void  WaferAnalysisPanel::startViewer(QString path){
  m_dataView->browseFile(path.toLatin1().data());
	m_dataView->setWindowModality(Qt::WindowModal);
	m_dataView->show();
}
void  WaferAnalysisPanel::exitAll(){
	m_dataView->close();
	close();
}
void  WaferAnalysisPanel::browseCuts(){
	QStringList filter;
	filter += "Text file (*.txt)";
	filter += "Any file (*.*)";
	QFileDialog fdia(this,"Specify the name of the cuts file");
#if defined(QT5_FIX_QDIALOG)
	fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
	fdia.setNameFilters(filter);
	fdia.setFileMode(QFileDialog::ExistingFile);
	if(fdia.exec() == QDialog::Accepted){
	  _waferAnalysis._waferCuts.setCutFileName(QString(fdia.selectedFiles().at(0)).toStdString());
		_waferAnalysis._waferCuts.ReadCutFile();
		label_cutversion->setText(QString::number(_waferAnalysis._waferCuts.getCutFileVersion()));
		std::string tCutFileName = _waferAnalysis._waferCuts.getCutFileName();
		tCutFileName = tCutFileName.substr(tCutFileName.find_last_of("/")+1,tCutFileName.size()-tCutFileName.find_last_of("/"));
		cutsFileName->setText(QString(tCutFileName.c_str()));
		actionAdd_Data->setEnabled(true);
		actionAdd_Data_2Chip->setEnabled(true);
	}
}

void  WaferAnalysisPanel::browseSettings(){
	QStringList filter;
	filter += "Text file (*.txt)";
	filter += "Any file (*.*)";
	QFileDialog fdia(this,"Specify the name of the settings file");
#if defined(QT5_FIX_QDIALOG)
	fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
	fdia.setNameFilters(filter);
	fdia.setFileMode(QFileDialog::ExistingFile);
	if(fdia.exec() == QDialog::Accepted){
	  _waferAnalysis._waferSettings.setSettingsFileName(QString(fdia.selectedFiles().at(0)).toStdString());
		_waferAnalysis._waferSettings.ReadSettingsFile();
		std::string tSettingsFileName = _waferAnalysis._waferSettings.getSettingsFileName();
		tSettingsFileName = tSettingsFileName.substr(tSettingsFileName.find_last_of("/")+1,tSettingsFileName.size()-tSettingsFileName.find_last_of("/"));
		settingsFileName->setText(QString(tSettingsFileName.c_str()));
	}
}

void  WaferAnalysisPanel::plotTable()
{
	resultsTable->hide();	//some QT people say this enhances the speed when you redraw the table
	resultsTable->clear();
	if (_waferAnalysis._waferData.getNchips() > 0)
		plotTableFrame();
	for (unsigned int i=0; i <_waferAnalysis._waferData.getNchips(); ++i){
		plotTableIndex(i);
		if (i * _waferAnalysis._waferData.getNresults() < __MAXTABLEENTRIES) plotTableRow(i);	//do not plot all data (QT speed reason)
	}
	if (_waferAnalysis._waferData.getNchips() > 0)
		plotTableSummary();
	resultsTable->show();	//some QT people say this enhances the speed when you redraw the table
}
void  WaferAnalysisPanel::analyzeData()
{
	analyzesStarted();
	progressBar->setValue(0);
	progressBarSetting = 0;
	_waferAnalysis._waferCuts.ReadCutFile();
	label_cutversion->setText(QString::number(_waferAnalysis._waferCuts.getCutFileVersion()));
	_waferAnalysis._waferSettings.ReadSettingsFile();

	if(_MULTITHREAD){
		QThread* tWorkerThread = new QThread;	//thread to to the analysis
		WaferWorker* tWaferWorker = new WaferWorker(_waferAnalysis);	//worker that does the analysis
		tWaferWorker->moveToThread(tWorkerThread);
		QObject::connect(tWorkerThread, SIGNAL(started()), tWaferWorker, SLOT(start()));
		QObject::connect(tWaferWorker, SIGNAL(progress()), this, SLOT(increaseProgressBar()));
		QObject::connect(tWaferWorker, SIGNAL(finished()), tWorkerThread, SLOT(quit()));
		QObject::connect(tWaferWorker, SIGNAL(finished()), tWaferWorker, SLOT(deleteLater()));
		QObject::connect(tWaferWorker, SIGNAL(finished()), tWorkerThread, SLOT(deleteLater()));
		QObject::connect(tWaferWorker, SIGNAL(finished()), this, SLOT(refreshTableSummary()));
		QObject::connect(tWaferWorker, SIGNAL(finished()), this, SLOT(analyzesFinished()));
		tWorkerThread->start();
	}
	else{
		for (unsigned int i = 0; i < _waferAnalysis._waferData.getNchips(); ++i){
			_waferAnalysis.openAndAnalyzeChipFile(i);
			progressBar->setValue((int) ((double)(i+1)/(double)_waferAnalysis._waferData.getNchips()*100.));
		}
		_waferAnalysis._waferData.calculateChipStatuses();
		_waferAnalysis._waferData.AverageResults();
		plotTable();
		analyzesFinished();
	}
}

void  WaferAnalysisPanel::refreshResultCuts()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	progressBar->setValue(0);
	_waferAnalysis._waferCuts.ReadCutFile();
	label_cutversion->setText(QString::number(_waferAnalysis._waferCuts.getCutFileVersion()));
	for (unsigned int i = 0; i < _waferAnalysis._waferData.getNchips(); ++i){
		_waferAnalysis.refreshResultCuts(i);
		progressBar->setValue((int) ((double)(i+1)/(double)_waferAnalysis._waferData.getNchips()*100.));
	}
	_waferAnalysis._waferData.calculateChipStatuses();
	_waferAnalysis._waferData.AverageResults();
	label_cutversion->setText(QString::number(_waferAnalysis._waferCuts.getCutFileVersion()));
	plotTable();
	QApplication::restoreOverrideCursor();
}

void  WaferAnalysisPanel::addTableText(unsigned int pX, unsigned int pY, QString pQstring, QBrush pQbrush)
{
	QTableWidgetItem* tItem = new QTableWidgetItem(pQstring);
	tItem->setTextAlignment(4);
	resultsTable->setItem(pX,pY, tItem);
	resultsTable->item(pX,pY)->setBackground(pQbrush);
}

void WaferAnalysisPanel::addTableItem(unsigned int pX, unsigned int pY, QWidget* pWidget)
{
	resultsTable->setCellWidget(pX,pY,pWidget);
}

void WaferAnalysisPanel::createWaferResult()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	_waferAnalysis.createWaferResults();
	QApplication::restoreOverrideCursor();
}

void WaferAnalysisPanel::setDebugOutput()
{
	_waferAnalysis.setDebugOutput(actionDebug->isChecked());
}
void WaferAnalysisPanel::setBugReport()
{
	_waferAnalysis.setBugReport(actionCreateBugReport->isChecked());
}
void WaferAnalysisPanel::setInfoOutput()
{
	_waferAnalysis.setInfoOutput(actionInfo->isChecked());
}

void WaferAnalysisPanel::setWarningOutput()
{
	_waferAnalysis.setWarningOutput(actionWarning->isChecked());
}

void WaferAnalysisPanel::setErrorOutput()
{
	_waferAnalysis.setErrorOutput(actionError->isChecked());
}

void WaferAnalysisPanel::setWaferSerialNumber(int pSerialNumber)
{
	_waferAnalysis._waferData.setWaferSerialNumber((unsigned int) pSerialNumber);
}

void WaferAnalysisPanel::setResultStatus(unsigned int pChipIndex, std::string pResultName, int pResultStatus, QString)
{
	switch (pResultStatus){
		case 1:
			_waferAnalysis._waferData.setResultStatus(pChipIndex,pResultName,SETRED);
			break;
		case 2:
			_waferAnalysis._waferData.setResultStatus(pChipIndex,pResultName,SETYELLOW);
			break;
		case 3:
			_waferAnalysis._waferData.setResultStatus(pChipIndex,pResultName,SETGREEN);
			break;
		default:
			_waferAnalysis._waferData.setResultStatus(pChipIndex,pResultName,BLUE);
	}
	_waferAnalysis._waferData.calculateChipStatus(pChipIndex);
	plotTableRow(pChipIndex);
	plotTableSummary();
}
void WaferAnalysisPanel::setChipStatus(unsigned int pChipIndex, std::string /*pResultName*/, int pChipStatus, QString)
{
	switch (pChipStatus){
		case 1:
			_waferAnalysis._waferData.setChipStatus(pChipIndex,SETRED);
			break;
		case 2:
			_waferAnalysis._waferData.setChipStatus(pChipIndex,SETYELLOW);
			break;
		case 3:
			_waferAnalysis._waferData.setChipStatus(pChipIndex,SETGREEN);
			break;
		default:
			_waferAnalysis._waferData.setChipStatus(pChipIndex,BLUE);
	}
//	plotTableIndex(pChipIndex);
	plotTableRow(pChipIndex);
	plotTableSummary();
}

void WaferAnalysisPanel::plotTableFrame()
{
	QStringList list;
	list<<"#chip";
	list<<"WaferSN";
	list<<"Wafer/Chip type";
	std::list<std::string> tResultNames;
	_waferAnalysis._waferData.getResultNames(tResultNames);

	for (std::list<std::string>::iterator it=tResultNames.begin(); it!=tResultNames.end(); ++it)
		list<<it->c_str();

	list<<"Action";
	resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); //results cannot be changed by user
	resultsTable->setColumnCount(list.size());
	resultsTable->setColumnWidth(0,40);
	resultsTable->setColumnWidth(1,75);
	resultsTable->setColumnWidth(2,75);
	resultsTable->setColumnWidth(3,75);
	resultsTable->setColumnWidth(4,80);
	resultsTable->setColumnWidth(5,80);
	resultsTable->setColumnWidth(6,80);
	resultsTable->setColumnWidth(11,40);
	resultsTable->setColumnWidth(12,40);
	resultsTable->setHorizontalHeaderLabels(list);
	resultsTable->verticalHeader()->hide();	//header cannot have different colors
	resultsTable->setRowCount(_waferAnalysis._waferData.getNchips()+7);	//+7 for average, deviation and results statistics output
}
void WaferAnalysisPanel::plotTableRow(unsigned int pIndex)
{
	std::list<std::string> tResultNames;
	_waferAnalysis._waferData.getResultNames(tResultNames);

	QBrush tCellCol;

	int j = 2;

	for (std::list<std::string>::iterator it=tResultNames.begin(); it!=tResultNames.end(); ++it){
		double tValue = 0;
		j++;
		status tResultStatus = GREY;
		if (_waferAnalysis._waferData.getResult(pIndex, *it, tValue)){
			_waferAnalysis._waferData.getResultStatus(pIndex, *it, tResultStatus);
			if (tResultStatus != BLUE && tResultStatus != SETRED && tResultStatus != SETYELLOW && tResultStatus != SETGREEN){
				addTableText(pIndex,j,QString::number(tValue, 'd',0),QBrush("white"));
				switch(tResultStatus){
					case GREY:
						tCellCol = QBrush("lightgrey");
						break;
					case GREEN:
						tCellCol = QBrush("lightgreen");
						break;
					case YELLOW:
						tCellCol = QBrush("yellow");
						break;
					case RED:
						tCellCol = QBrush("tomato");
						break;
					case BLUE:
						tCellCol = QBrush("lightblue");
						break;
					default:
						tCellCol = QBrush("white");
					}
				resultsTable->item(pIndex,j)->setBackground(tCellCol);
			}
			else{
				if (it->compare("status") == 0){
					WaferComboBox* tComboBox = new WaferComboBox(pIndex, *it, QString::number(tValue, 'd',0), tResultStatus, resultsTable);
					QObject::connect(tComboBox, SIGNAL(getState(unsigned int, std::string, int, QString)), this, SLOT(setChipStatus(unsigned int, std::string, int, QString)));
					addTableItem(pIndex,j,tComboBox);
				}
				else{
					WaferComboBox* tComboBox = new WaferComboBox(pIndex, *it, QString::number(tValue, 'd',0), tResultStatus, resultsTable);
					QObject::connect(tComboBox, SIGNAL(getState(unsigned int, std::string, int, QString)), this, SLOT(setResultStatus(unsigned int, std::string, int, QString)));
					addTableItem(pIndex,j,tComboBox);
				}
			}
		}
		else{
//			WaferComboBox* tComboBox = new WaferComboBox(pIndex, *it, QString("-"), tResultStatus);
//			QObject::connect(tComboBox, SIGNAL(getState(unsigned int, std::string, int)), this, SLOT(setResultStatus(unsigned int, std::string, int)));
//			addTableItem(pIndex,j,tComboBox);
			addTableText(pIndex,j,QString("-"),QBrush("lightgrey"));
		}
		addTableText(_waferAnalysis._waferData.getNchips()+2, j, QString::number((double) _waferAnalysis._waferData.getResultStatistic(*it, GREEN)/(double) _waferAnalysis._waferData.getNchips()*100., 'd',0)+QString("%"), QBrush("lightgreen"));
		addTableText(_waferAnalysis._waferData.getNchips()+3, j, QString::number((double) _waferAnalysis._waferData.getResultStatistic(*it, YELLOW)/(double) _waferAnalysis._waferData.getNchips()*100., 'd',0)+QString("%"), QBrush("yellow"));
		addTableText(_waferAnalysis._waferData.getNchips()+4, j, QString::number((double) _waferAnalysis._waferData.getResultStatistic(*it, RED)/(double) _waferAnalysis._waferData.getNchips()*100., 'd',0)+QString("%"), QBrush("tomato"));
		addTableText(_waferAnalysis._waferData.getNchips()+5, j, QString::number((double) _waferAnalysis._waferData.getResultStatistic(*it, BLUE)/(double) _waferAnalysis._waferData.getNchips()*100., 'd',0)+QString("%"), QBrush("lightblue"));
		addTableText(_waferAnalysis._waferData.getNchips()+6, j, QString::number((double) _waferAnalysis._waferData.getResultStatistic(*it, GREY)/(double) _waferAnalysis._waferData.getNchips()*100., 'd',0)+QString("%"), QBrush("lightgrey"));
	}
	WaferButton* button = new WaferButton(QString(_waferAnalysis._waferData.getFileName(pIndex).c_str()), resultsTable);
	QObject::connect(button, SIGNAL(showViewer(QString)), this, SLOT(startViewer(QString)));
	addTableItem(pIndex,_waferAnalysis._waferData.getNresults()+3, button);
}
void WaferAnalysisPanel::plotTableSummary()
{
	//average and deviation output
	resultsTable->setItem(_waferAnalysis._waferData.getNchips(),0, new QTableWidgetItem(QString("Avr.")));
	resultsTable->setItem(_waferAnalysis._waferData.getNchips()+1,0, new QTableWidgetItem(QString("Std.")));

	//number of chips in the different states output
	addTableText(_waferAnalysis._waferData.getNchips()+2, 0, QString::number(_waferAnalysis._waferData.getNgreenChips()), QBrush("lightgreen"));
	addTableText(_waferAnalysis._waferData.getNchips()+3, 0, QString::number(_waferAnalysis._waferData.getNyellowChips()), QBrush("yellow"));
	addTableText(_waferAnalysis._waferData.getNchips()+4, 0, QString::number(_waferAnalysis._waferData.getNredChips()), QBrush("tomato"));
	addTableText(_waferAnalysis._waferData.getNchips()+5, 0, QString::number(_waferAnalysis._waferData.getNblueChips()), QBrush("lightblue"));
	addTableText(_waferAnalysis._waferData.getNchips()+6, 0, QString::number(_waferAnalysis._waferData.getNgreyChips()), QBrush("lightgrey"));

	int i = 3;
	std::map<std::string, std::pair<double, double> > tAverageValues;
	_waferAnalysis._waferData.getAverageValues(tAverageValues);
	for (std::map<std::string, std::pair<double, double> >::iterator it = tAverageValues.begin(); it != tAverageValues.end(); ++it){
		QTableWidgetItem* tItem = new QTableWidgetItem(QString::number(it->second.first, 'd',0));
		tItem->setTextAlignment(4);
		resultsTable->setItem(_waferAnalysis._waferData.getNchips(),i, tItem);
		tItem = new QTableWidgetItem(QString::number(it->second.second, 'd',0));
		tItem->setTextAlignment(4);
		resultsTable->setItem(_waferAnalysis._waferData.getNchips()+1,i, tItem);
		i++;
	}
	QPushButton* tButton = new QPushButton();
	tButton->setText("Refresh");
	QObject::connect(tButton, SIGNAL(clicked()), this, SLOT(refreshTableSummary()));
	addTableItem(_waferAnalysis._waferData.getNchips(), i,tButton);

}
void WaferAnalysisPanel::plotTableIndex(unsigned int pIndex)
{
	QTableWidgetItem* tItem = new QTableWidgetItem(QString::number(_waferAnalysis._waferData.getChipNumber(pIndex)));	//chip number, NOT index
	QTableWidgetItem* tItem2 = new QTableWidgetItem(QString::number(_waferAnalysis._waferData.getWaferSerialNumber(pIndex)));	//wafer serial number of the chip
	QTableWidgetItem* tItem3 = new QTableWidgetItem(QString(_waferAnalysis._waferData.getWaferName(pIndex).c_str()));	//wafer name of the chip
	tItem->setTextAlignment(4);
	tItem2->setTextAlignment(4);
	tItem3->setTextAlignment(4);
	resultsTable->setItem(pIndex,0, tItem);
	resultsTable->setItem(pIndex,1, tItem2);
	resultsTable->setItem(pIndex,2, tItem3);

	QBrush tCellCol;
	status tChipStatus = GREY;
	_waferAnalysis._waferData.getChipStatus(pIndex, tChipStatus);

	switch(tChipStatus){
		case GREY:
			tCellCol = QBrush("lightgrey");
			break;
		case GREEN:
			tCellCol = QBrush("lightgreen");
			break;
		case YELLOW:
			tCellCol = QBrush("yellow");
			break;
		case RED:
			tCellCol = QBrush("tomato");
			break;
		case SETGREEN:
			tCellCol = QBrush("lightgreen");
			break;
		case SETYELLOW:
			tCellCol = QBrush("yellow");
			break;
		case SETRED:
			tCellCol = QBrush("tomato");
			break;
		case BLUE:
			tCellCol = QBrush("lightblue");
			break;
		default:
			tCellCol = QBrush("white");
	}
	resultsTable->item(pIndex,0)->setBackground(tCellCol);
	resultsTable->item(pIndex,1)->setBackground(QBrush("lightgrey"));
	resultsTable->item(pIndex,2)->setBackground(QBrush("lightgrey"));
}
void WaferAnalysisPanel::refreshTableSummary()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	_waferAnalysis._waferData.AverageResults();
	plotTable();
	QApplication::restoreOverrideCursor();
}

void WaferAnalysisPanel::tryAutoOpenCutFile()
{
	QDir curr = QDir::current();
//	curr.cdUp();
	FILE *cuts = fopen((curr.path()+"/CutConfWafer.txt").toLatin1().data(),"r");
	if(cuts!=0){
		fclose(cuts);
		_waferAnalysis._waferCuts.setCutFileName(QString(curr.path()+"/CutConfWafer.txt").toStdString());
		_waferAnalysis._waferCuts.ReadCutFile();
		std::string tCutFileName = _waferAnalysis._waferCuts.getCutFileName();
		tCutFileName = tCutFileName.substr(tCutFileName.find_last_of("/")+1,tCutFileName.size()-tCutFileName.find_last_of("/"));
		cutsFileName->setText(QString(tCutFileName.c_str()));
		label_cutversion->setText(QString::number(_waferAnalysis._waferCuts.getCutFileVersion()));
		actionAdd_Data->setEnabled(true);
		actionAdd_Data_2Chip->setEnabled(true);
	}
	else{ // failed, so let's check if the DAQ_BASE envir. variable is set
		QString basePath = gSystem->Getenv("DAQ_BASE");
		basePath.replace(QString("\\"),QString("/"));
		cuts = fopen((basePath+"/config/CutConfWafer.txt").toLatin1().data(),"r");
		if(cuts!=0){
			fclose(cuts);
			_waferAnalysis._waferCuts.setCutFileName(QString(basePath+"/config/CutConfWafer.txt").toStdString());
			_waferAnalysis._waferCuts.ReadCutFile();
			label_cutversion->setText(QString::number(_waferAnalysis._waferCuts.getCutFileVersion()));
			std::string tCutFileName = _waferAnalysis._waferCuts.getCutFileName();
			tCutFileName = tCutFileName.substr(tCutFileName.find_last_of("/")+1,tCutFileName.size()-tCutFileName.find_last_of("/"));
			cutsFileName->setText(QString(tCutFileName.c_str()));
			actionAdd_Data->setEnabled(true);
			actionAdd_Data_2Chip->setEnabled(true);
		}
		else{
			actionAdd_Data->setEnabled(false);
			actionAdd_Data_2Chip->setEnabled(false);
			cutsFileName->setText("(open cuts file)");
		}
	}
}

void WaferAnalysisPanel::tryAutoOpenSettingsFile()
{
	QDir curr = QDir::current();
//	curr.cdUp();
	FILE *cuts = fopen((curr.path()+"/SettingsWafer.txt").toLatin1().data(),"r");
	if(cuts!=0){
		fclose(cuts);
		_waferAnalysis._waferSettings.setSettingsFileName(QString(curr.path()+"/SettingsWafer.txt").toStdString());
		_waferAnalysis._waferSettings.ReadSettingsFile();
		std::string tSettingsFileName = _waferAnalysis._waferSettings.getSettingsFileName();
		tSettingsFileName = tSettingsFileName.substr(tSettingsFileName.find_last_of("/")+1,tSettingsFileName.size()-tSettingsFileName.find_last_of("/"));
		settingsFileName->setText(QString(tSettingsFileName.c_str()));
	}
	else{ // failed, so let's check if the DAQ_BASE envir. variable is set
		QString basePath = gSystem->Getenv("DAQ_BASE");
		basePath.replace(QString("\\"),QString("/"));
		cuts = fopen((basePath+"/config/SettingsWafer.txt").toLatin1().data(),"r");
		if(cuts!=0){
			fclose(cuts);
			_waferAnalysis._waferSettings.setSettingsFileName(QString(basePath+"/config/SettingsWafer.txt").toStdString());
			_waferAnalysis._waferSettings.ReadSettingsFile();
			std::string tSettingsFileName = _waferAnalysis._waferSettings.getSettingsFileName();
			tSettingsFileName = tSettingsFileName.substr(tSettingsFileName.find_last_of("/")+1,tSettingsFileName.size()-tSettingsFileName.find_last_of("/"));
			settingsFileName->setText(QString(tSettingsFileName.c_str()));
		}
		else{
			settingsFileName->setText("(open settings file)");
		}
	}
}

void WaferAnalysisPanel::reloadSettings()
{
	_waferAnalysis._waferSettings.ReadSettingsFile();
}

void WaferAnalysisPanel::analyzesStarted()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	actionOpen_Settings->setEnabled(false);
	actionOpen_Cuts->setEnabled(false);
	actionAdd_Data->setEnabled(false);
	actionAdd_Data_2Chip->setEnabled(false);
	actionClear_Data->setEnabled(false);
	actionReload_Settings->setEnabled(false);
	actionReload_Cuts->setEnabled(false);
	actionCreate_Wafer_Plots->setEnabled(false);
	pushButton_Analyze->setEnabled(false);
	//pushButton_Stop->setEnabled(false);
	analysing = true;
}
void WaferAnalysisPanel::analyzesFinished()
{
	QApplication::restoreOverrideCursor();
	actionOpen_Settings->setEnabled(true);
	actionOpen_Cuts->setEnabled(true);
	actionAdd_Data->setEnabled(true);
	actionAdd_Data_2Chip->setEnabled(true);
	actionClear_Data->setEnabled(true);
	pushButton_Analyze->setEnabled(true);
	actionReload_Settings->setEnabled(true);
	actionReload_Cuts->setEnabled(true);
	actionCreate_Wafer_Plots->setEnabled(true);
	//pushButton_Stop->setEnabled(false);
	analysing = false;
}
void WaferAnalysisPanel::increaseProgressBar()
{
	progressBarSetting++;
	progressBar->setValue((int) ((double)(progressBarSetting)/(double)_waferAnalysis._waferData.getNchips()*100.));
}

void WaferAnalysisPanel::setChipPlotOutput()
{
	_waferAnalysis.setPlotOutput(actionCreate_Chip_Plots->isChecked());
}
void WaferAnalysisPanel::resetChipStatuse()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	_waferAnalysis._waferData.resetChipStatuses();
	plotTable();
	QApplication::restoreOverrideCursor();
}

void WaferAnalysisPanel::showInfoWindow()
{
	QMessageBox tMsgBox;
	QString tPrgVers(PrgVers);
	QString tPrgRev(PrgRev);
	QString tPrgAut(PrgAut);
	QString tPrgCom(PrgCom);
	tMsgBox.setText("Version: "+tPrgVers+", Revision: "+tPrgRev+"\nAuthor: "+tPrgAut+"\n\n"+tPrgCom);
	tMsgBox.exec();
}

void WaferAnalysisPanel::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}

bool WaferAnalysisPanel::checkForSettingsCutsFile()
{
	FILE *cuts = fopen(_waferAnalysis._waferCuts.getCutFileName().c_str(),"r");
	FILE *settings = fopen(_waferAnalysis._waferSettings.getSettingsFileName().c_str(),"r");
	if(cuts == 0){
		QMessageBox::warning(this,"Cuts file not found","Can't find cut file "+QString(_waferAnalysis._waferCuts.getCutFileName().c_str())+".\nPlease open needed cut file first.");
		if(settings != 0) fclose(settings);
		return false;
	}
	if(settings == 0){
		QMessageBox::warning(this,"Settings file not found","Can't find settings file "+QString(_waferAnalysis._waferSettings.getSettingsFileName().c_str())+".\nPlease open needed settings file first.");
		if(cuts != 0) fclose(cuts);
		return false;
	}
	fclose(cuts);
	fclose(settings);
	return true;
}

void WaferAnalysisPanel::dropEvent(QDropEvent *event)
{
	const QMimeData* mimeData = event->mimeData();

	QApplication::setOverrideCursor(Qt::WaitCursor);

	if (mimeData->hasUrls())
	{
		QStringList pathList;
		QList<QUrl> urlList = mimeData->urls();

		for (int i = 0; i < urlList.size(); ++i){	// extract the local paths of the files
			std::string tFileName = (urlList.at(i).toLocalFile().toLocal8Bit().constData());
			std::string tFileNameNoFolder = tFileName.substr(tFileName.find_last_of("/")+1, tFileName.size());
			std::string tFileType = tFileName.substr(tFileName.find_last_of(".")+1, tFileName.npos);

			if(tFileType.compare("waprj") == 0){
				if(_waferAnalysis._waferData.getNwafers() == 0)
					openProject(tFileName);
				else
					_waferAnalysis.addProjectData(tFileName);
			}
			else if(tFileType.compare("root")  == 0){
				if(_waferAnalysis._waferData.getNwafers() > 0 && checkForSettingsCutsFile())
					_waferAnalysis._waferData.addFileGroup(tFileName);
				else{
					if(checkForSettingsCutsFile())
						_waferAnalysis._waferData.openFileGroup(tFileName);
				}
			}
			else if(tFileType.compare("txt")  == 0){
				if(tFileNameNoFolder.find("Conf") != std::string::npos || tFileNameNoFolder.find("conf") != std::string::npos){
					_waferAnalysis._waferCuts.setCutFileName(tFileName);
					_waferAnalysis._waferCuts.ReadCutFile();
					label_cutversion->setText(QString::number(_waferAnalysis._waferCuts.getCutFileVersion()));
					std::string tCutFileName = _waferAnalysis._waferCuts.getCutFileName();
					tCutFileName = tCutFileName.substr(tCutFileName.find_last_of("/")+1,tCutFileName.size()-tCutFileName.find_last_of("/"));
					cutsFileName->setText(QString(tCutFileName.c_str()));
				}
				else if(tFileNameNoFolder.find("setting") != std::string::npos || tFileNameNoFolder.find("Setting") != std::string::npos){
					_waferAnalysis._waferSettings.setSettingsFileName(tFileName);
					_waferAnalysis._waferSettings.ReadSettingsFile();
					std::string tSettingsFileName = _waferAnalysis._waferSettings.getSettingsFileName();
					tSettingsFileName = tSettingsFileName.substr(tSettingsFileName.find_last_of("/")+1,tSettingsFileName.size()-tSettingsFileName.find_last_of("/"));
					settingsFileName->setText(QString(tSettingsFileName.c_str()));
				}
				else
					QMessageBox::warning(this,"Text file","Do not know if this is a settings or cut file.\nPlease put 'cut'/'settings' word in the file name.");
			}
			else
				QMessageBox::warning(this,"Unknown file type","Do not know what to do with a '"+QString(tFileType.c_str())+"' file.\nPlease open a root or Wafer Analysis project file.");
			progressBar->setValue((int) ((double)(i)/(double) urlList.size()*100));
		}
	}
  if(_waferAnalysis._waferData.getNchips()>0)
	  waferName->setText(_waferAnalysis._waferData.getWaferName().c_str());
	label_chipNr->setText(QString::number(_waferAnalysis._waferData.getNchips()));
	plotTable();
	QApplication::restoreOverrideCursor();
	progressBar->setValue(0);
	progressBarSetting = 0;
}
