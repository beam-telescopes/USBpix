//------------------------------------------------------------------------------
//       USBDeviceManager.h
//
//       SILAB, Phys. Inst Bonn, HK
//
//  Handle multiple USB devices associated to slbusb.sys device driver
//
//  History:
//  31.01.03   BLUETRAK version!!!
//  03.10.01  HK  created
//  22.02.02  PF  see .cpp file
//------------------------------------------------------------------------------


#ifndef USBDeviceManagerH
#define USBDeviceManagerH

#include <stdlib.h>
#include <vector>
#include "SiUSBDevice.h"

const int MAX_USB_DEV_NUMBER = 16;         // number of devices

typedef struct  _SLB_USB_DEVICE_TYPE       // Used in constant array 'DeviceTypeList'
{
	unsigned short                    VendorID;
	unsigned short                    ProductID;
	//	SLB_USB_DEVICE_TYPE_NAME  DeviceTypeName;
	char                    * FirmwareFilename;
} SLB_USB_DEVICE_TYPE;

typedef struct _USB_DEVICE_LIST_ITEM
{
	bool                      DevicePresent;
	TUSBDevice              * Device;
	//SLB_USB_DEVICE_TYPE_NAME  DeviceTypeName;
} USB_DEVICE_LIST_ITEM;

class TUSBDevice;
class TUSBDeviceManager
{
public:
	TUSBDeviceManager();
	~TUSBDeviceManager();
	bool HandleDeviceChange(void);
	int  DevToIndex(TUSBDevice *dev);
	int  DevToId(TUSBDevice *dev);
	bool IsBusy();
	int GetNumberofDevices();
	int GetMaxNumberofDevices();
	void ForceRefresh();
	void SetAddCallBack(void (*addfunc) (TUSBDevice*, void*), void *cnt);
	void SetRemoveCallBack(void (*remfunc) (TUSBDevice*, void*), void *cnt);
	void* GetDevice(int id = -1);
	void* GetDevice(int id, int DevClass);
	USB_DEVICE_LIST_ITEM DeviceList[MAX_USB_DEV_NUMBER];
	std::vector <std::stringstream*> devInfoStrings;

#ifdef WIN32LOCK
	int usb_rwlock;
#endif

private:
	bool AddDeviceToList      (int index);
	bool RemoveDeviceFromList (int index);
	int UpdateDevStringList ();
	bool busy;
	void * DeviceContext;
	void (* onDevicePlugged) (TUSBDevice* dev, void *ptr);
	void (* onDeviceUnplugged) (TUSBDevice* dev, void *ptr);
};

#endif
