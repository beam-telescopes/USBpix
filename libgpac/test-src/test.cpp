#include <QTest>
#include <memory>
#include <iostream>
#include "gpac.h"
#include "phy.h"
#include "logical.h"
#include "i2chost.h"
#include <SiLibUSB.h>

#define CF__LINUX
#include <unistd.h>
#include <upsleep.h>

using namespace gpac;

class test: public QObject {
  Q_OBJECT

  private:
    std::unique_ptr<logical> l;
 
    void test_power_supply(int id);
  private slots:
    void initTestCase();
    void power_supplies();
    void random();
};
    
void test::initTestCase()
{
  InitUSB();
  std::cout << "Number of USB devices: " << GetNumberOfUSBBoards() << std::endl;
  void * udev = GetUSBDevice(137);
  std::cout << "USBPix 137: " << udev << std::endl;
  SiUSBDevice *dev = nullptr;
  if (udev) {
    dev = new SiUSBDevice(udev);
    //bool download = dev->DownloadXilinx("../config/usbpixi4.bit");
    //std::cout << "Download FPGA Firmware: " << download << std::endl;
    //UPGen::Sleep(5000);
    std::cout << "USB Device: " << dev->GetFWVersion() << std::endl;
  }
  l.reset(new logical(dev));
  QVERIFY(l.get());
}

void test::test_power_supply(int id)
{
  std::cerr << "-- Power Supply " << id << " --" << std::endl;
  l->power_supplies.current_limit(0.1);
  l->power_supplies[id].enabled(false);
  l->power_supplies[id].output_voltage(0);
  l->power_supplies[id].enabled(true);
  UPGen::Sleep(100);
  double vlow = l->power_supplies[id].capture_voltage();
  UPGen::Sleep(100);
  l->power_supplies[id].output_voltage(5);
  double vhigh = l->power_supplies[id].capture_voltage();
  std::cerr << " + Minimum voltage: " << vlow << " V" << std::endl;
  std::cerr << " + Maximum voltage: " << vhigh << " V" << std::endl;
  l->power_supplies[id].output_voltage(1.0);
  UPGen::Sleep(100);
  std::cerr << " + Vdiff (1.0V): " << l->power_supplies[id].capture_voltage() << " V" << std::endl;
  l->power_supplies[id].output_voltage(1.2);
  UPGen::Sleep(100);
  std::cerr << " + Vdiff (1.2V): " << l->power_supplies[id].capture_voltage() << " V" << std::endl;
  l->power_supplies[id].output_voltage(1.4);
  UPGen::Sleep(100);
  std::cerr << " + Vdiff (1.4V): " << l->power_supplies[id].capture_voltage() << " V" << std::endl;

}
    
void test::power_supplies()
{
  for (int i = 0; i < 4; i++)
  {
    test_power_supply(i);
  }
}

void test::random()
{
//  std::unique_ptr<i2chost> h{new dummy_i2chost()}; 
//  h->write8(0xaa, 0x55);
//  QVERIFY(h.get());
//  
//  std::unique_ptr<phy> p{new phy()}; 
//  QVERIFY(p.get());
//  std::cout << __LINE__ << std::endl;
//  QVERIFY(&(p->core().dac()));
//  std::cout << __LINE__ << std::endl;
//  QVERIFY(&(p->core().adc()));
//  std::cout << __LINE__ << std::endl;
//  QVERIFY(&(p->core().adc()));
//  std::cout << __LINE__ << std::endl;
//  QVERIFY(&(p->core().dac()[gpac::devices::dac_block::U4]));
//  QVERIFY(&(p->core().dac()[1]));
//  QVERIFY(&(p->core().dac()[2]));
//  std::cout << __LINE__ << std::endl;
//  p->core().i2c_mux().disable();
//  std::cout << __LINE__ << std::endl;
//  p->core().dac()[0].update(3, 1024);

//  std::cout << "Current limit to 100 mA" << std::endl;
//  l->power_supplies.current_limit(0.1);
//  std::cout << "Voltage 0 to 2" << std::endl;
//  l->power_supplies[0].output_voltage(2);
//  std::cout << "Voltage 0 to 0" << std::endl;
//  l->power_supplies[0].output_voltage(0);
//  std::cout << "Voltage 0 to 2" << std::endl;
//  l->power_supplies[0].output_voltage(2);
//  std::cout << "Voltage 0 to 1.3" << std::endl;
//  l->power_supplies[0].output_voltage(1.3);
//  std::cout << "Voltage 0 enable" << std::endl;
//  l->power_supplies[0].enabled(true);
//  std::cout << "Voltage 0 capture..." << std::endl;
//  double x = l->power_supplies[0].capture_voltage();
//  std::cout << " -> " << x << std::endl;
//  std::cout << "Current 0 capture..." << std::endl;
//  x = l->power_supplies[0].capture_current();
//  UPGen::Sleep(1000);
//  std::cout << " -> " << x << std::endl;
//  std::cout << "Voltage 1 to 0" << std::endl;
//  l->power_supplies[1].output_voltage(0);
//  std::cout << "Voltage 1 to 2" << std::endl;
//  l->power_supplies[1].output_voltage(2);
//  std::cout << "Voltage 1 to 1.3" << std::endl;
//  l->power_supplies[1].output_voltage(1.3);
//  std::cout << "Voltage 1 enable" << std::endl;
//  l->power_supplies[1].enabled(true);
//  std::cout << "Voltage 0 disable" << std::endl;
//  l->power_supplies[0].enabled(false);
//  std::cout << "Voltage 0 overcurrent: " << std::flush;
//  bool oc = l->power_supplies[0].overcurrent();
//  std::cout << oc << std::endl;
}

QTEST_MAIN(test)
#include "test.moc"
