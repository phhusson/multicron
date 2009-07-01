#include <string.h>
#include <stdlib.h>
#include "../cfg.h"
#include "../multicron.h"
#include "../uevent.h"
#include "uevent.h"
#include "power.h"

static const char *status_text[] = {
	"Unknown", "Charging", "Discharging", "Not charging", "Full"
};
static const char *health_text[] = {
	"Unknown", "Good", "Overheat", "Dead", "Over voltage",
	"Unspecified failure", "Cold",
};
static const char *technology_text[] = {
	"Unknown", "NiMH", "Li-ion", "Li-poly", "LiFe", "NiCd",
	"LiMn"
};
static const char *type_text[] = { "Battery", "UPS", "Mains", "USB" };

UEvents::Power::Power(UEvents::Event *ev) {
	subsys=strdup(ev->subsys);//Well, it will be power_supply... I hope.
	if(strcmp(subsys, "power_supply")!=0)
		throw "UEvents::Power constructor for non power_supply subsystem";
	devpath=strdup(ev->devpath);
	seqnum=ev->seqnum;
	action=ev->action;

	power_name=NULL;
	//type= ?
	status=UNKNOWN_STATUS;
	technology=UNKNOWN_TECHNOLOGY;
	voltage_min_design=-1;
	voltage_now=-1;
	charge_full_design=-1;
	charge_full=-1;
	charge_now=-1;
	model_name=NULL;
	manufacturer=NULL;
	serialnum=NULL;

}

UEvents::Power::Power() {
	devpath=NULL;
	subsys=NULL;
	seqnum=-1;
	action=::UEvent::UNKNOWN;

	power_name=NULL;
	//type= ?
	status=UNKNOWN_STATUS;
	technology=UNKNOWN_TECHNOLOGY;
	voltage_min_design=-1;
	voltage_now=-1;
	charge_full_design=-1;
	charge_full=-1;
	charge_now=-1;
	model_name=NULL;
	manufacturer=NULL;
	serialnum=NULL;

}

UEvents::Power::~Power() {
	Display();
	if(power_name) {
		free(power_name);
		power_name=NULL;
	}
	if(model_name) {
		free(model_name);
		model_name=NULL;
	}
	if(manufacturer) {
		free(manufacturer);
		manufacturer=NULL;
	}
	if(serialnum) {
		free(serialnum);
		serialnum=NULL;
	}
}

void UEvents::Power::SetVar(const char *name, const char *value) {
	unsigned int i;
	if(strcmp(name, "POWER_SUPPLY_NAME")==0) {
		power_name=strdup(value);
	} else if(strcmp(name, "POWER_SUPPLY_TYPE")==0) {
		for(i=0;i<(sizeof(type_text)/sizeof(*type_text));i++)
			if(strcmp(type_text[i], value)==0)
				type=(UEvents::Power::power_type)i;
	} else if(strcmp(name, "POWER_SUPPLY_PRESENT")==0) {
		present=atoi(value)==1 ? true : false;
	} else if(strcmp(name, "POWER_SUPPLY_TECHNOLOGY")==0) {
		for(i=0;i<(sizeof(technology_text)/sizeof(*technology_text));i++)
			if(strcmp(technology_text[i], value)==0)
				technology=(UEvents::Power::power_technology)i;
	} else if(strcmp(name, "POWER_SUPPLY_VOLTAGE_MIN_DESIGN")==0) {
		voltage_min_design=atoi(value);
	} else if(strcmp(name, "POWER_SUPPLY_VOLTAGE_NOW")==0) {
		voltage_now=atoi(value);
	} else if(strcmp(name, "POWER_SUPPLY_CURRENT_NOW")==0) {
		current_now=atoi(value);
	} else if(strcmp(name, "POWER_SUPPLY_FULL_CHARGE_DESIGN")==0) {
		charge_full_design=atoi(value);
	} else if(strcmp(name, "POWER_SUPPLY_CHARGE_FULL")==0) {
		charge_full=atoi(value);
	} else if(strcmp(name, "POWER_SUPPLY_CHARGE_NOW")==0) {
		charge_now=atoi(value);
	} else if(strcmp(name, "POWER_SUPPLY_MODEL_NAME")==0) {
		model_name=strdup(value);
	} else if(strcmp(name, "POWER_SUPPLY_MANUFACTURER")==0) {
		manufacturer=strdup(value);
	} else if(strcmp(name, "POWER_SUPPLY_SERIAL_NUMBER")==0) {
		serialnum=strdup(value);
	}
}

void UEvents::Power::Display() {
	printf("Battery at %f%%\n", (100.0*charge_now)/(1.0*charge_full));
}

bool UEvents::Power::Match(cfgNode cfg) {
	if(cfg["type"]) {
		if(strcmp(cfg["type"], "battery")==0) {
			if(type!=BATTERY)
				return false;
		} else if(strcmp(cfg["type"], "ac")==0) {
			if(type!=MAINS)
				return false;
		} else
			return false;
	}
	return true;

}
