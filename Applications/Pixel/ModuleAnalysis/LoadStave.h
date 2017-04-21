#ifndef LOAD_STAVE
#define LOAD_STAVE

#include "ui_LoadStaveBase.h"

class LoadStave : public QDialog, public Ui::LoadStaveBase {

 Q_OBJECT

 public:
    LoadStave( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~LoadStave();

 public slots:
    void browseRootPath();
    void browseOutPath(); 
    void checkPaths();
    void loadPhase();
    void typeChanged();
    void getModuleList();
};

#endif // LOAD_STAVE
