#include "Records.h"
#include "RawFileWriter.h"

#include <QtTest/QtTest>
#include <vector>
#include <memory>
#include <iostream>
#include <utility>

class RecordsTest: public QObject {
  Q_OBJECT
  private:
    uint32_t build_dh(int flag, int lv1id, int bcid, bool FE_I4B);
    void decode_dh_test(int flag, int lv1id, int bcid, bool FE_I4B);
  private slots:
    void decode_dh();

  private:
    uint32_t build_ar(int type, int address);
    void decode_ar_test(int type, int address);
  private slots:
    void decode_ar();
  
  private:
    uint32_t build_dr(int column, int row, int tot1, int tot2);
    void decode_dr_test(int column, int row, int tot1, int tot2,
        bool intend_failed_cast);
  private slots:
    void decode_dr();
  
  private:
    uint32_t build_vr(int value);
    void decode_vr_test(int value);
  private slots:
    void decode_vr();
  
  private:
    uint32_t build_sr(int code, int trigger_number);
    void decode_sr_test(int code, int trigger_number);
  private slots:
    void decode_sr();

  private:
    std::pair<int, int> build_td(int trigger_mode, int error_code, 
        uint32_t trigger_number);
    void decode_td_test(int trigger_mode, int error_code, 
        uint32_t trigger_number);
  private slots:
    void decode_td();

  private:
    void record_stream_check(RecordStream& rs);
  private slots:
    void record_stream();
};
  
uint32_t RecordsTest::build_dh(int flag, int lv1id, int bcid, bool FE_I4B)
{
  uint32_t h = (0xe8 + 0x01)   << 16 // ID
       | (flag  & 0x01)  << 15;
  if (FE_I4B)
  {
    return h
        | (lv1id & 0x1f)  << 10
        | (bcid  & 0x3ff) <<  0;
  }
  else
  {
    return h
        | (lv1id & 0x7f) << 8
        | (bcid  & 0xff) << 0;
  }
}
    
void RecordsTest::decode_dh_test(int flag, int lv1id, int bcid, bool FE_I4B)
{
  bool next_word_required;
  uint32_t test_word = build_dh(flag, lv1id, bcid, FE_I4B);
  Record *r = Record::create(test_word, FE_I4B, next_word_required);
  QCOMPARE(next_word_required, false);
  std::auto_ptr<Record> foo(r);
  
  DataHeader *rh = dynamic_cast<DataHeader*>(r);
  QVERIFY(rh);
  QCOMPARE(rh->getFlag(), flag);
  QCOMPARE(rh->getBcid(), bcid);
  QCOMPARE(rh->getLv1id(), lv1id);
}
    
void RecordsTest::decode_dh()
{
  decode_dh_test(0, 1,  2,    true);
  decode_dh_test(1, 31, 1023, true);
  decode_dh_test(1, 0,  1023, true);
  decode_dh_test(1, 31, 0,    true);

  decode_dh_test(0, 1,   2,   false);
  decode_dh_test(1, 127, 255, false);
  decode_dh_test(1, 0,   255, false);
  decode_dh_test(1, 127, 0,   false);
}

uint32_t RecordsTest::build_ar(int type, int address)
{
  return (0xe8 + 0x02)       << 16 // ID
       | (type     & 0x01)   << 15
       | (address  & 0x7fff) <<  0;
}

void RecordsTest::decode_ar_test(int type, int address)
{
  bool next_word_required;
  uint32_t test_word = build_ar(type, address);
  Record *r = Record::create(test_word, false, next_word_required);
  QCOMPARE(next_word_required, false);
  std::auto_ptr<Record> foo(r);
  
  AddressRecord *rh = dynamic_cast<AddressRecord *>(r);
  QVERIFY(rh);
  QCOMPARE(rh->getType(), type);
  QCOMPARE(rh->getAddress(), address);
}
   
void RecordsTest::decode_ar()
{
  decode_ar_test(0, 0);
  decode_ar_test(1, 0);
  decode_ar_test(1, 0x7fff);
  decode_ar_test(0, 0x7fff);
  decode_ar_test(0, 5);
  decode_ar_test(0, 325);
}
  
uint32_t RecordsTest::build_dr(int column, int row, int tot1, int tot2)
{
  return (column & 0x07f) << 17
       | (row    & 0x1ff) <<  8
       | (tot1   & 0x00f) <<  4
       | (tot2   & 0x00f) <<  0;
}

void RecordsTest::decode_dr_test(int column, int row, int tot1, int tot2,
    bool intend_failed_cast)
{
  bool next_word_required;
  uint32_t test_word = build_dr(column, row, tot1, tot2);
  Record *r = Record::create(test_word, true, next_word_required);
  QCOMPARE(next_word_required, false);
  std::auto_ptr<Record> foo(r);
 
  DataRecord *rh = dynamic_cast<DataRecord *>(r);
  if (intend_failed_cast)
  {
    QVERIFY(rh == 0);
  }
  else
  { 
    QVERIFY(rh);
    QCOMPARE(rh->getColumn(), column);
    QCOMPARE(rh->getRow(), row);
    QCOMPARE(rh->getTot1(), tot1);
    QCOMPARE(rh->getTot2(), tot2);
  }
}

void RecordsTest::decode_dr()
{
  std::vector<int> column, row, tot;
  column.push_back(1);
  column.push_back(25);
  column.push_back(80);

  row.push_back(1);
  row.push_back(29);
  row.push_back(102);
  row.push_back(336);

  tot.push_back(0);
  tot.push_back(5);
  tot.push_back(14);

  for (std::vector<int>::iterator c = column.begin(); c != column.end(); c++)
  {
    for (std::vector<int>::iterator r = row.begin(); r != row.end(); r++)
    {
      for (std::vector<int>::iterator t1 = tot.begin(); t1 != tot.end(); t1++)
      {
        for (std::vector<int>::iterator t2 = tot.begin(); t2 != tot.end(); t2++)
        {
          decode_dr_test(*c, *r, *t1, *t2, false);
        }
      }
    }
  }
}

uint32_t RecordsTest::build_vr(int value)
{
  return (0xe8 + 0x04)       << 16 // ID
    | (value & 0xffff);
}

void RecordsTest::decode_vr_test(int value)
{
  bool next_word_required;
  uint32_t test_word = build_vr(value);
  Record *r = Record::create(test_word, true, next_word_required);
  QCOMPARE(next_word_required, false);
  std::auto_ptr<Record> foo(r);
  
  ValueRecord *rh = dynamic_cast<ValueRecord *>(r);
  QVERIFY(rh);
  QCOMPARE(rh->getValue(), value);
}

void RecordsTest::decode_vr()
{
  decode_vr_test(0);
  decode_vr_test(1);
  decode_vr_test(32);
  decode_vr_test(2613);
  decode_vr_test(0xfffe);
  decode_vr_test(0xffff);
}

uint32_t RecordsTest::build_sr(int code, int trigger_number)
{
  return (0xe8 + 0x07)       << 16 // ID
    | ((code    &  0x3f) << 10)
    | ((trigger_number & 0x3ff) <<  0);
}

void RecordsTest::decode_sr_test(int code, int trigger_number)
{
  bool next_word_required;
  uint32_t test_word = build_sr(code, trigger_number);
  Record *r = Record::create(test_word, true, next_word_required);
  QCOMPARE(next_word_required, false);
  std::auto_ptr<Record> foo(r);
  
  ServiceRecord *rh = dynamic_cast<ServiceRecord *>(r);
  QVERIFY(rh);
  QCOMPARE(rh->getCode(), code);
  QCOMPARE(rh->getCounter(), trigger_number);
}

void RecordsTest::decode_sr()
{
  decode_sr_test(0, 0);
  decode_sr_test(1, 0);
  decode_sr_test(63, 0);
  decode_sr_test(0, 1);
  decode_sr_test(1, 1);
  decode_sr_test(63, 1);
  decode_sr_test(0, 0x3ff);
  decode_sr_test(1, 0x3ff);
  decode_sr_test(63, 0x3ff);
}

std::pair<int, int> RecordsTest::build_td(int trigger_mode, int error_code, uint32_t trigger_number)
{
  int h = (0xf8UL << 16) 
        | (uint64_t(((trigger_mode & 0x7) << 5) | (error_code & 0x1f)) << 8)
        | (trigger_number >> 24);
  int l = (trigger_number & 0xffffff);
  return std::pair<int, int>(h, l);
}
void RecordsTest::decode_td_test(int trigger_mode, int error_code, 
    uint32_t trigger_number)
{
  bool next_word_required;

  std::pair<int, int> d = build_td(trigger_mode, error_code, trigger_number);
  Record *r = Record::create(d.first, true, next_word_required);
  QCOMPARE(next_word_required, true);

  std::auto_ptr<Record> foo(r);
  
  TriggerDataRecord *rh = dynamic_cast<TriggerDataRecord *>(r);
  QVERIFY(rh);

  rh->addWord(d.second);

  QCOMPARE(rh->getTriggerNumber(), trigger_number);
  QCOMPARE(int(rh->getTriggerMode()), trigger_mode);
  QCOMPARE(int(rh->getErrorCode()), error_code);
}

void RecordsTest::decode_td()
{
  QVERIFY(true);
  decode_td_test(0, 0, 0);
  decode_td_test(0, 0, 0x00ffffff);
  decode_td_test(0, 0, 0xff000000);
  decode_td_test(0, 0, 0xffffffff);
  decode_td_test(0, 0, 0xf0f0f0f0);
  decode_td_test(0, 0, 0x0f0f0f0f);
  decode_td_test(0, 0, 0xffffffff);
  decode_td_test(0, 1, 0);
  decode_td_test(0, 1, 0xffffffff);
  decode_td_test(0, 0x1f, 0);
  decode_td_test(0, 0x1f, 0xffffffff);
  decode_td_test(0x7, 1, 0);
  decode_td_test(0x7, 1, 0xffffffff);
  decode_td_test(0x7, 0x1f, 0);
  decode_td_test(0x7, 0x1f, 0xffffffff);
}

void RecordsTest::record_stream_check(RecordStream& rs)
{
  QCOMPARE(dynamic_cast<DataRecord *>(rs.data.at(5))->getColumn(), 10);
  QCOMPARE(dynamic_cast<DataRecord *>(rs.data.at(6))->getColumn(), 10);
  QCOMPARE(dynamic_cast<DataRecord *>(rs.data.at(7))->getColumn(), 10);
  QCOMPARE(dynamic_cast<DataRecord *>(rs.data.at(5))->getRow(), 31);
  QCOMPARE(dynamic_cast<DataRecord *>(rs.data.at(6))->getRow(), 32);
  QCOMPARE(dynamic_cast<DataRecord *>(rs.data.at(7))->getRow(), 33);

  int dh = 0;
  for (std::vector<Record *>::iterator it = rs.data.begin();
      it != rs.data.end(); it++)
  {
    if (dynamic_cast<DataHeader *>(*it))
      dh++;
  }
  QCOMPARE(dh, 16);
}

void RecordsTest::record_stream()
{
  int lv1id = 0;
  std::vector<int> test_stream;
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dr(10, 31, 5, 15));
  test_stream.push_back(build_dr(10, 32, 2, 15));
  test_stream.push_back(build_dr(10, 33, 1, 15));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));
  test_stream.push_back(build_dh(0, lv1id++, 30, true));

  {
    RecordStream rs(true);
    rs.append(test_stream);
    record_stream_check(rs);
    RawFileWriter rfw((std::cout));
    rfw.digest(&rs);
  }

  {
    RecordStream rs(true);
    rs.append(&(test_stream.at(0)), test_stream.size());
    record_stream_check(rs);
  }
}

QTEST_MAIN(RecordsTest)
#include "RecordsTest.moc"
