#include "RawDataHistogrammer.h"
#include <iostream>

RawDataHistogrammer::RawDataHistogrammer(
    TOTHisto_t &TOTHisto,
    ConfHisto_t &ConfHisto,
    HitLV1Histo_t& HitLV1Histo,
    LV1IDHisto_t& LV1IDHisto,
    BCIDHisto_t& BCIDHisto,
    bool isFEI4B):
  TOTHisto(TOTHisto),
  ConfHisto(ConfHisto),
  HitLV1Histo(HitLV1Histo),
  LV1IDHisto(LV1IDHisto),
  BCIDHisto(BCIDHisto),
  chip(0),
  FE_I4B(isFEI4B)
{
  reset();
  if (FE_I4B)
    bcid_low_top_value = 1024;
  else
    bcid_low_top_value = 256;
}
    
void RawDataHistogrammer::reset()
{
  lv1id_high    = 0;
  lv1id_low     = 0;
  bcid_high     = 0;
  bcid_low      = 0;
  lv1_int       = 0;
  next_bcid_low = 0;
  count_td      = 0;
  count_dr      = 0;
  count_hits    = 0;
  count_dh      = 0; 
  first_data    = true;
  m_suppress_tot_14 = false;
}

void RawDataHistogrammer::digest(const RecordStream* rs)
{
  std::list<Record *>::const_iterator it = rs->data.begin();
  for (;it != rs->data.end(); it++)
  {
    digest(*it);
  }
}

void RawDataHistogrammer::digest(Record *r)
{
  if (r->isDH())
  {
    DataHeader *dh = static_cast<DataHeader*>(r);
    digest(dh);
  }
  else if (r->isDR())
  {
    DataRecord *dr = static_cast<DataRecord*>(r);
    digest(dr);
  }
  else if (r->isTD())
  {
    count_td++;
  }
}

void RawDataHistogrammer::digest(DataHeader *r)
{
  count_dh++;
  bcid_low = r->getBcid();

  if (first_data || (lv1id_low != r->getLv1id()) || (bcid_low != next_bcid_low))
  {
    first_data = false;
    lv1_int = 0;
  }
  else
  {
    lv1_int = (lv1_int + 1) % 16;
  }

  next_bcid_low = (bcid_low + 1) % bcid_low_top_value;
  lv1id_low = r->getLv1id();
}

void RawDataHistogrammer::add(int col, int row, int tot)
{
  if ((tot != 15) && (!m_suppress_tot_14 || (tot != 14)))
  {
    ConfHisto.at(chip).at(col).at(row).at(0) += 1;
    TOTHisto.at(chip).at(col).at(row).at(tot) += 1;
    count_hits++;
  }
}
void RawDataHistogrammer::digest(DataRecord *r)
{
  count_dr++;
  add(r->getColumn() - 1, r->getRow() - 1, r->getTot1());
  add(r->getColumn() - 1, r->getRow(), r->getTot2());

  HitLV1Histo.at(chip).at(lv1_int)++;
  LV1IDHisto .at(chip).at(lv1id() % 4096)++;
  BCIDHisto  .at(chip).at(bcid()  % 8192)++;
}
    
RawDataHistogrammer& RawDataHistogrammer::operator=(RawDataHistogrammer rds)
{
  lv1id_high    = rds.lv1id_high;
  lv1id_low     = rds.lv1id_low;
  bcid_high     = rds.bcid_high;
  bcid_low      = rds.bcid_low;
  lv1_int       = rds.lv1_int;
  next_bcid_low = rds.next_bcid_low;
  count_td      = rds.count_td;
  count_dr      = rds.count_dr;
  count_hits    = rds.count_hits;
  count_dh      = rds.count_dh;
  first_data    = rds.first_data;
	return *this;
}
