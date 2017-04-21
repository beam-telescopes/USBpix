#ifndef MULTILOAD_H
#define MULTILOAD_H

#include "ui_MultiLoadBase.h"
#include <qstring.h>
#include <vector>

class Q3Http;
class PDBForm;
class TopWin;
class LoginPanel;
class CDatabaseCom;
class LoadStave;
class StdListChoice;

class MultiLoad : public QDialog, public Ui::MultiLoadBase
{
    Q_OBJECT

public:
    MultiLoad( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~MultiLoad();

    PDBForm *m_PDBForm;
    LoginPanel *m_lp;
    std::vector<bool> m_loadStd;

public slots:
    void addModule();
    void addModule(const char *mname, const char *staveSN=0, int pos=-1);
    void addRoot();
    void browseModPath();
    void showPDB();
    void meastypeChanged();
    void removeCell();
    void currentCell(int,int);
    void getPDBLogin();
    void addStave();
    void moveEntryUp();
    void moveEntryDown();
    void checkPDBpath();
    void addModuleList();
    void restrictStdList();
    void browsePlotsPath();

private:
    TopWin *m_tw;
    int m_currCol, m_currRow;
    LoadStave *m_lst;
    StdListChoice *m_slc;
};

#endif // MULTILOAD_H
