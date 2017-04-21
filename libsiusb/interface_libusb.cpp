#include "interface_libusb.h"
#include "interface_libusb_ioctl.h"

/* Global Variables */
BULK_IOCTL  bulk_ioctl; /* Contains the bulk structure */
static int ENABLE_ALARM = 0;
int* USBDeviceH;

/*
 * Wrapper for Windows message
 */
void
ShowMessage(std::string  msg)
{
	printf("****************************************************\n");
	printf("*  MESSAGE:: %s \n",msg.c_str());
	printf("****************************************************\n");
}

/*
 * Wrapper for Windows message
 */
void ShowLastError(const char *msg)
{
#ifndef WIN32
	printf("%s", msg);
#else
	MessageBox(NULL, msg, "Error", MB_OK|MB_ICONINFORMATION);
#endif
};

/*
 * Set alarm level
 */
void
usb_alarm_enable(int m)
{ 
	ENABLE_ALARM=m;
}

/* 
 * The USB verbose output
 */
void
usb_alarm()
{ 
	BULK_TRANSFER_CONTROL* Ctrl_tmp;
	time_t tm;
	time(&tm);

	printf(" ***************************************\n"); 
	printf(" *** USB ALARM *** %s \n",ctime(&tm)); 
	printf(" *** USB ALARM *** USB=%p \n",USBDeviceH); 
	printf(" *** USB ALARM *** IOCTL=%#X\n",bulk_ioctl.IOCTL_SLBUSB); 

	Ctrl_tmp=(BULK_TRANSFER_CONTROL*)bulk_ioctl.Control;

	printf(" *** USB ALARM *** pipeNum=%lu\n",Ctrl_tmp->pipeNum); 
	printf(" *** USB ALARM *** Buffer size=%u\n",bulk_ioctl.sizeBuffer);
	printf(" ***************************************\n"); 
}

/* 
 * This is the main wrapper function, which takes the calls from the 
 * userland library.
 */
bool
DeviceIoControl(HANDLE USBDeviceHandle, int IOCTL_SLBUSB, void *Control, int sizeControl,
		void *ioBuffer, int sizeBuffer, unsigned long *doneBytes, void */*dummyNULL*/)
{
	int result, pipe, bpointer = 0;
	unsigned char epa;
	int transferred = 0;
	int ioctl_RD, ioctl_WR, ioctl_DL,ioctl_VR;
	BULK_TRANSFER_CONTROL* Ctrl;
	VENDOR_REQUEST_IN* Req;
	ioctl_RD=IOCTL_SLBUSB_BULK_READ;
	ioctl_WR=IOCTL_SLBUSB_BULK_WRITE;
	ioctl_DL=IOCTL_SLBUSB_ANCHOR_DOWNLOAD;
	ioctl_VR=IOCTL_SLBUSB_VENDOR_REQUEST;

	/* unused, so commented */
	//dummyNULL = NULL;

	/* Pipe number */
	Ctrl=(BULK_TRANSFER_CONTROL*)Control;
	pipe=Ctrl->pipeNum;


	/* Enable alarm function, for verbose output */
	//if (ENABLE_ALARM) alarm(5);

	/* Check what request we got and act on it */
	if (IOCTL_SLBUSB == ioctl_RD || IOCTL_SLBUSB == ioctl_WR) {
		bulk_ioctl.IOCTL_SLBUSB=IOCTL_SLBUSB;
		bulk_ioctl.Control=Control;
		bulk_ioctl.sizeControl=sizeControl;

		/* Find the endpoint address */
		if ((epa = findepa(pipe, USBDeviceHandle)) == 255) {
			return 0;
		}

		result = libusb_bulk_transfer(USBDeviceHandle, epa,
				 (unsigned char *) ioBuffer, sizeBuffer, &transferred, 0);

		if (result) {
			perror("Error: libusb_bulk_transfer");
			return 0;
		}

		*doneBytes = transferred;

		return true;

	} else if (IOCTL_SLBUSB == ioctl_DL) {
		/* Have to split the transfer into several parts, because
		 * of WinUSB limitations of 4096 bytes.
		 */
		do {

			result = libusb_control_transfer(USBDeviceHandle, (USB_DIR_OUT | USB_TYPE_VENDOR),
					 0xA0, bpointer, 0, ((unsigned char *) (Control)) + bpointer, (uint16_t) (1024 * 4), 0);

			bpointer += (1024 * 4);

		} while ((bpointer < sizeControl) && (result >= 0));

		if (result < 0) {
			printf("Error: %i \n", result);
			perror("Error: libusb_control_transfer");
			return 0;
		}

	} else if (IOCTL_SLBUSB == ioctl_VR) {
		Req = (VENDOR_REQUEST_IN *) Control;
		unsigned char data = Req->bData;
		unsigned char req_type;

		if (Req->direction) {
			req_type = USB_DIR_IN | USB_TYPE_VENDOR;
		} else {
			req_type = USB_DIR_OUT | USB_TYPE_VENDOR;
		}

		result = libusb_control_transfer(USBDeviceHandle, req_type,
				 Req->bRequest, Req->wValue, Req->wIndex, &data, Req->wLength, 0);

		if (result < 0) {
			printf("Error: %i \n", result);
			perror("Error: libusb_bulk_transfer");
			return 0;
		}

	} else {
		if ((epa = findepa(pipe, USBDeviceHandle)) == 255) {
			return 0;
		}

		result = libusb_bulk_transfer(USBDeviceHandle, epa,
				 (unsigned char *) ioBuffer, sizeBuffer, &transferred, 0);

		if (result) {
			perror("Error: libusb_bulk_transfer");
			return 0;
		}
	}

	*doneBytes = transferred;

	return true;

	//if (ENABLE_ALARM) alarm(0);
  return !(result == 0);
}

/*
 * Find endpoint address to a given pipe.
 */
unsigned char
findepa (int pipe, HANDLE USBDeviceHandle)
{
	libusb_config_descriptor *config;
	const libusb_interface *inter;
	const libusb_interface_descriptor *interdesc;
	const libusb_endpoint_descriptor *epdesc;

	/* Get Configuration descriptor */
	if(libusb_get_active_config_descriptor(libusb_get_device(USBDeviceHandle), &config)) {
		perror("Error: get_config_descriptor");

		return 255; 
	}

	/* Get interface and its description */
	inter = &config->interface[0];
	interdesc = &inter->altsetting[0];

	/* Check for pipe size */
	if (pipe >= interdesc->bNumEndpoints) {
		printf("Error: Pipe number larger than number of endpoints\n");

    libusb_free_config_descriptor(config);
		return 255;
	}

	epdesc = &interdesc->endpoint[pipe];
  unsigned char res = (epdesc->bEndpointAddress);

  libusb_free_config_descriptor(config);
	return res;
}

/*
 * hardcoded endpoint addresses
 */
//unsigned char
//findepa (int pipe, HANDLE USBDeviceHandle)
//{
//	unsigned char endpoint[] = {0x1, 0x81, 0x2, 0x86, 0x4, 0x88};
//
//	return endpoint[pipe];
//}
