#ifndef MODLIST
#define MODLIST

#include "ui_ModList.h"
#include <QTreeWidget>

#include <vector>
#include <map>

class QString;
class QPoint;
class QTime;

class MainPanel;
class STControlEngine;
class STRodCrate;
class STPixModuleGroup; 

class ModListItem : public QTreeWidgetItem{

 friend class ModList;

 public:
  enum ContentType {
    tnone,
    tcrate,
    tgroup,
    tROD,
    tBOC,
    tmodule,
    tUSB,
    tCTRL,
    tDCS,
    tDCSC
  };
  // ModListItem is a crate
  ModListItem(STControlEngine &engine_in, QTreeWidget *parent, const char *name);
  ModListItem(STControlEngine &engine_in, QTreeWidgetItem *parent, ContentType type_in, int id_in);
  ModListItem(STControlEngine &engine_in, QTreeWidget *parent, STRodCrate *crate);

  STRodCrate *getCrate();
  int crateId(){return m_crateId;};
  int grpId(){return m_grpId;};
  int modId(){return m_modId;};
  ContentType getType(){return m_type;};
  ~ModListItem();
  /** Write property of bool enabled. */
  virtual void setEnabled( const bool& _newVal );
  /** Read property of bool enabled. */
  virtual const bool& isEnabled();
  /** Toggles the enable flag */
  virtual void toggleEnable();
  /** returns group/ROD-level parent (null if no such parent) **/
  ModListItem* parentGroup();
  /** returns crate-level parent (null if no such parent) **/
  ModListItem* parentCrate();
  void up();
  void down();

 protected:
  /** type of referenced object */
  ContentType        m_type;
  int m_crateId, m_grpId, m_modId;
  bool m_enabled;
  STControlEngine &m_engine;
};

class ModList : public QWidget, public Ui::ModList {
  
  Q_OBJECT

 friend class MainPanel;
 friend class ModListItem;

 enum MLmode{DAQList=0, DCSList};

 public:
 ModList( STControlEngine &engine_in, QWidget* parent = 0, Qt::WindowFlags fl = 0, MLmode mode=DAQList);
  ~ModList();
/*   ModListItem* loadModuleGroup(PixLib::PixConfDBInterface *DB, SctPixelRod::VmeInterface *vme); */
/*   void deleteModuleGroup(ModListItem *); */
  /** Return the currently selected Module as given by currentItem() */
  virtual ModListItem * currentModule() const;
  /** Return the currently selected PixController as given by currentItem() */
  ModListItem * currentROD() const;
  /** Return the currently selected BOC as given by currentItem() */
  ModListItem * currentBOC() const;
  /** Return the currently selected Group as given by currentItem(), or its parent Group */
  virtual ModListItem * currentGroup() const;
  /** Return the currently selected crate as given by currentItem() */
  virtual ModListItem * currentCrate() const;
 protected:
  void emitStatusChanged(){emit statusChanged();};

 public slots:
   void editModule(QTreeWidgetItem *item, int);
  void editModule(QTreeWidgetItem *in_item, bool doubleClick);
  void rightClick(const QPoint &);
  void toggleEnable(QTreeWidgetItem *item);
  /** Update the ListView */
  void updateView();
  void updateViewDAQ();
  void updateViewDCS();
  /** update the status of items in the ListView */
  void changeStatus();
  void allOff(){allSwitch(false);};
  void allOn(){allSwitch(true);};
  void allSwitch(bool on);
  void editClosed(int);
  void saveFromEdit(int, int);
  void setBusy(){m_scanBusy=true;};
  void setIdle(){m_scanBusy=false;};
  void checkStatus();
  void itemSelected(QTreeWidgetItem *item, int col);
  void itemSelected(QTreeWidgetItem *item){itemSelected(item, 0);};
  void upCurrItem();
  void downCurrItem();

 signals:
  // emitted after updating the module list
  void listUpdated(QTreeWidgetItem*);
  /** emitted after changing the status of items in the ListView */
  void statusChanged();

 private:
  STControlEngine &m_engine;
  MLmode m_mode;
  std::map< int, std::vector<int>* > m_edtItem;
  ModListItem *m_selItem;
  bool m_scanBusy;
  QTime *m_lastDcsRead;

};
#endif // MODLIST

