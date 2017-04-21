//--------------------------------------------------------------------------
//       USBDeviceManager.cpp
//
//       SILAB, Phys. Inst Bonn, HK
//
//  Handle multiple USB devices associated to slbusb.sys device driver
//
//  History:
//
//  09 10 11 13 14 15
//  18.06.09  HK  - removed DeviceTypeList (new SiUSBDeviceManager)
//                  device type identification now with DeviceClass
//  21.11.02  HK  - changed HandleDeviceChange function:
//                  does not stop and restart available devices anymore during
//                  search loop
//  22.02.02  PF  - added FindDevice()
//                - added call to HandleDeviceChange() in constructor
//                - made DeviceList private
//  03.10.01  HK  created
//------------------------------------------------------------------------------
#ifdef __VISUALC__
#include <Windows.h>
#endif

#include <sstream>
#include "SiUSBDeviceManager.h"

/*
 * This is the contstruction of the device manager. It creates the main
 * device list.
 */
TUSBDeviceManager::TUSBDeviceManager()
{
	onDevicePlugged   = NULL;
	onDeviceUnplugged = NULL;
	DeviceContext     = NULL;
	busy              = false;

	/* Init device list */
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		DeviceList[i].DevicePresent = false;
		DeviceList[i].Device = new TUSBDevice(i);
	}
}

/*
 * The desctruction of the device manager, which frees the device list.
 */
TUSBDeviceManager::~TUSBDeviceManager() {
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		delete DeviceList[i].Device;
	}

	/* Deinit libusb */
	libusb_exit(NULL);
}

void
TUSBDeviceManager::SetAddCallBack(void (*addfunc) (TUSBDevice*, void*), void *cont)
{
	onDevicePlugged   = addfunc;
	DeviceContext = cont;
}

void
TUSBDeviceManager::SetRemoveCallBack(void (*remfunc) (TUSBDevice*, void*), void *cont)
{
	onDeviceUnplugged   = remfunc;
	DeviceContext = cont;
}

bool
TUSBDeviceManager::HandleDeviceChange()
{
	bool something_changed = false;
	busy = true;

	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		/* Check if device is available */
		if(!DeviceList[i].Device->DeviceAvailable(i)) {
			/* Device removed, still listed */
			if (DeviceList[i].DevicePresent) {
				DeviceList[i].Device->StopDriver();
				RemoveDeviceFromList(i);
				something_changed = true;
				if ( onDeviceUnplugged != NULL)
					onDeviceUnplugged(DeviceList[i].Device, DeviceContext);
			}
		} else {
			/* New device found, but not listed yet */
			if (!DeviceList[i].DevicePresent) {
				if (DeviceList[i].Device->StartDriver()) {
					AddDeviceToList(i); /* add pointer to list */
					something_changed = true;
				}
			}
		}
	}

	busy = false;

	return something_changed;
}

bool
TUSBDeviceManager::IsBusy()
{
	return busy;
}

void
TUSBDeviceManager::ForceRefresh()
{
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		/* Device removed, but still listed */
		if (DeviceList[i].DevicePresent) {
			DeviceList[i].Device->StopDriver();
			RemoveDeviceFromList(i);
			if ( onDeviceUnplugged != NULL)
				onDeviceUnplugged(DeviceList[i].Device, DeviceContext);
		}
	}

	HandleDeviceChange();
}

/*
 * Makes the device present.
 */
bool
TUSBDeviceManager::AddDeviceToList(int index)
{
	DeviceList[index].DevicePresent = true;

	return true;
}

/*
 * Makes the device present.
 */
bool
TUSBDeviceManager::RemoveDeviceFromList(int index)
{
	DeviceList[index].DevicePresent = false;
	return true;
}

int
TUSBDeviceManager::GetNumberofDevices()
{
	int count = 0;

	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		if (DeviceList[i].DevicePresent)
			count++;
	}

	return count;
}

int
TUSBDeviceManager::GetMaxNumberofDevices()
{
	return MAX_USB_DEV_NUMBER;
}

void *
TUSBDeviceManager::GetDevice(int  Id)
{
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++)
	{
		/* Get first available */
		if (Id == -1) {
			if (DeviceList[i].DevicePresent)
				return DeviceList[i].Device;
		}

		if (DeviceList[i].Device->Id == Id)
			return DeviceList[i].Device;
	}

	return NULL;
}

void *
TUSBDeviceManager::GetDevice(int Id, int DevClass)
{
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		if (DeviceList[i].DevicePresent) {
			if (((Id == -1) || (DeviceList[i].Device->Id == Id)) && 
			    (DeviceList[i].Device->DeviceClass == DevClass)) {
				return (void*)(DeviceList[i].Device);
			}
		}
	}

	return NULL;
}

int
TUSBDeviceManager::DevToIndex(TUSBDevice *dev)
{
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		if (DeviceList[i].Device == dev) {
			return i;
		}
	}

	return -1;
}
int
TUSBDeviceManager::DevToId(TUSBDevice *dev)
{
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		if (DeviceList[i].Device == dev) {
			return DeviceList[i].Device->Id;
		}
	}

	return -1;
}
int
TUSBDeviceManager::UpdateDevStringList() {
	int count =0;
	std:: stringstream tmp;

	devInfoStrings.clear();

	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		if (DeviceList[i].DevicePresent) {
			count ++;
			tmp << (int)DeviceList[i].Device->GetId() << " " 
				<< " " << DeviceList[i].Device->GetName() << " "
				<< " " << (int)DeviceList[i].Device->GetClass() << " "
				<< (int)DeviceList[i].Device->GetFWVersion() << std::endl;
			//OutputDebugString((const char*) tmp.str().c_str());
		}
	}

	return count;
}

