/*(C) 2009 HUSSON Pierre-Hugues <phhusson@free.fr>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <linux/netlink.h>

#include <assert.h>
#include <ctype.h>

#include "cfg.h"
#include "multicron.h"
#include "commands.h"
#include "uevent.h"
#include "uevent/uevent.h"
#include "uevent/power.h"
#include "uevent/usb.h"
#include "uevent/input.h"

#define HOTPLUG_BUFFER_SIZE             1024
#define HOTPLUG_NUM_ENVP                32
#define OBJECT_SIZE                     512

#ifndef NETLINK_KOBJECT_UEVENT
#error Your kernel headers are too old, and do not define NETLINK_KOBJECT_UEVENT. You need Linux 2.6.10 or higher for KOBJECT_UEVENT support.
#endif

static int ueventInit() {
	//Return associated fd
	static int fd=-1;
	if(fd>=0)
		return fd;
	struct sockaddr_nl ksnl;
	memset(&ksnl, 0x00, sizeof(struct sockaddr_nl));
	ksnl.nl_family=AF_NETLINK;
	ksnl.nl_pid=getpid();
	ksnl.nl_groups=0xffffffff;
	fd=socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (fd==-1) {
		perror("Couldn't open kobject-uevent netlink socket");
		return -1;
	}
	if (bind(fd, (struct sockaddr *) &ksnl, sizeof(struct sockaddr_nl))<0) {
		perror("Error binding to netlink socket");
		close(fd);
		return -1;
	}
	return fd;
}

void UEvent::Callback(int fd, EventManager::ETYPE event_type) {
	if(event_type==EventManager::TIMEOUT)
		return;
	(void)event_type;
	char buffer[HOTPLUG_BUFFER_SIZE + OBJECT_SIZE];
	int buflen;
	int tot=0;
	buflen=recv(fd, &buffer, sizeof(buffer), 0);
	while(buffer[tot]!=0) ++tot;
	++tot;

	UEvents::Event *ev=new UEvents::Event;
	char *action_name=NULL;
	while(tot<buflen) {
#define ACT "ACTION="
		//Ignore "invalid" (mostly sent by udevd) messages
		if(!isalpha(*(buffer+tot)))
			break;
		printf("%s\n", buffer+tot);
		if(strncmp(buffer+tot, ACT, strlen(ACT))==0) {
			char *type=buffer+tot+strlen(ACT);
			action_name=strdup(type);
			if(strcmp(type, "add")==0)
				ev->action=UEvent::ADD;
			else if(strcmp(type, "remove")==0)
				ev->action=UEvent::REMOVE;
			else if(strcmp(type, "change")==0)
				ev->action=UEvent::CHANGE;
			else if(strcmp(type, "move")==0)
				ev->action=UEvent::MOVE;
			else if(strcmp(type, "online")==0)
				ev->action=UEvent::ONLINE;
			else if(strcmp(type, "offline")==0)
				ev->action=UEvent::OFFLINE;
			else
				ev->action=UEvent::UNKNOWN;//Shouldn't happen on 2.6.30-rc6
#define DEVPATH "DEVPATH="
		} else if(strncmp(buffer+tot, DEVPATH, strlen(DEVPATH))==0) {
			ev->devpath=strdup(buffer+tot+strlen(DEVPATH));
#define SUBSYS "SUBSYSTEM="
		} else if(strncmp(buffer+tot, SUBSYS, strlen(SUBSYS))==0) {
			ev->subsys=strdup(buffer+tot+strlen(SUBSYS));
			//Got subsystem ? Ok now we can look for an appropriate event handler.
			//ATM only power events and no search.
			if(strcmp(ev->subsys, "power_supply")==0) {
				UEvents::Event *tmp=ev;
				ev=new UEvents::Power(tmp);
				delete tmp;
			} else if(strcmp(ev->subsys, "usb")==0) {
				UEvents::Event *tmp=ev;
				ev=new UEvents::USB(tmp);
				delete tmp;
			} else if(strcmp(ev->subsys, "input")==0) {
				UEvents::Event *tmp=ev;
				ev=new UEvents::Input(tmp);
				delete tmp;
			} else {
				printf("Unsupported subsys\n");
			}

#define SEQNUM "SEQNUM="
		} else if(strncmp(buffer+tot, SEQNUM, strlen(SEQNUM))==0) {
			ev->seqnum=atoi(buffer+tot+strlen(SEQNUM));
		} else {
			char *name=buffer+tot;
			char *value=index(buffer+tot, '=');
			if(!value) {
				while(buffer[tot]!=0) ++tot;
				++tot;
				continue;
			}
			value[0]=0;
			value++;
			ev->SetVar(name, value);
			while(buffer[tot]!=0) ++tot;//Move twice as we added a new \0
			++tot;
		}
		while(buffer[tot]!=0) ++tot;
		++tot;
	}

	cfgNode node;
	int i;
	for(i=0;i<n_cfg;++i) {
		if(!cfg[i])
			continue;
		node=cfg[i][0];
		assert(strcmp(node.getName(), name)==0);
		if(node["subsystem"])
			if(!regexp_match(node["subsystem"], ev->subsys)) {
				++node;
				continue;
			}
		if(!node["subsystem"])
			throw "No subsystem attribute for uevent ? Are you crazy?";
		if(node["devpath"])
			if(!regexp_match(node["devpath"], ev->devpath)) {
				++node;
				continue;
			}
		if(node["action"])
			if(!regexp_match(node["action"], action_name)) {
				++node;
				continue;
			}
		if(!ev->Match(node)) {
			++node;
			continue;
		}
		struct context ctx;
		bzero(&ctx, sizeof(ctx));
		ev->FillCtx(ctx);
		//uevent infos would be usefull
		Cmds::Call(node, ctx);
		++node;
	}
	free(action_name);
	delete ev;
}

UEvent::UEvent() {
	rfds=(int*)malloc(sizeof(int)*2);
	rfds[0]=ueventInit();
	rfds[1]=-1;
	wfds=NULL;
	efds=NULL;

	name=strdup("uevent");
	cfg=NULL;
	n_cfg=0;
}

UEvent::~UEvent() {
	if(rfds) {
		free(rfds);
		rfds=NULL;
	}
	if(name) {
		free(name);
		name=NULL;
	}
	if(cfg) {
		int i;
		for(i=0;i<n_cfg;++i)
			if(cfg[i])
				delete cfg[i];
		free(cfg);
	}
}

extern "C" {
	void registerSelf() {
		MainLoop::AddEM(new UEvent);
	}
};
