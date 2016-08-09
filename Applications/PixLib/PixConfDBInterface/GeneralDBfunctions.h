#ifndef GENDBF_H
#define GENDBF_H

#include <string>
#include <vector>

namespace PixLib{
  class PixConfDBInterface;
  class DBInquire;
  class DBField;
  class fieldIterator;
  class ConfInt;

  struct rodData{
    int slot;
    std::string IPfile, IDfile, Xfile;
    unsigned int fmtLinkMap[8][4];
    int mode;
  };
  struct bocData{
    bool haveBoc;
    int mode;
    std::vector<unsigned int> used_in_links[4];
  };
  struct modData{
    int inLink, outLink[4];
    int modID, grpID, roType;
    int pp0, slot;
    bool active;
    std::string fname, modname, connName;
    std::string feName, cpixName;
    int assyType, assyID, pos_on_assy;
  };
  struct grpData{
    rodData myROD;
    bocData myBOC;
    std::vector<modData> myMods;
    int cfgType;
  };

  void insertDBInquire(DBInquire *org_inquire, DBInquire *insert_inquire, bool tree=true, const char *newName=0);
  void insertDBField(DBField *org_field, DBInquire *insert_inquire);
  void getDecNameCore(std::string&);


  void WriteIntConf(ConfInt& myco,int myvalue);

  PixConfDBInterface* createEmptyDB(const char *fname);
  PixConfDBInterface* createDefaultDB(const char *fname, const char *);
  PixConfDBInterface* addAppInq(const char *fname, const char *appName=0);
  DBInquire* findAppInq(PixConfDBInterface* myDB, const char *appName=0);
  DBInquire* newGroupWiz(const char *class_name, const char *grp_name, DBInquire *startInq);
  DBInquire* newGroupWiz(rodData grpDt_in, DBInquire *startInq);
  void addBocToGroup(bocData bocDt_in, DBInquire *grpInq);
  std::vector<DBInquire*> newModWiz(std::vector<modData> modDt_in, DBInquire *grpInq, const char *plPath);
  DBInquire* createEmptyModule(DBInquire *baseInq, std::string modName, int nFe, int nMcc, int nCcpd);
  DBInquire* createEmptyBoc(DBInquire *baseInq, std::string bocName);
  void fixModule(DBInquire *modInq, modData modDt_in);
  /** fills vectors with input and output links corresponding to PP0 slot and BOC setup */
  void listLinks(std::vector<int> &in, std::vector<int> &out1, std::vector<int> &out2,
                       std::vector<int> &out3, std::vector<int> &out4, int setup, int pp0);
  void addGrouptoDB(DBInquire *startInq, grpData inGrpData, const char *pathPL);
}

#endif // GENDBF_H
