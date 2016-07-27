#include "RawFileWriter.h"

#include <memory>
#include <iostream>
#include <iomanip>
#include <algorithm>  
#include <functional>

RawFileWriter::RawFileWriter(std::ostream& os): os(os)
{
}

RawFileWriter::~RawFileWriter()
{
}
    
void RawFileWriter::digest(const RecordStream* rs)
{
  std::list<Record *>::const_iterator it = rs->data.begin();
  for (;it != rs->data.end(); it++)
  {
    (*it)->write_to(os);
  }
}

void RawFileWriter::writeCounts(int td, int dh, int dr, int hits)
{
  os << "# Counters:"    << std::endl;
  os << "# #Triggers: " 
     << std::setw(10) << td   << std::setw(0) << std::endl;

  os << "# #DHs:      " 
     << std::setw(10) << dh   << std::setw(0) << std::endl;

  os << "# #DRs:      " 
     << std::setw(10) << dr   << std::setw(0) << std::endl;

  os << "# #Hits:     " 
     << std::setw(10) << hits << std::setw(0) << std::endl;
}

void RawFileWriter::writeLV1Histo(const std::vector<int> &v)
{
  os << "# HitLV1:" << std::endl; 
  int i = 0;
  for (std::vector<int>::const_iterator it = v.begin(); it != v.end(); it++)
  {
    os << "# " << std::setw(2) << i << " : " << std::setw(10) << (*it) << std::setw(0) << std::endl; 
    i++;
  }
  os << "#" << std::endl; 
}
    
void RawFileWriter::epilogue(RawDataHistogrammer &h)
{
  os << "End of dfifo block read" << std::endl
     << "#" << std::endl;
 
  os << "# CHIP SUMMARY" << std::endl;
  os << "# "             << std::endl;

  writeCounts(h.getCountTD(), h.getCountDH(), h.getCountDR(), h.getCountHits());

  os << "#" << std::endl; 
  
  writeLV1Histo(h.getHitLV1());
}
    
void RawFileWriter::epilogue(std::vector<RawDataHistogrammer> &hs)
{
  int td = 0, dh = 0, dr = 0, hits = 0;
  std::vector<int> lv1;
  for (std::vector<RawDataHistogrammer>::iterator it = hs.begin();
      it != hs.end(); it++)
  {
    const std::vector<int> &lv1c = it->getHitLV1();
    if (lv1.empty())
    {
      lv1.resize(lv1c.size());
      std::fill(lv1.begin(), lv1.end(), 0);
    }

    std::transform(lv1.begin(), lv1.end(), lv1c.begin(), lv1.begin(), 
        std::plus<int>());

    td   += it->getCountTD();
    dh   += it->getCountDH();
    dr   += it->getCountDR();
    hits += it->getCountHits();
  }

  os << "# "               << std::endl;
  os << "# MODULE SUMMARY" << std::endl;
  os << "# "               << std::endl;

  writeCounts(td, dh, dr, hits);

  os << "#" << std::endl; 
  
  writeLV1Histo(lv1);
}
