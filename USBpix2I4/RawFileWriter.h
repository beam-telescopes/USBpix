#if !defined(USBPIX_RAW_FILE_WRITER_H)
#define USBPIX_RAW_FILE_WRITER_H

#include "Records.h"
#include "dllexport.h"

#include "RawDataHistogrammer.h"

#include <ostream>
#include <vector>

class DllExport RawFileWriter 
{
  private:
    std::ostream& os;

  public:
    RawFileWriter(std::ostream& os);
    ~RawFileWriter();

    void digest(const RecordStream* rs);
    void epilogue(RawDataHistogrammer &h);
    void epilogue(std::vector<RawDataHistogrammer> &hs);

  private:
    void writeCounts(int td, int dh, int dr, int hits);
    void writeLV1Histo(const std::vector<int> &v);
    static const int max_lv1 = 16;
};

#endif
