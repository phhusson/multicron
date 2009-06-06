#include <string.h>
#include <stdlib.h>
#include "../xml.h"
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

UEvents::Power::Power() {
	name=NULL;
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
	if(name)
		free(name);
	if(model_name)
		free(model_name);
	if(manufacturer)
		free(manufacturer);
	if(serialnum)
		free(serialnum);
}

void UEvents::Power::SetVar(const char *name, const char *value) {
	unsigned int i;
	if(strcmp(name, "POWER_SUPPLY_NAME")==0) {
		name=strdup(value);
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
