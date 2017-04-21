#ifndef CORRELSEL_H
#define CORRELSEL_H

#include "ui_CorrelSelBase.h"
#include <vector>
#include <map>

class ModItem;

/*! panel for selection of two data items to be scatter-plotted
 *  to check the correlation
 * @authors Joern Grosse-Knetter <jgrosse@physik.uni-bonn.de>
 */

class CorrelSel : public QDialog, public Ui::CorrelSelBase
{
    Q_OBJECT

 public:
    CorrelSel( std::vector<ModItem *> moditems, QWidget* parent = 0, const char* name = 0, 
	       bool modal = FALSE, Qt::WFlags fl = 0);
    ~CorrelSel();
    
    std::vector<ModItem*> m_inlistA;
    std::vector<ModItem*> m_inlistB;
    std::vector<int> m_plotidA;
    std::vector<int> m_plotidB;

 public slots:
   void DisplayFirst(){DisplayPlotItems(1);};
   void DisplaySecond(){DisplayPlotItems(2);};
   void CheckForRMS();

 private:
    std::vector<ModItem*> m_mod_list;
    void FillPlotList(QComboBox *currList, ModItem *thisit, std::vector<ModItem*> &inlist, std::vector<int> &plotid);
    void DisplayPlotItems(int whichMod=0);
    
};

#endif // CORRELSEL_H
