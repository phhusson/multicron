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
#include <pcreposix.h>

#include <string>

#include "xml.h"
#include "multicron.h"
#include "commands.h"
#include "uevent.h"

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

void UEvent::Callback(xmlNode config, int fd, EventManager::ETYPE event_type) {
	if(event_type==EventManager::TIMEOUT)
		return;
	(void)event_type;
	char buffer[HOTPLUG_BUFFER_SIZE + OBJECT_SIZE];
	int buflen;
	int tot=0;
	buflen=recv(fd, &buffer, sizeof(buffer), 0);
	while(buffer[tot]!=0) ++tot;
	++tot;

	printf("\n");
	struct uev uev;
	bzero(&uev, sizeof(uev));
	while(tot<buflen) {
#define ACT "ACTION="
		printf("%s\n", buffer+tot);
		if(strncmp(buffer+tot, ACT, strlen(ACT))==0) {
			char *type=buffer+tot+strlen(ACT);
			if(strcmp(type, "add")==0)
				uev.action=UEvent::ADD;
			else if(strcmp(type, "remove")==0)
				uev.action=UEvent::REMOVE;
			else if(strcmp(type, "change")==0)
				uev.action=UEvent::CHANGE;
			else if(strcmp(type, "move")==0)
				uev.action=UEvent::MOVE;
			else if(strcmp(type, "online")==0)
				uev.action=UEvent::ONLINE;
			else if(strcmp(type, "offline")==0)
				uev.action=UEvent::OFFLINE;
			else
				uev.action=UEvent::UNKNOWN;//Shouldn't happen on 2.6.30-rc6
#define DEVPATH "DEVPATH="
		} else if(strncmp(buffer+tot, DEVPATH, strlen(DEVPATH))==0) {
			uev.devpath=strdup(buffer+tot+strlen(DEVPATH));
#define SUBSYS "SUBSYSTEM="
		} else if(strncmp(buffer+tot, SUBSYS, strlen(SUBSYS))==0) {
			uev.subsys=strdup(buffer+tot+strlen(SUBSYS));
#define SEQNUM "SEQNUM="
		} else if(strncmp(buffer+tot, SEQNUM, strlen(SEQNUM))==0) {
			uev.seqnum=atoi(buffer+tot+strlen(SEQNUM));
		}
		while(buffer[tot]!=0) ++tot;
		++tot;
	}
	printf("uev = %d:%s:%s:%d\n", uev.action, uev.devpath, uev.subsys, seqnum);
	printf("\n");
}

UEvent::UEvent() {
	rfds=(int*)malloc(sizeof(int)*2);
	rfds[0]=ueventInit();
	rfds[1]=-1;
	wfds=NULL;
	efds=NULL;

	name=strdup("cnproc");
}

UEvent::~UEvent() {
	if(rfds) {
		free(rfds);
	}
	if(name)
		free(name);
}
