/////////////////////////////////////////////////////////////////////
// PixFeData.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 27/06/03  Version 1.0 (CS)
//           Initial release
//
// 17/03/04  Version 1.1 (CS)
//           Added methods to copy sub-blocks
//

//! Image of internal FE data structures

#ifndef _PIXLIB_PIXFEDATA
#define _PIXLIB_PIXFEDATA

#ifdef WIN32
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#endif

#include <map>
#include <string>

namespace PixLib {
	
  class Bits;
  template<class T> class ConfMask;
  class PixFeStructures;
  
  class PixFeData {

  public:

	typedef std::vector<std::string> nameVector;
	typedef nameVector::iterator nameIterator;

    PixFeData();  //! Constructor
    ~PixFeData(); //! Destructor
		
    //! Assignment operator
    PixFeData& operator = (const PixFeData& d);

    //! Partial copy methods
    void copyGlobReg(const PixFeData& source);
    void copyPixReg(const PixFeData& source, std::string name="ALL");
    void copyTrim(const PixFeData& source, std::string name="ALL");

    //! Chip Index and Geographical Address accessor methods
    int& getIndex() {return m_index;}
    void setIndex(int index) {m_index = index;}
    int& getCmdAddr() {return m_cmdAddr;}
    void setCmdAddr(int cmdadd) {m_cmdAddr = cmdadd;}

    //! Enabling mask methods
    bool& getCfgEn() {return m_cfgEnable;}
    void setCfgEn(bool en) {m_cfgEnable = en;}
    bool& getScanEn() {return m_scanEnable;}
    void setScanEn(bool en) {m_scanEnable = en;}
    bool& getDacsEn() {return m_dacsEnable;}
    void setDacsEn(bool en) { m_dacsEnable = en;}

    //! Uniform value for the DACs of each pixel during a DAC scan 
    int& getDefFdac() {return m_defPixelFdac;}
    void setDefFdac(int val) {m_defPixelFdac = val;}
    int& getDefTdac() {return m_defPixelTdac;}
    void setDefTdac(int val) {m_defPixelTdac = val;}

    //! Calibration injection parameters
    float& getCInjLo() {return m_cInjLo;};
    void  setCInjLo(float val) {m_cInjLo = val;}
    float& getCInjMed() {return m_cInjMed;};
    void  setCInjMed(float val) {m_cInjMed = val;}
    float& getCInjHi() {return m_cInjHi;};
    void  setCInjHi(float val) {m_cInjHi = val;}
    float& getVcalGradient(unsigned int i) {if (i<4) return m_vcalGradient[i]; else return m_vcalDummy;;}
    void  setVcalGradient(unsigned int i, float val) {if (i<4) m_vcalGradient[i] = val;}
    float& getOffsetCorrection() {return m_offsetCorrection;}
    void  setOffsetCorrection(float val) {m_offsetCorrection = val;}

    //! Global Register accessor methods
    void setGlobReg(std::string name, int value);
    int& getGlobReg(std::string name, bool &ok);
    bool getGlobReg(Bits &value);
    void resetGlobReg();

    //! Pixel Register accessor methods
    void setPixReg(std::string name, ConfMask<bool> &value);
    void setPixReg(std::string name, int row, int col, bool value);
    void setPixRegCol(std::string name, int col, bool value);
    void setPixRegRow(std::string name, int row, bool value);
    ConfMask<bool>& getPixReg(std::string name, bool &ok);
    bool getPixReg(std::string name, int row, int col, bool &value);
    void resetPixReg(std::string name);

    //! Trim accessor methods
    void setTrim(std::string name, ConfMask<unsigned short int> &value);
    void setTrim(std::string name, int row, int col, unsigned short int value);
    void setTrimCol(std::string name, int col, unsigned short int value);
    void setTrimRow(std::string name, int row, unsigned short int value);
    ConfMask<unsigned short int>& getTrim(std::string name, bool &ok);
    bool getTrim(std::string name, int row, int col, unsigned short int &value);
    void resetTrim(std::string name);

    void setCalib(std::string name, ConfMask<float> &value);
    void setCalib(std::string name, float maxValue, float defValue);
    ConfMask<float>& getCalib(std::string name, bool &ok);

    nameIterator calibBegin();	//iterator to the first calib element (e.g. TOT0)
    nameIterator calibEnd();	//iterator to the last calib element (e.g. TOT12)

    //! FE structures setting
    void setFeStructures(PixFeStructures* structures) {m_structures = structures;}

  private:
    
    int  m_index;   // Front End Index
    int  m_cmdAddr; // Geographical address

    bool m_cfgEnable;  // Mask enable FE config (true if chip has to be configured)
    bool m_scanEnable; // Mask scan FE (true if the chip has to be scanned) 
    bool m_dacsEnable; // Enable the Dacs in the chip (true if the dacs in the chip have to be enabled)

    int  m_defPixelFdac; // Value to be copied into the PixelDacScanData struct
    int  m_defPixelTdac; // Value to be copied into the PixelDacScanData struct

    float m_cInjLo;           // Charge injection LOW
    float m_cInjHi;           // Charge injection HIGH
    float m_cInjMed;          // Charge injection MEDIUM (only FE-I4)
    float m_vcalGradient[4];  // VCAL FE gradient
    float m_vcalDummy;        // VCAL dummy value
    float m_offsetCorrection; // Internal injection offset correction
    nameVector m_calibNames;    // FE calib names

    std::map<std::string, int> m_globRegisters; // FE Global register values
    std::map<std::string, ConfMask<bool> > m_pixRegisters; // FE Pixel register values
    std::map<std::string, ConfMask<unsigned short int> > m_trims; // FE Trim values
    std::map<std::string, ConfMask<float> > m_calibs; // FE calib value maps
    
    PixFeStructures* m_structures; // FE structures
  };

}

#endif
