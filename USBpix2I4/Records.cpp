#include <iostream>
#include <exception>
#include <algorithm>
#include <iomanip>

#include "Records.h"

const uint8_t IDB_DATA_HEADER    = 0xe8 + 0x01; // 11101 | 001
const uint8_t IDB_ADDRESS_RECORD = 0xe8 + 0x02; // 11101 | 010
const uint8_t IDB_VALUE_RECORD   = 0xe8 + 0x04; // 11101 | 100
const uint8_t IDB_SERVICE_RECORD = 0xe8 + 0x07; // 11101 | 111
const uint8_t IDB_TRIGGER_DATA   = 0xf8 + 0x00; // 11111 | 000

const bool DEBUG_DECODER = false;

Record *Record::create(int data_word, bool FE_I4B, bool &next_word_required)
{
  next_word_required = false;

  uint8_t id_block = (data_word >> 16) & 0xff;

  try
  {
    switch (id_block)
    {
      case IDB_DATA_HEADER:
        return new DataHeader(data_word, FE_I4B);

      case IDB_ADDRESS_RECORD:
        return new AddressRecord(data_word);

      case IDB_VALUE_RECORD:
        return new ValueRecord(data_word);

      case IDB_SERVICE_RECORD:
        return new ServiceRecord(data_word);

      case IDB_TRIGGER_DATA:
        next_word_required = true;
        return new TriggerDataRecord(data_word);

      default:
        return new DataRecord(data_word);
    }
  }
  catch (DecoderError* r)
  {
    return r;
  }
}
    
Record::Record(int data_word): data_word(data_word)
{
}

Record::~Record()
{
}

void Record::addWord(int data_word)
{
  (void) data_word;
}
    
void Record::write_to(std::ostream& os)
{
  os << "0x" 
     << std::hex << std::setw(6) << std::setfill('0') 
     << data_word 
     << std::dec << std::setw(0) << std::setfill(' ')
     << std::endl;
}
    
DataHeader::DataHeader(int data_word, bool FE_I4B): Record(data_word)
{
  this->flag  = (data_word >> 15) & 1;
  if (FE_I4B)
  {
    this->lv1id = (data_word >> 10) & 0x1f;
    this->bcid  = (data_word >>  0) & 0x3ff;
  }
  else
  {
    this->lv1id = (data_word >>  8) & 0x7f;
    this->bcid  = (data_word >>  0) & 0xff;
  }

  if (DEBUG_DECODER)
    std::cerr << __FILE__ << ":" << __LINE__ << ": DataHeader" 
      << "; flag = "   << int(flag)
      << "; lv1id = "  << int(lv1id)
      << "; bcid = "   << int(bcid)
      << "; FE_I4B = " << int(FE_I4B)
      << std::endl;
}

void DataHeader::write_to(std::ostream& os)
{
  Record::write_to(os);
  os << "DH " << int(flag)
      << " "  << int(lv1id)
      << " "  << int(bcid)
      << std::endl;
}

AddressRecord::AddressRecord (int data_word): Record(data_word)
{
  this->type    = (data_word >> 15) & 1;
  this->address = (data_word >>  0) & 0x7fff;

  if (DEBUG_DECODER)
    std::cerr << __FILE__ << ":" << __LINE__ << ": AddressRecord" 
      << "; type = "  << type
      << "; address = " << address
      << std::endl;
}
    
void AddressRecord::write_to(std::ostream& os)
{
  Record::write_to(os);
  os << "AR " << type
      << " "  << address
      << std::endl;
}

DataRecord::DataRecord(int data_word): Record(data_word)
{
  this->tot2   = (data_word >>  0) & 0xf;
  this->tot1   = (data_word >>  4) & 0xf;
  this->row    = (data_word >>  8) & 0x1ff;
  this->column = (data_word >> 17) & 0x7f;

  if (DEBUG_DECODER)
    std::cerr << __FILE__ << ":" << __LINE__ << ": DataRecord" 
      << "; column = " << int(column)
      << "; row = "  << int(row)
      << "; tot1 = " << int(tot1)
      << "; tot2 = " << int(tot2)
      << std::endl;

  if ((row > 336) || (row == 0) || (column > 80) || (column == 0))
  {
    throw new DecoderError(data_word);
  }
}

void DataRecord::write_to(std::ostream& os)
{
  Record::write_to(os);
  os << "DR"
      << " " << int(column)
      << " " << int(row)
      << " " << int(tot1)
      << " " << int(tot2)
      << std::endl;
}

ServiceRecord::ServiceRecord (int data_word): Record(data_word)
{
  this->code    = (data_word >> 10) & 0x3f;
  this->counter = (data_word >>  0) & 0x3ff;

  if (DEBUG_DECODER)
    std::cerr << __FILE__ << ":" << __LINE__ << ": AddressRecord" 
      << "; code = "  << int(code)
      << "; counter = " << int(counter)
      << std::endl;
}

void ServiceRecord::write_to(std::ostream& os)
{
  Record::write_to(os);
  os << "SR " << int(code)
      << " "  << int(counter)
      << std::endl;
}

ValueRecord::ValueRecord (int data_word): Record(data_word)
{
  this->value = (data_word) & 0xffff;

  if (DEBUG_DECODER)
    std::cerr << __FILE__ << ":" << __LINE__ << ": AddressRecord" 
      << "; value = "  << int(value)
      << std::endl;

}

void ValueRecord::write_to(std::ostream& os)
{
  Record::write_to(os);
  os << "VR " << int(value)
      << std::endl;
}

TriggerDataRecord::TriggerDataRecord (int data_word): Record(data_word)
{
  this->trigger_mode   =  (data_word >> 13) & 0x07;
  this->error_code     =  (data_word >>  8) & 0x1f;

  this->trigger_number = ((data_word >>  0) & 0xff) << 24;

  if (DEBUG_DECODER)
    std::cerr << __FILE__ << ":" << __LINE__ << ": TriggerDataRecord" 
      << "; trigger_mode = "         << int(trigger_mode)
      << "; error_code = "           << int(error_code)
      << "; trigger_number (MSB) = " << trigger_number
      << std::endl;

}

void TriggerDataRecord::write_to(std::ostream& os)
{
  Record::write_to(os);
  os << "TD" 
      << " " << int(trigger_mode)
      << " " << int(error_code)
      << " " << trigger_number
      << std::endl;
}

void TriggerDataRecord::addWord(int data_word)
{
  this->trigger_number = (this->trigger_number & 0xff000000UL)
                       | (data_word            & 0x00ffffffUL);

  if (DEBUG_DECODER)
    std::cerr << __FILE__ << ":" << __LINE__ << ": TriggerDataRecord supplemented" 
      << "; trigger_mode = "   << int(trigger_mode)
      << "; error_code = "     << int(error_code)
      << "; trigger_number = " << trigger_number
      << std::endl;
}

DecoderError::DecoderError(int data_word): Record(data_word)
{
  if (DEBUG_DECODER)
    std::cerr << __FILE__ << ":" << __LINE__ << ": DecoderError." << std::endl;
}

void DecoderError::write_to(std::ostream& os)
{
  Record::write_to(os);
  os << "# Decoder error"
      << std::endl;
}
    
void RecordStream::append(int data_word)
{
  if (partial_word)
  {
    partial_word = false;
    data.back()->addWord(data_word);
  }
  else
  {
    data.push_back(Record::create(data_word, FE_I4B, partial_word));
  }
}

void RecordStream::append(const int data_words[], int length)
{
  for (int i = 0; i < length; i++)
  {
    append(data_words[i]);
  }
}

void RecordStream::append(const std::vector<int> &data_words)
{
  for (std::vector<int>::const_iterator it = data_words.begin(); 
      it != data_words.end(); it++)
  {
    append(*it);
  }
}

RecordStream::~RecordStream()
{
  for (std::list<Record *>::iterator it = data.begin(); it != data.end(); 
      it++)
  {
    delete *it;
  }
}
    
RecordStream::RecordStream(bool FE_I4B): 
  FE_I4B(FE_I4B),
  partial_word(false)
{
}

RecordStream::RecordStream(const RecordStream& rs, bool FE_I4B): 
  FE_I4B(FE_I4B),
  partial_word(false)
{
  for (std::list<Record *>::const_iterator it = rs.data.begin(); 
      it != rs.data.end(); it++)
  {
    data.push_back(new Record((*it)->getDataWord()));
  }
}

RecordStream& RecordStream::operator=(RecordStream rs)
{
  std::swap(rs.data, this->data);
  this->FE_I4B = rs.FE_I4B;
  this->partial_word = rs.partial_word;
  return *this;
}
