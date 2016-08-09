/////////////////////////////////////////////////////////////////////
// PixCcpdv2.h
// version 0.1
/////////////////////////////////////////////////////////////////////
//
// 11/02/13  Version 1.0 (JGK)
//           more or less a copy of v1

//! Class for the CCPD V1 chip 

#ifndef _PIXLIB_CCPDV2
#define _PIXLIB_CCPDV2

#include <map>
#include <string>

#include "PixCcpd.h"

template<class T> class ConfMask;

namespace PixLib {

  class PixCcpdv2 : public PixCcpd {

    friend class PixModule;

  public:

    PixCcpdv2(DBInquire *dbInquire);
    ~PixCcpdv2();
                  
    void loadConfig(std::string configName); // Read the config from DB
    void saveConfig(std::string configName); // Save the config to DB

  private:

    unsigned char m_BLRes;
    unsigned char m_ThRes;
    unsigned char m_VN;
    unsigned char m_VN2;
    unsigned char m_VNFB;
    unsigned char m_VNFoll;
    unsigned char m_VNLoad;
    unsigned char m_VNDAC;
    unsigned char m_ThPRes;
    unsigned char m_ThP;
    unsigned char m_VNOut;
    unsigned char m_VNComp;
    unsigned char m_VNCompLd;
    unsigned char m_VNOut1;
    unsigned char m_VNOut2;
    unsigned char m_VNOut3;
	unsigned char m_VNBuffer;
	unsigned char m_VPFoll;
	unsigned char m_VNBias;
	unsigned char m_Q0;
	unsigned char m_Q1;
	unsigned char m_Q2;
	unsigned char m_Q3;
	unsigned char m_Q4;
	unsigned char m_Q5;
	
	bool m_Str;
	bool m_dc;
	bool m_SimplePixel;

    ConfMask<unsigned short int>  m_InDAC_mask;
    ConfMask<unsigned short int>  m_Ampout_mask;
    ConfMask<unsigned short int>  m_Enable_mask;
	ConfMask<unsigned short int>  m_Monitor_mask;

  };
}
#endif // _PIXLIB_CCPDV2
