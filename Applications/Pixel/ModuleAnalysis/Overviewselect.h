#ifndef OVERVIEWSELECT_H
#define OVERVIEWSELECT_H

#include "ui_Overviewselectbase.h"

#include "FileTypes.h"

#include <map>
#include <vector>

class ModItem;

class TopWin;

class OverviewSelect : public QDialog, public Ui::OverviewSelectBase
{
	Q_OBJECT

public:
	OverviewSelect(QWidget* parent=0, const char* name=0, bool modal=FALSE, Qt::WFlags fl=0);
	~OverviewSelect();

	void PlotItemmap();

public slots:
	void DoPlot();
	void GotoSystestMode();
	void SelectDataset();
private:
	TopWin *m_tw;

};
#endif // OVERVIEWSELECT_H

