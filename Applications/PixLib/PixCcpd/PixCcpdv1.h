/////////////////////////////////////////////////////////////////////
// PixCcpdv1.h
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 11/02/13  Version 0.1 (JGK)
//           Skeleton version

//! Class for the CCPD V1 chip 

#ifndef _PIXLIB_CCPDV1
#define _PIXLIB_CCPDV1

#include <map>
#include <string>

#include "PixCcpd.h"

template<class T> class ConfMask;

namespace PixLib {

  class PixCcpdv1 : public PixCcpd {

    friend class PixModule;

  public:

    PixCcpdv1(DBInquire *dbInquire);
    ~PixCcpdv1();

    void loadConfig(std::string configName); // Read the config from DB
    void saveConfig(std::string configName); // Save the config to DB

  private:

    unsigned char m_BLRes;
    unsigned char m_ThRes;
    unsigned char m_VN;
    unsigned char m_VNFB;
    unsigned char m_VNFoll;
    unsigned char m_VNLoad;
    unsigned char m_VNDAC;
    unsigned char m_NU1;
    unsigned char m_NU2;
    unsigned char m_NotUsed;
    unsigned char m_VNComp;
    unsigned char m_VNCompL;
    unsigned char m_VNOut0;
    unsigned char m_VNOut1;
    unsigned char m_VNOut2;
	
	unsigned char m_InDACR;
	unsigned char m_InDACL;
	bool m_EnR;
	bool m_EnL;
	bool m_L0;
	bool m_L1;
	bool m_L2;
	bool m_R0;
	bool m_R1;
	bool m_R2;
	bool m_EnStrip;
	bool m_EnCurrent;
	
	unsigned int m_CcpdThreshold;
	unsigned int m_CcpdVcal;

    ConfMask<unsigned short int>  m_InDAC_mask;
  };
}
#endif // _PIXLIB_CCPDV1
