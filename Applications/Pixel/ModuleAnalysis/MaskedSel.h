#ifndef MASKEDSEL_H
#define MASKEDSEL_H

#include "ui_MaskedSelBase.h"
#include <vector>

class ModItem;

/*! panel for selection of data items to be selected by
 *  application of an existing mask stored in ModuleAnalysis
 * @authors Giorgio Cardarelli <giorgio@pcteor1.mi.infn.it>
 */

class MaskedSel : public QDialog, public Ui::MaskedSelBase
{
    Q_OBJECT

 public:
    MaskedSel( ModItem* thisis, QWidget* parent = 0, const char* name = 0, 
	       bool modal = FALSE, Qt::WFlags fl = 0, int mode=0);
    ~MaskedSel();
    
    ModItem *callit;
    std::vector<ModItem*> alist, blist;
    std::vector<int> aplotid, bplotid;
    
 private:
    void fillMasked();
    void fillBadChan();
};

#endif // MASKEDSEL_H
