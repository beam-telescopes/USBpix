/////////////////////////////////////////////////////////////////////
// PixCcpdLF.h
// version 0.1
/////////////////////////////////////////////////////////////////////

#ifndef _PIXLIB_CCPDLF
#define _PIXLIB_CCPDLF

#include <map>
#include <string>

#include "PixCcpd.h"

template<class T> class ConfMask;

namespace PixLib {

  class PixCcpdLF : public PixCcpd {

    friend class PixModule;

  public:

    PixCcpdLF(DBInquire *dbInquire);
    ~PixCcpdLF();
                  
    void loadConfig(std::string configName); // Read the config from DB
    void saveConfig(std::string configName); // Save the config to DB

  private:

    unsigned char m_BLRes;
    unsigned char m_VN;
    unsigned char m_VPFB;
    unsigned char m_VNFoll;
    unsigned char m_VPLoad;
    unsigned char m_LSBdacL;
    unsigned char m_IComp;
    unsigned char m_VStretch;
    unsigned char m_WGT0;
    unsigned char m_WGT1;
    unsigned char m_WGT2;
	unsigned char m_IDacTEST;
	unsigned char m_IDacLTEST;
	unsigned char m_Trim_En;
	
	bool m_Inject_En;
	bool m_Monitor_En;
	bool m_Preamp_En;

	ConfMask<unsigned short int>  m_SW_Ana_mask;
    ConfMask<unsigned short int>  m_Pixels_mask;
   

  };
}
#endif // _PIXLIB_CCPDLF
