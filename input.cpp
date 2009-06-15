#include <string.h>
#include "cfg.h"
#include "multicron.h"
#include "input.h"
#include "commands.h"

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <linux/input.h>

void InputEvent::CheckForEvent() {
	if(rfds)
		return;
	char *path;
	if(!cfg["devpath"]) {
		MainLoop::DelEM(this);
		return;
	}
	asprintf(&path, "/sys/%s", cfg["devpath"]);
	DIR *dir=opendir(path);
	free(path);
	path=NULL;
	if(!dir) {
		MainLoop::DelEM(this);
		return;
	}
	int fd;
	struct dirent *entry;
	while( (entry=readdir(dir))!=NULL) {
		if(entry->d_type!=DT_DIR)
			continue;
		if(strncmp(entry->d_name, "event", 5)==0) {
			asprintf(&path, "/dev/input/%s", entry->d_name);
			fd=open(path, O_RDONLY);
			if(fd>=0) {
				printf("GOTCHA\n");
				rfds=(int*)malloc(sizeof(int)*2);
				rfds[0]=fd;
				rfds[1]=-1;
				if(cfg["capture"])
					ioctl(fd, EVIOCGRAB, 1);
			}
			free(path);
			path=NULL;
			break;
		}
	}
	closedir(dir);
}

InputEvent::InputEvent(cfgNode conf) : cfg(conf) {
	printf("Called input creator!\n");
	rfds=NULL;
	wfds=NULL;
	efds=NULL;
	name=strdup("input");
	if(cfg["devpath"]==NULL) {
		printf("Got null devpath, delete myself\n");
		MainLoop::DelEM(this);
	}
	CheckForEvent();
}


void InputEvent::Callback(int fd, ETYPE event_type) {
	if(!rfds) {
		CheckForEvent();
		return;
	} 
	if(event_type==TIMEOUT)
		return;
	if(fd!=rfds[0]) {
		//Ouch
		throw "fd doesn't match our fds";
	}
	struct input_event ev;
	memset(&ev, 0, sizeof(ev));
	if(read(fd, &ev, sizeof(ev))<0) {
		MainLoop::DelEM(this);
		return;
	}
	if(ev.type==EV_KEY)
		printf("\tcode=%d\n", ev.code);
	cfgNode conf(cfg.getChild());
	while(!!conf) {
		if(strcmp(conf.getName(), "on")!=0)
			throw "Only 'on' nodes allowed in input configuration";
		if(!conf["key"]) {
			printf("Non-key input-event not supported atm\n");
			++conf;
			continue;
		}
		if(conf["key"]) {
			if(ev.type!=EV_KEY) {
				++conf;
				continue;
			}
			if(ev.code!=atoi(conf["key"])) {
				++conf;
				continue;
			}
			if(conf["value"])
				if(ev.value!=atoi(conf["value"])) {
					++conf;
					continue;
				}
			struct context ctx;
			bzero(&ctx, sizeof(ctx));
			//Anything to put in ctx?
			Cmds::Call(conf, ctx);
		}
		++conf;
	}
}

void InputEvent::RefreshConfig() {
	printf("Called InputEvent Refreshconfig ?!?\n");
}

struct timeval InputEvent::NextTimeout() {
	struct timeval tv;
	if(rfds) {
		tv.tv_sec=1e9;
		tv.tv_usec=0;
	} else {
		tv.tv_sec=0;
		tv.tv_usec=500*1000;
	}
	return tv;
}

InputEvent::~InputEvent() {
	printf("Called InputEvent destructor\n");
}
