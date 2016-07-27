#include <vector>
#include <qlistview.h>
#include <qstring.h>
#include "Module.h"
#include "ModItem.h"

Moditem::Moditem(QListView *parent, QString l1, QString l2, Modul *mod)
  :QListViewItem(parent,l1,mod->GetPassed(),l2)
{
  m_modul =mod;
}

Moditem::~Moditem(){
  delete m_modul;
}

void Moditem::updateStatus()
{
  //setText(1,m_modul->GetPassed());
}
// inline Modul *Moditem::GetModul()
// {
//   return m_modul;
// }
