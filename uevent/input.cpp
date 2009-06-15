#include <string.h>
#include <stdlib.h>
#include "../cfg.h"
#include "../commands.h"
#include "../multicron.h"
#include "../uevent.h"
#include "uevent.h"
#include "input.h"

UEvents::Input::Input(UEvents::Event *ev) {
	Zero();
	subsys=strdup(ev->subsys);//Well, it will be input... I hope.
	if(strcmp(subsys, "input")!=0)
		throw "UEvents::Input constructor for non usb subsystem";
	devpath=strdup(ev->devpath);
	seqnum=ev->seqnum;
	action=ev->action;
}

UEvents::Input::Input() {
	Zero();
}

void UEvents::Input::Zero() {
	devpath=NULL;
	subsys=NULL;
	seqnum=-1;
	action=::UEvent::UNKNOWN;
	
	product=NULL;
	name=NULL;
	phys=NULL;
}

UEvents::Input::~Input() {
	Display();
	if(devpath) {
		free(devpath);
		devpath=NULL;
	}
	if(subsys) {
		free(subsys);
		subsys=NULL;
	}
	if(product) {
		free(product);
		product=NULL;
	}
	if(name) {
		free(name);
		name=NULL;
	}
	if(phys) {
		free(phys);
		phys=NULL;
	}
}

void UEvents::Input::SetVar(const char *name, const char *value) {
	if(strcmp(name, "NAME")==0) {
		name=strdup(value);
	} else if(strcmp(name, "PHYS")==0) {
		phys=strdup(value);
	} else if(strcmp(name, "PRODUCT")==0) {
		product=strdup(value);
	}
}

bool UEvents::Input::Match(cfgNode config) {
	if(config["name"])
		if(!regexp_match(config["name"], name))
			return false;
	if(config["product"])
		if(!regexp_match(config["product"], product))
			return false;
	if(config["phys"])
		if(!regexp_match(config["phys"], phys))
			return false;
	return true;
}

void UEvents::Input::Display() {
	printf("product=%s\n", product);
}

void UEvents::Input::FillCtx(struct context &ctx) {
	ctx.devpath=strdup(devpath);
}
