#ifndef GROUPEDITOR_H
#define GROUPEDITOR_H

#include "ui_GroupEditor.h"
#include <map>
#include <vector>

//class STPixModuleGroup;
class STControlEngine;
class STRodCrate;
class optionsPanel;
namespace PixLib{
  class PixModule;
}
class QAction;

class GroupEditor : public QDialog, public Ui::GroupEditor
{
    Q_OBJECT

 public:
  GroupEditor( STControlEngine &in_engine, int iCrate, int iGrp, QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~GroupEditor();

    std::vector<int> editedModules(){return m_edited;};

 public slots:
    void saveConfig();
    void setupTable();
    void setEdited(int,int);
    void copyToCB();
    void pasteFromCB();
    void setAllLinks();

 private:
    STControlEngine &m_engine;
    STRodCrate *m_crate;
    int m_grpId;
    std::vector<int> m_edited;
    QAction *m_pasteAction, *m_copyAction;
    optionsPanel *m_op;

};

#endif // GROUPEDITOR_H
