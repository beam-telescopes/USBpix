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
//  08.10.12  HK  - libusb 1.0 support (libusb_windows_backend)
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

#include "SiUSBDeviceManager.h"
#include <stdexcept>

/*
 * This is the contstruction of the device manager. It creates the main
 * device list.
 */
using namespace std;

TUSBDeviceManager::TUSBDeviceManager()
{
	onDevicePlugged   = NULL;
	onDeviceUnplugged = NULL;
	DeviceContext     = NULL;

#ifdef _LIBUSB_	/* Init libusb */
	int err = libusb_init(&ctx);
	if (err<0)
	  throw std::runtime_error("Error from libusb_init - "+std::string(_libusb_strerror((libusb_error) err)));
	libusb_set_debug(ctx, 3);  // for debug
#endif
  
	/* Init device list */
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) 
	{
		DeviceList[i].DevicePresent = false;
		DeviceList[i].Device = new TUSBDevice(i);
	}
}

/*
 * The desctruction of the device manager, which frees the device list.
 */
TUSBDeviceManager::~TUSBDeviceManager() 
{
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		delete DeviceList[i].Device;
	}

#ifdef _LIBUSB_	/* Deinit libusb */
	libusb_exit(ctx);
#endif
}

void TUSBDeviceManager::SetAddCallBack(void (*addfunc) (TUSBDevice*, void*), void *cont)
{
	onDevicePlugged   = addfunc;
	DeviceContext = cont;
}

void TUSBDeviceManager::SetRemoveCallBack(void (*remfunc) (TUSBDevice*, void*), void *cont)
{
	onDeviceUnplugged   = remfunc;
	DeviceContext = cont;
}

bool TUSBDeviceManager::HandleDeviceChange()
{

  bool something_changed = false;

#ifdef _LIBUSB_

  for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
    // Check if device is available 
    if(!DeviceList[i].Device->DeviceAvailable(i, ctx)) {
      // Device removed, still listed 
      if (DeviceList[i].DevicePresent) {
	DeviceList[i].Device->StopDriver();
	RemoveDeviceFromList(i);
	something_changed = true;
	if ( onDeviceUnplugged != NULL)
	  onDeviceUnplugged(DeviceList[i].Device, DeviceContext);
      }
    } else {
      // New device found, but not listed yet 
      if (!DeviceList[i].DevicePresent) {
	if (DeviceList[i].Device->StartDriver()) {
	  AddDeviceToList(i); 
	  something_changed = true;
	}
      }
    }
  }
  

#else


    GUID guidDeviceInterface=GUID_DEVINTERFACE_SILAB_USB_DEVICE;
    BOOL bResult = TRUE;
    HDEVINFO hDeviceInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = NULL;
    ULONG requiredLength=0;
    LPTSTR lpDevicePath = NULL;
    DWORD index = 0;
		std::stringstream buf;

    // Get information about all the installed devices for the specified
    // device interface class.
    hDeviceInfo = SetupDiGetClassDevs( 
        &guidDeviceInterface,
				NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hDeviceInfo == INVALID_HANDLE_VALUE) 
    { 
        DebugOutLastError("Error SetupDiGetClassDevs:");
        goto done;
    }

		something_changed = PurgeDeviceList();  // remove devices which are not present

    //Enumerate all the device interfaces in the device information set.
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (index = 0; SetupDiEnumDeviceInfo(hDeviceInfo, index, &DeviceInfoData); index++)
    {
        //Reset for this iteration
        if (lpDevicePath)
            LocalFree(lpDevicePath);
        if (pInterfaceDetailData)
            LocalFree(pInterfaceDetailData);

        deviceInterfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

        //Get information about the device interface.
        bResult = SetupDiEnumDeviceInterfaces( 
           hDeviceInfo,
           &DeviceInfoData,
           &guidDeviceInterface,
           0, 
           &deviceInterfaceData);

        // Check if last item
        if (GetLastError () == ERROR_NO_MORE_ITEMS)
            break;

        //Check for some other error
        if (!bResult) 
        {
            DebugOutLastError("Error SetupDiEnumDeviceInterfaces:");
            goto done;
        }

        //Interface data is returned in SP_DEVICE_INTERFACE_DETAIL_DATA
        //which we need to allocate, so we have to call this function twice.
        //First to get the size so that we know how much to allocate
        //Second, the actual call with the allocated buffer
        
        bResult = SetupDiGetDeviceInterfaceDetail(
            hDeviceInfo,
            &deviceInterfaceData,
            NULL, 0,
            &requiredLength,
            NULL);


        //Check for some other error
        if (!bResult) 
        {
            if ((ERROR_INSUFFICIENT_BUFFER==GetLastError()) && (requiredLength>0))
            {
                //we got the size, allocate buffer
                pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, requiredLength);
                if (!pInterfaceDetailData) 
                { 
                    OutputDebugString("Error allocating memory for the device detail buffer.\n");
                    goto done;
                }
            }
            else
            {
                DebugOutLastError("Error SetupDiEnumDeviceInterfaces:");
                goto done;
            }
        }

        //get the interface detailed data
        pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        //Now call it with the correct size and allocated buffer
        bResult = SetupDiGetDeviceInterfaceDetail(
                hDeviceInfo,
                &deviceInterfaceData,
                pInterfaceDetailData,
                requiredLength,
                NULL,
                &DeviceInfoData);
        
        //Check for some other error
        if (!bResult) 
        {
            DebugOutLastError("Error SetupDiGetDeviceInterfaceDetail:");
            goto done;
        }

        //copy device path
        size_t nLength = strlen (pInterfaceDetailData->DevicePath) + 1;  
        lpDevicePath = (TCHAR *) LocalAlloc (LPTR, nLength * sizeof(TCHAR));
        StringCchCopy(lpDevicePath, nLength, pInterfaceDetailData->DevicePath);
        lpDevicePath[nLength-1] = 0;
                        
				DBGOUT("Device "<< index << ": path: " << lpDevicePath << endl);

				// check is path name is already in use (device started)
				for (int i = 0; i < MAX_USB_DEV_NUMBER; i++)
				{
					if (DeviceList[i].Device->DevicePathName == std::string(lpDevicePath))
					{
					  DBGOUT("Device enumerated as " << i << " already listed" << endl);
						break; // device already listed, do nothing
					}
					else // device not listed yet, find unsued device list index and start driver
					{
  				  DBGOUT("Device enumerated as " << i << " not yet listed" << endl);
            int idx = GetFreeDeviceListIndex();
						DeviceList[idx].Device->DevicePathName = std::string(lpDevicePath);
						if (DeviceList[idx].Device->StartDriver())
						{
	  				  DBGOUT("Device " << i << " driver started, listed in index " << idx << endl);
						  DeviceList[idx].DevicePresent = true;
						  something_changed = true;									
						}
						else  // something went wrong
						{
	  				  DBGOUT("Could not start driver for device " << i << " at list index " << idx << endl);
							DeviceList[idx].DevicePathName = "";
						}
					break;
					}// find unused index
				}// iterate device list
    }

done:
    LocalFree(lpDevicePath);
    LocalFree(pInterfaceDetailData);    
    bResult = SetupDiDestroyDeviceInfoList(hDeviceInfo);

		if (index != 0)
		{
			std::stringstream buf;
			buf << (int) index << " device(s) found" << endl;
			OutputDebugString(buf.str().c_str());
		}
		else
		  OutputDebugString("No devices found.\n");
    
#endif
    return something_changed;
}

int  TUSBDeviceManager::GetFreeDeviceListIndex(void)
{
	// check is device index is used
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++)
	{
		if (!DeviceList[i].DevicePresent)
		{
		  DBGOUT("Device list index " << i << " is free" << endl);
			return i; // device already listed, do nothing
		}
	}
	return -1;
}

bool TUSBDeviceManager::PurgeDeviceList(void)
{
	std::stringstream buf;
	bool device_removed = false;
  
	buf << "TUSBDeviceManager::PurgeDeviceList:" << endl;
  
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++)
	{
		buf << "device at index " << i << ": ";
		if (DeviceList[i].DevicePresent)
		{
#ifdef _LIBUSB_
		        if(!DeviceList[i].Device->DeviceAvailable(i, ctx)) // a bit ugly, but consistent with libusb needs...
#else
			if(!DeviceList[i].Device->DeviceAvailable())	// clean device list from abandoned entries
#endif
			{
				buf << "has been removed" << endl;
				DeviceList[i].DevicePresent = false; //RemoveDeviceFromList[i];
				DeviceList[i].Device->StopDriver();
				DeviceList[i].Device->DevicePathName="" ;
        device_removed = true;
			}
			else
				buf << "is present" << endl;
		}
		else
			buf << "is not available" << endl;
	}
	OutputDebugString(buf.str().c_str());
	return device_removed;
}


void TUSBDeviceManager::ForceRefresh()
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
bool TUSBDeviceManager::AddDeviceToList(int index)
{
	DeviceList[index].DevicePresent = true;

	return true;
}

/*
 * Makes the device present.
 */
bool TUSBDeviceManager::RemoveDeviceFromList(int index)
{
	DeviceList[index].DevicePresent = false;
	return true;
}

int  TUSBDeviceManager::GetNumberofDevices()
{
	int count = 0;

	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		if (DeviceList[i].DevicePresent)
			count++;
	}

	return count;
}

int  TUSBDeviceManager::GetMaxNumberofDevices()
{
	return MAX_USB_DEV_NUMBER;
}

void *TUSBDeviceManager::GetDevice(int  Id)
{
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++)
	{
		if (Id == -1) // ID not specified, get next available device
		{
			if (DeviceList[i].DevicePresent)
				return DeviceList[i].Device;
		}
		else //  look for ID
		if (DeviceList[i].Device->Id == Id)
			return DeviceList[i].Device;
	}
	return NULL;
}

void *TUSBDeviceManager::GetDevice(int Id, int DevClass)
{
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		if (DeviceList[i].DevicePresent) {
			if (((Id == -1) || (DeviceList[i].Device->Id == Id)) && 
			    (DeviceList[i].Device->DeviceClass == (unsigned int)DevClass)) {
				return (void*)(DeviceList[i].Device);
			}
		}
	}

	return NULL;
}

int  TUSBDeviceManager::DevToIndex(TUSBDevice *dev)
{
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		if (DeviceList[i].Device == dev) {
			return i;
		}
	}

	return -1;
}

int  TUSBDeviceManager::DevToId(TUSBDevice *dev)
{
	for (int i = 0; i < MAX_USB_DEV_NUMBER; i++) {
		if (DeviceList[i].Device == dev) {
			return DeviceList[i].Device->Id;
		}
	}

	return -1;
}

int  TUSBDeviceManager::UpdateDevStringList() {
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
