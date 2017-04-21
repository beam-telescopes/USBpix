#ifndef WAFERANALYSISPANEL_H
#define WAFERANALYSISPANEL_H

#include <QThread>
#include <QDragEnterEvent>

#include "ui_WaferAnalysisPanel.h"
#include "WaferAnalysis.h"
#include "WaferWorker.h"

#define __MAXTABLEENTRIES 36000	//defines the maximum number entries in the table (speed reasons), if the number is reached the actual row is still fully plotted

#define PrgVers "RELEASE 1.11"	//program version
#define PrgRev "11000"	//program revision
#define PrgAut "David-Leon Pohl\npohl@physik.uni-bonn.de"	//program author
#define PrgCom "SILAB Universität Bonn"	//program company

class QString;
class QBrush;
class QButton;
class DataViewer;

class WaferAnalysisPanel : public QMainWindow, public Ui::WaferAnalysisPanel {

	Q_OBJECT

	public:
		WaferAnalysisPanel(QWidget * parent = 0, Qt::WindowFlags flags = 0);
		 ~WaferAnalysisPanel();

	public slots:
		void addData();
		void exportResultsToXML();
		void exportDataTextFile();
		void writeRankingData();
		void saveProject();
		void openProject();
		void openProject(std::string tProjectFileName);
		void addProjectData();
		void clearData();
		void clearAnalyzedData();
		void startViewer(QString);
		void setResultStatus(unsigned int pChipIndex, std::string pResultName, int pResultStatus, QString pValue=0);
		void setChipStatus(unsigned int pChipIndex, std::string pResultName, int pChipStatus, QString pValue="0");
		void exitAll();
		void browseCuts();
		void browseSettings();
		void analyzeData();
		void createWaferResult();
		void refreshResultCuts();
		void refreshTableSummary();
		void reloadSettings();
		void setChipPlotOutput();
		void resetChipStatuse();
		void setDebugOutput();
		void setInfoOutput();
		void setWarningOutput();
		void setErrorOutput();
		void setBugReport();
		void setWaferSerialNumber(int pSerialNumber);
		void increaseProgressBar();
		void analyzesStarted();
		void analyzesFinished();
		void showInfoWindow();
		//drag and drop support
		void dropEvent(QDropEvent *event);
		void dragEnterEvent(QDragEnterEvent *event);

	private:
		void addTableText(unsigned int pX, unsigned int pY, QString pQstring, QBrush pQbrush);
		void tryAutoOpenCutFile();					//tries to automatically guess the path of the cut file
		void tryAutoOpenSettingsFile();				//tries to automatically guess the path of the scan list file
		void addTableItem(unsigned int pX, unsigned int pY, QWidget* pWidget);
		void plotTable();							//plots the hole table (calls all plot table functions)
		void plotTableFrame(); 						//plots the headers and the needed number of rows and cols
		void plotTableRow(unsigned int pIndex);		//plots the row of the table (pIndex = [0, nChips[)
		void plotTableIndex(unsigned int pIndex);	//plots the first two columnn of the table (the chip Nr and Wafer SN)
		void plotTableSummary();					//plots the summary of the data at the end of the column
		bool checkForSettingsCutsFile();			//checks if the cut and settings file are available and shows error window

		double progressBarSetting;
		DataViewer* m_dataView;
		WaferAnalysis _waferAnalysis;
		bool analysing;
		bool _twoChip;								//is set to true to activate that new added data is treated as a 2 chip module

};
#endif // WAFERANALYSISPANEL_H
