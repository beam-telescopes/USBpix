#ifndef MODULE_H
#define MODULE_H

#include <qvariant.h>
#include <qwidget.h>
#include <string.h>
#include <Histo.h>
#include <PixDBData.h>
#include <TH1.h>
#include <TH2.h>
#include <RootDB.h>
#include <Histo.h>
#include <iostream>



class TH2F;
class TH1F;
class CutItem;
class Cut;

struct gaus_para
{ //structur fuer Beschreibung einer gausskurve
  double constant;
  double mean;
  double sigma;
};


class Modul  
{
public:
    enum actionType {ARITH_MEAN, ARITH_SPREAD, GAUSS_MEAN, GAUSS_SPREAD, 
		     ARITH_MEAN_0, ARITH_SPREAD_0, GAUSS_MEAN_0, GAUSS_SPREAD_0, 
		     NBAD_PIXEL};
    enum pixelType {NORMAL, LONG, GANGED, LONG_GANGED, ALL, INTER_GANGED};

 
    Modul(const char *name, const char *path, const char *file, 
	  int pos=0, int staveID=0, std::string ass_type="unknown");
    ~Modul();
    // std::string GetPath()  {return m_path);};
    std::string GetName()  {return m_name;};
    // std::string GetFile()  {return m_file;};
    std::string GetPassed(){return m_passed;};
    std::vector <std::string> getOutput();
    // void AnaStart(const char *scan, std::map<int, std::vector<CutItem *> > );
    void AnaStart(const char *scan, std::map<int, std::vector<Cut *> > );
    void addScan(const char *name, const char *file, const char *time=0, int ID=-1);
    std::map <std::string, int> getActionTypes(){return m_actTypes;};
    std::map <std::string, int> getPixelTypes(){return m_pixTypes;};
    
    TH2F  *cast_histo_2_root(PixLib::Histo *);


public:  
    //members
    std::string m_ass_Type;
    int m_position;
    int m_staveID;
    // std::stringstream scanstream;
private:
    //private methoden

    // general analysis methods
    double performAna(PixLib::Histo *histo, int action,int pixel, double mincut=0, double maxcut=0);
    int badpix( TH2F *histo, double mincut, double maxcut);

    // specific analysis functions - obsolete?
    // void startThreshold(PixDBData *pixDB, const char *scan);//scantype aus combobox1
    void startBadPix(PixDBData *pixDB);

        
    // histo operations
    std::vector<gaus_para *>  Modul::calc_gaus_2dim_sigma(TH2F *h);
    gaus_para calc_gaus_2dim(TH2F *h,const char *scan,int pixel_type = 4, bool include_zero = false);
    gaus_para calc_gaus_2dim(TH2F *h ,int pixel_type = 4,bool include_zero = false)
      {return calc_gaus_2dim(h,"default",pixel_type ,include_zero);};
    double calc_MEAN_2dim(TH2F *h,int pixel_type = 4, bool include_zero = false);
    pair<double,double> calc_Mean_RMS_2dim(TH2F *h,int pixel_type = 4, bool include_zero = false);

    
    //members
    std::string m_path;
    std::string m_file;
    std::string m_name;
    std::string m_passed;
    std::string m_modgroup;
   
   
    // labels of action types
    std::map <std::string, int> m_actTypes;
    std::map <std::string, int> m_pixTypes;
    std::map <int, std::string> m_scanTypes ;

    // vectors with scan info
    std::vector<string> m_scans;
    std::vector<string> m_scanFiles;
    std::vector<string> m_scanTime;
    std::vector<int> m_scanID;

    std::vector<string> m_scanstring;

    //histogramm schlechter pixel
    int m_badpix[144][320];//("dddf","schlechte Pixel",144,0,144,320,0,320);
};

#endif
