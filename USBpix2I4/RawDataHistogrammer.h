#if !defined(USBPIX_RAW_DATA_HISTOGRAMMER_H)
#define USBPIX_RAW_DATA_HISTOGRAMMER_H

#include "Records.h"
#include "HistogramTypes.h"
#include "dllexport.h"

class DllExport RawDataHistogrammer
{
  private:
    TOTHisto_t &TOTHisto;
    ConfHisto_t &ConfHisto;
    HitLV1Histo_t& HitLV1Histo;
    LV1IDHisto_t& LV1IDHisto;
    BCIDHisto_t& BCIDHisto;
    int chip;
    bool FE_I4B;

    int lv1id_high;
    int lv1id_low;

    int bcid_high;
    int bcid_low;
    int bcid_low_top_value;

    int next_bcid_low;
    int lv1_int;
    bool first_data;


  public:
    RawDataHistogrammer(TOTHisto_t &TOTHisto,
        ConfHisto_t &ConfHisto,
        HitLV1Histo_t& HitLV1Histo,
        LV1IDHisto_t& LV1IDHisto,
        BCIDHisto_t& BCIDHisto,
        bool isFEI4B);
    RawDataHistogrammer& operator=(RawDataHistogrammer rds);

    void setChipId(int chip) {this->chip = chip;};
    void digest(const RecordStream* rs);
    void reset();
  
  private:
    int count_td;
    int count_dh;
    int count_dr;
    int count_hits;

  public:
    int getCountTD()       {return count_td;};
    int getCountDH()       {return count_dh;};
    int getCountDR()       {return count_dr;};
    int getCountHits()     {return count_hits;};
    int getHitLV1(int lv1) {return HitLV1Histo.at(chip).at(lv1);}
    const std::vector<int>& getHitLV1() {return HitLV1Histo.at(chip);}

  private:
    void digest(Record *r);
    void digest(DataHeader *r);
    void digest(DataRecord *r);
    void add(int col, int row, int tot);

    int bcid()  {return bcid_low  + bcid_high;  };
    int lv1id() {return lv1id_low + lv1id_high; };
   
  private: 
    bool m_suppress_tot_14;
  public:
    void suppress_tot_14(bool v) { m_suppress_tot_14 = v;}; 
};

#endif
