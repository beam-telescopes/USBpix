#ifndef MAENGINE_H
#define MAENGINE_H

#include <map>
#include <vector>
#include <string>

class TopWin;
class ModItem;
class StdTestWin;
class QApplication;
class QLineEdit;
class CutPanel;
class DatSet;
#ifdef PIXEL_ROD // can link to PixScan
namespace PixLib{
class DBInquire;
class PixScan;
}
#endif

namespace MAEngine{

 void     LocalLoadStd(TopWin *tw,ModItem *item, StdTestWin &stdwin, QApplication *app, QLineEdit *textwin);
  /*! Perform fit with know functions and VCAL->charge conversion (if needed)
   */
 ModItem* PerformFit(ModItem *item, bool badfitonly=false, bool prompt_user=true, bool warn_user=true, 
		     TopWin *tw=0, QApplication *app=0, int min_chip=-1, int max_chip=-1);
 void setNeededData(bool *loadData,int cuttype);
 void* defineSpecialCuts(CutPanel &cutpanel, int cutID);
 const char* getSpecialCutLabel(int cutID);
 void getRootFromRDB(QString modSN, QString measType, QString path, bool retainName, QApplication *app=0);
 void getStaveRootFromRDB(QString staveSN, QString userPDB, QString passwdPDB, bool retainName, 
			  QString measType, QString fixedName, QString varName, QApplication *app=0, TopWin *tw=0);
 int  checkPath(const char *);
 /*! returns map of possible files, labels for file menues and file extensions */
 void fileExtensions(std::map<int,QString> &labels, std::map<int,QString> &extensions);
 /*! create TurboDAQ-style (ascii) configuration file
  *  with supp. T/FDAC and mask files
  */
 int WriteConfig(int type, const char *root_in, const char *cfg_out, const char *cfg_name, 
		 bool doAscii=true, int phase1=-999, int phase2=-999);
 /*! load data from a file, either TurboDAQ ascii or (MA-)ROOT; data type is guessed */
 ModItem* loadDataAny(const char *file, TopWin *tw); 
 /*! load data from a TurboDAQ ascii file; doesn't need GUI functionality */
 DatSet* loadDataAscii(const char *file); 
 /*! load a certain std. type of measurement of a set of data in a (MA-)ROOT file */
 DatSet* LoadStdFromRoot(int type, const char *root_in);
 /*! ToT mean, max and RMS analysis */
 void checkToT(DatSet *data, std::vector<float> &result);
 /* ToT label */
 std::string getToTCheckLabel(int);
 /*! guess (integer-) file type from file name extension */
 int GuessType(QString fname);
#ifdef PIXEL_ROD // can link to PixScan
 /*! load data from PixScan object for specified module */
 DatSet* readFromPixScan(PixLib::PixScan *ps, PixLib::DBInquire *moduleIn, const char *label, int *error, int modID=-1, const char *path=0);
 DatSet* findPixScan(const char *fileName, const char *scanName, const char *grpName, const char *moduleName);
#endif
}
#endif // MAENGINE_H
