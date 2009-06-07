#include <string.h>
#include <stdlib.h>
#include "../xml.h"
#include "../multicron.h"
#include "../uevent.h"
#include "uevent.h"
#include "usb.h"

UEvents::USB::USB(UEvents::Event *ev) {
	Zero();
	subsys=strdup(ev->subsys);//Well, it will be usb... I hope.
	if(strcmp(subsys, "usb")!=0)
		throw "UEvents::USB constructor for non usb subsystem";
	devpath=strdup(ev->devpath);
	seqnum=ev->seqnum;
	action=ev->action;
}

UEvents::USB::USB() {
	Zero();
}

void UEvents::USB::Zero() {
	devpath=NULL;
	subsys=NULL;
	seqnum=-1;
	action=::UEvent::UNKNOWN;
	
	bInterfaceClass=0;
	bInterfaceSubClass=0;
	bInterfaceProtocol=0;
	idVendor=0;
	bcdDevice=0;
	bDeviceClass=0;
	bDeviceSubClass=0;
	bDeviceProtocol=0;
}

UEvents::USB::~USB() {
	Display();
	if(devpath)
		free(devpath);
	if(subsys)
		free(subsys);
	if(devtype)
		free(devtype);
}

void UEvents::USB::SetVar(const char *name, const char *value) {
	if(strcmp(name, "DEVTYPE")==0) {
		devtype=strdup(value);
	} else if(strcmp(name, "DEVICE")==0) {
		sscanf(value, "/proc/bus/usb/%d/%d", &busid, &devid);
	} else if(strcmp(name, "PRODUCT")==0) {
		sscanf(value, "%04x/%04x/%04x", &idVendor, &idProduct, &bcdDevice);
	} else if(strcmp(name, "TYPE")==0) {
		sscanf(value, "%d/%d/%d", &bDeviceClass, &bDeviceSubClass, &bDeviceProtocol);
	} else if(strcmp(name, "INTERFACE")==0) {
		sscanf(value, "%d/%d/%d", &bInterfaceClass, &bInterfaceSubClass, &bInterfaceProtocol);
	} else if(strcmp(name, "MODALIAS")==0) {
		sscanf(value,
			"usb:"
			"v%04Xp%04Xd%04Xdc%02Xdsc%02Xdp%02Xic%02Xisc%02Xip%02X",
			&idVendor,
			&idProduct,
			&bcdDevice,
			&bDeviceClass,
			&bDeviceSubClass,
			&bDeviceProtocol,
			&bInterfaceClass,
			&bInterfaceSubClass,
			&bInterfaceProtocol);
	}
}

bool UEvents::USB::Match(xmlNode config) {
	if(config["vid"]()) {
		char vid[6];//5 should be enough
		sprintf(vid, "%04x", idVendor);
		if(!regexp_match(config["vid"](), vid))
			return false;
	}
	if(config["pid"]()) {
		char pid[6];//5 should be enough
		sprintf(pid, "%04x", idProduct);
		if(!regexp_match(config["pid"](), pid))
			return false;
	}
	if(config["devtype"]())
		if(!regexp_match(config["devtype"](), devtype))
			return false;
	return true;
}

void UEvents::USB::Display() {
	printf("product=%x/%x/%x\n", idVendor, idProduct, bcdDevice);
}
