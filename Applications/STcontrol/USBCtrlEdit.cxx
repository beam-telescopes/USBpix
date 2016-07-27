#include "USBCtrlEdit.h"
#include <QFileDialog>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <GeneralDBfunctions.h>
#include <QMessageBox>
#include <QComboBox>
#include <math.h>

USBCtrlEdit::USBCtrlEdit(PixLib::Config &in_cfg, QWidget* parent, Qt::WindowFlags fl): 
  QDialog(parent, fl), 
  m_config(in_cfg),
  m_overrideEnableDemux(false)
{
  setupUi(this);
  AdapterCardFlavourComboBoxStateChanged();
  connect(auxClkDiv, SIGNAL(valueChanged(int)), this, SLOT(calcAuxFreq(int)));
  connect(AdapterCardFlavourComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(AdapterCardFlavourComboBoxStateChanged()));
  
  m_recfgNeeded = false;

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["BoardID"].name()!="__TrashConfObj__"){
    boardID->setValue(((ConfInt&)m_config["general"]["BoardID"]).getValue());
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["Board2ID"].name()!="__TrashConfObj__"){
    int b2ID = ((ConfInt&)m_config["general"]["Board2ID"]).getValue();
    if(b2ID<0){
      //horizontalSpacer2b->hide();
      board2ID->hide();
      board2Label->hide();
    } else{
      board2ID->setMinimum(0);
      boardID->setMinimum(0);
    }
    board2ID->setValue(b2ID);
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["FirmwareFile"].name()!="__TrashConfObj__"){
    FPGA_File->setText(QString(((ConfString&)m_config["general"]["FirmwareFile"]).value().c_str()));
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["uCFirmwareFile"].name()!="__TrashConfObj__"){
    uC_File->setText(QString(((ConfString&)m_config["general"]["uCFirmwareFile"]).value().c_str()));
  }
//   if(m_config["general"].name()!="__TrashConfGroup__" && 
//      m_config["general"]["RegSyncDel"].name()!="__TrashConfObj__"){
//     RegSyncDelSB->setValue(45*((ConfInt&)m_config["general"]["RegSyncDel"]).getValue());
//   }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["CableLength"].name()!="__TrashConfObj__"){
    CableDelSB->setValue(((ConfInt&)m_config["general"]["CableLength"]).getValue());
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["TriggerReplication"].name()!="__TrashConfObj__"){
    std::map<std::string, int> trList = ((ConfList&)m_config["general"]["TriggerReplication"]).symbols();
    for(std::map<std::string, int>::iterator it=trList.begin();it!=trList.end(); it++)
      trModeBox->insertItem(it->second, it->first.c_str());
    trModeBox->setCurrentIndex(((ConfList&)m_config["general"]["TriggerReplication"]).getValue());
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["setSelCMD"].name()!="__TrashConfObj__"){
    SetSelCMDBox->setChecked(((ConfBool&)m_config["general"]["setSelCMD"]).value());
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["slowCtrlMode"].name()!="__TrashConfObj__"){
    SetSelSlowCtrlBox->setChecked(((ConfBool&)m_config["general"]["slowCtrlMode"]).value());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["enableCmdLvl1"].name()!="__TrashConfObj__"){
    enCMDLvl1Box->setChecked(((ConfBool&)m_config["general"]["enableCmdLvl1"]).value());
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["enableRJ45"].name()!="__TrashConfObj__"){
    enableRJ45->setChecked(((ConfBool&)m_config["general"]["enableRJ45"]).value());
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["IoMuxSel"].name()!="__TrashConfObj__"){
    int sel = ((ConfInt&)m_config["general"]["IoMuxSel"]).getValue();
    IoMuxSel2->setChecked(sel&4);
    IoMuxSel1->setChecked(sel&2);
    IoMuxSel0->setChecked(sel&1);
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["IoMuxIn"].name()!="__TrashConfObj__"){
    int inval = ((ConfInt&)m_config["general"]["IoMuxIn"]).getValue();
    IoMuxIn3->setChecked(inval&8);
    IoMuxIn2->setChecked(inval&4);
    IoMuxIn1->setChecked(inval&2);
    IoMuxIn0->setChecked(inval&1);
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["TX2Signal"].name()!="__TrashConfObj__"){
    int inval = ((ConfInt&)m_config["general"]["TX2Signal"]).getValue();
    TX2Signal->setCurrentIndex(inval);
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["AuxClkDiv"].name()!="__TrashConfObj__"){
    auxClkDiv->setValue(((ConfInt&)m_config["general"]["AuxClkDiv"]).getValue());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["IrefPads"].name()!="__TrashConfObj__"){
    irefPadsBox->setValue(((ConfInt&)m_config["general"]["IrefPads"]).getValue());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["invIrefPads"].name()!="__TrashConfObj__"){
    invIrefPadsBox->setChecked(((ConfBool&)m_config["general"]["invIrefPads"]).value());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
	  m_config["general"]["FEToRead"].name()!="__TrashConfObj__"){
		 ReadFENumberSpinBox->setValue(((ConfInt&)m_config["general"]["FEToRead"]).getValue());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["enaManCode"].name()!="__TrashConfObj__"){
    ManCodeEna->setChecked(((ConfBool&)m_config["general"]["enaManCode"]).value());
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["manCodePhase"].name()!="__TrashConfObj__"){
    ManPhaseBox->setValue(((ConfInt&)m_config["general"]["manCodePhase"]).getValue());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelsInput0"].name()!="__TrashConfObj__"){
    roch0inp->setCurrentIndex(((ConfInt&)m_config["general"]["readoutChannelsInput0"]).getValue());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelsInput1"].name()!="__TrashConfObj__"){
    roch1inp->setCurrentIndex(((ConfInt&)m_config["general"]["readoutChannelsInput1"]).getValue());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelsInput2"].name()!="__TrashConfObj__"){
    roch2inp->setCurrentIndex(((ConfInt&)m_config["general"]["readoutChannelsInput2"]).getValue());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelsInput3"].name()!="__TrashConfObj__"){
    roch3inp->setCurrentIndex(((ConfInt&)m_config["general"]["readoutChannelsInput3"]).getValue());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["inputDivider0"].name()!="__TrashConfObj__"){
    inp0div->setCurrentIndex(((ConfInt&)m_config["general"]["inputDivider0"]).getValue());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["inputDivider1"].name()!="__TrashConfObj__"){
    inp1div->setCurrentIndex(((ConfInt&)m_config["general"]["inputDivider1"]).getValue());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["inputDivider2"].name()!="__TrashConfObj__"){
    inp2div->setCurrentIndex(((ConfInt&)m_config["general"]["inputDivider2"]).getValue());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["inputDivider3"].name()!="__TrashConfObj__"){
    inp3div->setCurrentIndex(((ConfInt&)m_config["general"]["inputDivider3"]).getValue());
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["MultiChipWithSingleBoard"].name()!="__TrashConfObj__"){
    biMultiRadioButton->setChecked(((ConfBool&)m_config["general"]["MultiChipWithSingleBoard"]).value());
  }

  // must be last one read since it affects enable state of others
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["AdapterCardFlavor"].name()!="__TrashConfObj__"){
    AdapterCardFlavourComboBox->setCurrentIndex(((ConfInt&)m_config["general"]["AdapterCardFlavor"]).getValue());
  }
  
  m_overrideEnableDemux = false;
  if(m_config["general"].name()!="__TrashConfGroup__" &&
      m_config["general"]["OverrideEnableDemux"].name()!="__TrashConfObj__"){
    m_overrideEnableDemux = ((ConfBool&)m_config["general"]["OverrideEnableDemux"]).value();
  }
  
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelReadsChip0"].name()!="__TrashConfObj__"){
    roch0assoc->setCurrentIndex(((ConfInt&)m_config["general"]["readoutChannelReadsChip0"]).getValue()+2);
  }
  
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelReadsChip1"].name()!="__TrashConfObj__"){
    roch1assoc->setCurrentIndex(((ConfInt&)m_config["general"]["readoutChannelReadsChip1"]).getValue()+2);
  }
  
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelReadsChip2"].name()!="__TrashConfObj__"){
    roch2assoc->setCurrentIndex(((ConfInt&)m_config["general"]["readoutChannelReadsChip2"]).getValue()+2);
  }
  
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelReadsChip3"].name()!="__TrashConfObj__"){
    roch3assoc->setCurrentIndex(((ConfInt&)m_config["general"]["readoutChannelReadsChip3"]).getValue()+2);
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["EnableSCAStrobePin"].name()!="__TrashConfObj__"){
    enSCAStrobePin->setChecked(((ConfBool&)m_config["general"]["EnableSCAStrobePin"]).value());
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["EnableGPACStrobePin"].name()!="__TrashConfObj__"){
    enGPACStrobePin->setChecked(((ConfBool&)m_config["general"]["EnableGPACStrobePin"]).value());
  }

  connect(browseButton, SIGNAL(clicked()), this, SLOT(browseFPGA()));
  connect(browseButtonUC, SIGNAL(clicked()), this, SLOT(browseMicroCtrl()));
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(save()));

  AdapterCardFlavourComboBoxStateChanged();
}
USBCtrlEdit::~USBCtrlEdit(){
}
void USBCtrlEdit::browseFPGA(){
  QString qpath = QString::null;
  if(FPGA_File->text()!="") qpath = FPGA_File->text();
  QFileDialog fdia(this, "Specify name of FPGA firmware file", qpath,"FPGA firmware file (*.bit);;Any file (*.*)");
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setFileMode(QFileDialog::ExistingFile);
  if(fdia.exec() == QDialog::Accepted){
    qpath = fdia.selectedFiles().first();
    qpath.replace("\\", "/");
    FPGA_File->setText(qpath);
  }
}
void USBCtrlEdit::browseMicroCtrl(){
  QString qpath = QString::null;
  if(uC_File->text()!="") qpath = uC_File->text();
  QFileDialog fdia(this, "Specify name of uC firmware file", qpath,"uC firmware file (*.bix);;Any file (*.*)");
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setFileMode(QFileDialog::ExistingFile);
  if(fdia.exec() == QDialog::Accepted){
    qpath = fdia.selectedFiles().first();
    qpath.replace("\\", "/");
    uC_File->setText(qpath);
  }
}

bool USBCtrlEdit::check_setup()
{
  const int roch_count = 4;
  std::vector<int> used_chips(roch0assoc->count());
  used_chips[roch0assoc->currentIndex()] += 1;
  used_chips[roch1assoc->currentIndex()] += 1;
  used_chips[roch2assoc->currentIndex()] += 1;
  used_chips[roch3assoc->currentIndex()] += 1;

  int c = 0;
  for (std::vector<int>::iterator it = used_chips.begin();
      used_chips.end() != it; it++)
  {
    if ((used_chips.begin() + 2) <= it)
    {
      if ((*it) > 0)
      {
        c++;
      }
    }
    else
    {
      c += (*it);
    }
  }

  if (c != roch_count)
  {
    QMessageBox::warning(this, "Invalid readout channel configuration", 
        "Each chip must be uniquely assigned to one read out channel.");
    return false;
  }

  return true;
}

void USBCtrlEdit::save(){
  if (!check_setup())
  {
    return;
  }
 
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["BoardID"].name()!="__TrashConfObj__"){
    int oldV, newV = boardID->value();
    oldV = ((ConfInt&)m_config["general"]["BoardID"]).getValue();
    if(oldV!=newV){
      ((ConfInt&)m_config["general"]["BoardID"]).setValue(newV);
      m_recfgNeeded = true;
    }
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["Board2ID"].name()!="__TrashConfObj__"){
    int oldV, newV = board2ID->value();
    oldV = ((ConfInt&)m_config["general"]["Board2ID"]).getValue();
    if(oldV!=newV){
      ((ConfInt&)m_config["general"]["Board2ID"]).setValue(newV);
      m_recfgNeeded = true;
    }
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["FirmwareFile"].name()!="__TrashConfObj__"){
    std::string oldV, newV = FPGA_File->text().toLatin1().data();
    oldV = ((ConfString&)m_config["general"]["FirmwareFile"]).value();
    if(oldV!=newV){
      ((ConfString&)m_config["general"]["FirmwareFile"]).m_value = newV;
      m_recfgNeeded = true;
    }
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["uCFirmwareFile"].name()!="__TrashConfObj__"){
    std::string oldV, newV = uC_File->text().toLatin1().data();
    oldV = ((ConfString&)m_config["general"]["uCFirmwareFile"]).value();
    if(oldV!=newV){
      ((ConfString&)m_config["general"]["uCFirmwareFile"]).m_value = newV;
      m_recfgNeeded = true;
    }
  }
//   if(m_config["general"].name()!="__TrashConfGroup__" && 
//      m_config["general"]["RegSyncDel"].name()!="__TrashConfObj__"){
//     int oldV, newV = RegSyncDelSB->value();
//     oldV = ((ConfInt&)m_config["general"]["RegSyncDel"]).getValue();
//     if(oldV!=newV){
//       ((ConfInt&)m_config["general"]["RegSyncDel"]).setValue(newV);
//       m_recfgNeeded = true;
//     }
//   }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["CableLength"].name()!="__TrashConfObj__"){
    int oldV, newV = CableDelSB->value();
    oldV = ((ConfInt&)m_config["general"]["CableLength"]).getValue();
    if(oldV!=newV){
      ((ConfInt&)m_config["general"]["CableLength"]).setValue(newV);
    }
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["TriggerReplication"].name()!="__TrashConfObj__"){
    int oldV, newV = trModeBox->currentIndex();
    oldV = ((ConfInt&)m_config["general"]["TriggerReplication"]).getValue();
    if(oldV!=newV){
      ((ConfInt&)m_config["general"]["TriggerReplication"]).setValue(newV);
    }
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["setSelCMD"].name()!="__TrashConfObj__"){
    bool oldV, newV = SetSelCMDBox->isChecked();
    oldV = ((ConfBool&)m_config["general"]["setSelCMD"]).value();
    if(oldV!=newV){
      ((ConfBool&)m_config["general"]["setSelCMD"]).m_value = newV;
    }
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["slowCtrlMode"].name()!="__TrashConfObj__"){
    bool oldV, newV = SetSelSlowCtrlBox->isChecked();
    oldV = ((ConfBool&)m_config["general"]["slowCtrlMode"]).value();
    if(oldV!=newV){
      ((ConfBool&)m_config["general"]["slowCtrlMode"]).m_value = newV;
    }
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["enableCmdLvl1"].name()!="__TrashConfObj__"){
    bool oldV, newV = enCMDLvl1Box->isChecked();
    oldV = ((ConfBool&)m_config["general"]["enableCmdLvl1"]).value();
    if(oldV!=newV){
      ((ConfBool&)m_config["general"]["enableCmdLvl1"]).m_value = newV;
    }
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["enableRJ45"].name()!="__TrashConfObj__"){
    ((ConfBool&)m_config["general"]["enableRJ45"]).m_value = enableRJ45->isChecked();
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["IoMuxSel"].name()!="__TrashConfObj__"){
    int sel = (((int)IoMuxSel2->isChecked())<<2) + (((int)IoMuxSel1->isChecked())<<1) + (((int)IoMuxSel0->isChecked()));
    ((ConfInt&)m_config["general"]["IoMuxSel"]).setValue(sel);
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["IoMuxIn"].name()!="__TrashConfObj__"){
    int inval = (((int)IoMuxIn3->isChecked())<<3) + (((int)IoMuxIn2->isChecked())<<2) + 
      (((int)IoMuxIn1->isChecked())<<1) + (((int)IoMuxIn0->isChecked()));
    ((ConfInt&)m_config["general"]["IoMuxIn"]).setValue(inval);
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["AuxClkDiv"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["AuxClkDiv"]).setValue(auxClkDiv->value());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["IrefPads"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["IrefPads"]).setValue(irefPadsBox->value());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["invIrefPads"].name()!="__TrashConfObj__"){
    ((ConfBool&)m_config["general"]["invIrefPads"]).m_value = invIrefPadsBox->isChecked();
  }
  
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["EnableGPACStrobePin"].name()!="__TrashConfObj__"){
    ((ConfBool&)m_config["general"]["EnableGPACStrobePin"]).m_value = enGPACStrobePin->isChecked();
  }
  
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["EnableSCAStrobePin"].name()!="__TrashConfObj__"){
    ((ConfBool&)m_config["general"]["EnableSCAStrobePin"]).m_value = enSCAStrobePin->isChecked();
  }


  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["AdapterCardFlavor"].name()!="__TrashConfObj__"){
    int oldV, newV = AdapterCardFlavourComboBox->currentIndex();
    oldV = ((ConfInt&)m_config["general"]["AdapterCardFlavor"]).getValue();
    if(oldV!=newV){
      ((ConfInt&)m_config["general"]["AdapterCardFlavor"]).setValue(newV);
    }
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["TX2Signal"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["TX2Signal"]).setValue(TX2Signal->currentIndex());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
	  m_config["general"]["FEToRead"].name()!="__TrashConfObj__"){
		  int oldV, newV = ReadFENumberSpinBox->value();
		  oldV = ((ConfInt&)m_config["general"]["FEToRead"]).getValue();
		  if(oldV!=newV){
			((ConfInt&)m_config["general"]["FEToRead"]).setValue(newV);
			}
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["enaManCode"].name()!="__TrashConfObj__"){
    ((ConfBool&)m_config["general"]["enaManCode"]).m_value = ManCodeEna->isChecked();
  }
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["manCodePhase"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["manCodePhase"]).setValue(ManPhaseBox->value());
  }
  
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelsInput0"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["readoutChannelsInput0"]).setValue(roch0inp->currentIndex());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelsInput1"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["readoutChannelsInput1"]).setValue(roch1inp->currentIndex());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelsInput2"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["readoutChannelsInput2"]).setValue(roch2inp->currentIndex());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelsInput3"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["readoutChannelsInput3"]).setValue(roch3inp->currentIndex());
  }
  
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["inputDivider0"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["inputDivider0"]).setValue(inp0div->currentIndex());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["inputDivider1"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["inputDivider1"]).setValue(inp1div->currentIndex());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["inputDivider2"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["inputDivider2"]).setValue(inp2div->currentIndex());
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["inputDivider3"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["inputDivider3"]).setValue(inp3div->currentIndex());
  }

  
  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelReadsChip0"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["readoutChannelReadsChip0"]).setValue(roch0assoc->currentIndex()-2);
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelReadsChip1"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["readoutChannelReadsChip1"]).setValue(roch1assoc->currentIndex()-2);
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelReadsChip2"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["readoutChannelReadsChip2"]).setValue(roch2assoc->currentIndex()-2);
  }

  if(m_config["general"].name()!="__TrashConfGroup__" && 
     m_config["general"]["readoutChannelReadsChip3"].name()!="__TrashConfObj__"){
    ((ConfInt&)m_config["general"]["readoutChannelReadsChip3"]).setValue(roch3assoc->currentIndex()-2);
  }

  accept();
}
void USBCtrlEdit::calcAuxFreq(int freqPwr){
  double freq = 40000./pow(2.,freqPwr);
  QString sfreq;
  sfreq.setNum(freq,'f',3);
  actAuxFreq->setText("(i.e. freq. = "+sfreq+" kHz)");
}

void USBCtrlEdit::AdapterCardFlavourComboBoxStateChanged()
{
  biMultiRadioButton->setEnabled(false);
  biSingleRadioButton->setEnabled(false);

  if ((AdapterCardFlavourComboBox->currentIndex() == 0)){
    ReadFENumberSpinBox->setReadOnly(true);
    ReadFENumberSpinBox->setValue((int)4);
    ReadFENumberSpinBox->setEnabled(false);
  }
  else{
    ReadFENumberSpinBox->setReadOnly(biMultiRadioButton->isChecked());
    //ReadFENumberSpinBox->setValue((int)4);
    ReadFENumberSpinBox->setEnabled(!biMultiRadioButton->isChecked());
  }

  updateReadoutChannelComboboxes();
}

void USBCtrlEdit::updateReadoutChannelComboboxes()
{
  std::vector<QComboBox *> inpdiv;
  inpdiv.push_back(inp0div);
  inpdiv.push_back(inp1div);
  inpdiv.push_back(inp2div);
  inpdiv.push_back(inp3div);
  
  std::vector<QComboBox *> rochinp;
  rochinp.push_back(roch0inp);
  rochinp.push_back(roch1inp);
  rochinp.push_back(roch2inp);
  rochinp.push_back(roch3inp);
  
  std::vector<QComboBox *> rochassoc;
  rochassoc.push_back(roch0assoc);
  rochassoc.push_back(roch1assoc);
  rochassoc.push_back(roch2assoc);
  rochassoc.push_back(roch3assoc);

  int inpdiv_enable_max = inpdiv.size();
  switch(AdapterCardFlavourComboBox->currentIndex())
  {
    case 0:
      inpdiv_enable_max = 1;
      break;
    case 1:
      inpdiv_enable_max = 4;
      break;
  }
  
  bool rochinp_enable = false;
  if ((AdapterCardFlavourComboBox->currentIndex() == 1))
    rochinp_enable = true;
  if (m_overrideEnableDemux)
    rochinp_enable = true;

  for (std::vector<QComboBox *>::iterator it = inpdiv.begin();
      it != inpdiv.end(); it++)
  {
    (*it)->setEnabled(it - inpdiv.begin() < inpdiv_enable_max);
  }
  
  for (std::vector<QComboBox *>::iterator it = rochinp.begin();
      it != rochinp.end(); it++)
  {
    (*it)->setEnabled(rochinp_enable);
  }

  for (std::vector<QComboBox *>::iterator it = rochassoc.begin();
      it != rochassoc.end(); it++)
  {
    (*it)->setEnabled(rochinp_enable);
  }
}
