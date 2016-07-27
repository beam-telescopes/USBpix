#ifndef USBPIX_RECORDS_H
#define USBPIX_RECORDS_H

#include "dllexport.h"

#include <vector>
#include <list>
#include <ostream>
#ifdef WIN32
	#if _MSC_VER >= 1700
		#include <stdint.h>
	#else
		#include "inttypes_win32.h"
	#endif
#else
	#include <inttypes.h>
#endif

class DllExport Record
{
  protected:
    int data_word;
  public:
    Record(int data_word);

    static Record *create(int data_word, bool FE_I4B, bool &next_word_required);

    virtual ~Record();
    int getDataWord() {return data_word;};
    virtual void write_to(std::ostream& os);
    virtual void addWord(int data_word);

    virtual bool isDH() {return false;};
    virtual bool isDR() {return false;};
    virtual bool isTD() {return false;};
};

class DllExport DataHeader: public Record
{
  private:
    bool flag;
    uint8_t lv1id;
    uint16_t bcid;
  public:
    DataHeader(int data_word, bool FE_I4B);

    bool getFlag() {return flag;};
    int getLv1id() {return lv1id;};
    int getBcid() {return bcid;};
    void write_to(std::ostream& os);
    virtual bool isDH() {return true;};
};

class DllExport AddressRecord: public Record
{
  private:
    uint8_t type;
    uint16_t address;
  public:
    AddressRecord (int data_word);

    int getType() {return type;};
    int getAddress() {return address;};
    
    virtual void write_to(std::ostream& os);
};

class DllExport DataRecord: public Record
{
  private:
    uint8_t column;
    uint16_t row;
    uint8_t tot1;
    uint8_t tot2;
  public:
    DataRecord(int data_word);

    int getColumn() {return column;};
    int getRow() {return row;};
    int getTot1() {return tot1;};
    int getTot2() {return tot2;};
    
    virtual void write_to(std::ostream& os);
    virtual bool isDR() {return true;};
};

class DllExport ServiceRecord: public Record
{
  private:
    uint8_t code;
    uint16_t counter;
  public:
    ServiceRecord(int data_word);

    int getCode() {return code;};
    int getCounter() {return counter;};
    
    virtual void write_to(std::ostream& os);
};

class DllExport ValueRecord: public Record
{
  private:
    uint16_t value;
  public:
    ValueRecord(int data_word);

    int getValue() {return value;};
    
    virtual void write_to(std::ostream& os);
};

class DllExport TriggerDataRecord: public Record
{
  private:
    uint8_t trigger_mode;
    uint8_t error_code;
    uint32_t trigger_number;

  public:
    uint8_t getTriggerMode() {return trigger_mode; };
    uint8_t getErrorCode() {return error_code; };
    uint32_t getTriggerNumber() {return trigger_number; };
  
  public:
    TriggerDataRecord(int data_word);
    void addWord(int data_word);

    virtual void write_to(std::ostream& os);
    
    virtual bool isTD() {return true;};
};

class DllExport DecoderError: public Record
{
  public:
    DecoderError(int data_word);
    
    virtual void write_to(std::ostream& os);
};

class DllExport RecordStream
{
  private:
    bool FE_I4B;
    bool partial_word;
  public:
    std::list<Record *> data; 
    void append(int data_word);
    void append(const int data_words[], int length);
    void append(const std::vector<int> &data_words);
    RecordStream(bool FE_I4B);
    RecordStream(const RecordStream& rs, bool FE_I4B);
    ~RecordStream();
    RecordStream& operator=(RecordStream rs);
};

#endif
