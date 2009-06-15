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
		throw "UEvents::Input constructor for non input subsystem";
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

void UEvents::Input::SetVar(const char *arg_name, const char *value) {
	if(strcmp(arg_name, "NAME")==0) {
		name=strdup(value+1);
		name[strlen(value)-2]=0;
	} else if(strcmp(arg_name, "PHYS")==0) {
		phys=strdup(value+1);
		phys[strlen(value)-2]=0;
	} else if(strcmp(arg_name, "PRODUCT")==0) {
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
	printf("%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	printf("product=%s\n", product);
	printf("name=%s\n", name);
	printf("phys=%s\n", phys);
	printf("\n");
}

void UEvents::Input::FillCtx(struct context &ctx) {
	printf("Called FillCtx!\n");
	ctx.devpath=strdup(devpath);
}
