#include <iostream>
#include "xml.h"
#include "multicron.h"
#include "inotify.h"
#include <pcreposix.h>
extern "C" {
#include <sys/inotify.h>
#include <string.h>
};
#include "commands.h"

InotifyEvent::InotifyEvent() {
	rfds=(int*)malloc(sizeof(int)*2);
	rfds[0]=inotify_init();
	if( rfds[0] < 0 )
		throw "Couldn't create inotify fd";
	rfds[1]=-1;
	wfds=NULL;
	efds=NULL;

	name=strdup("inotify");
	inotify_files=(char**)malloc(sizeof(char*)*5);
	memset(inotify_files, 0, sizeof(char*)*5);
	
}

void InotifyEvent::RefreshConfig(xmlNode config) {
	int inotify=this->rfds[0];
	while(!!config) {
		char *file=strdup(config["folder"]());
		if( file[strlen(file)-1]=='/')
			file[strlen(file)-1]=0;
		int ret;
		struct stat buf;
		if(lstat(file, &buf))
			return;
		if(!S_ISDIR(buf.st_mode))
			return;
	
		ret=inotify_add_watch(inotify, file, IN_CREATE|IN_CLOSE_WRITE|IN_MOVED_TO|IN_DELETE|IN_MOVED_FROM);
		if(ret==-1) {
			printf("%s\n", file);
			perror("Mise en place de la surveillance du fichier");
			exit(0);
		}
		if((ret%5)==0)
			inotify_files=(char**)realloc(inotify_files, (ret+5)*sizeof(char*));
		inotify_files[ret]=file;
		return;
	}
}

void InotifyEvent::Callback(xmlNode config, int fd, EventManager::ETYPE event_type) {
	if(event_type==EventManager::TIMEOUT)
		return;
	char buffer[4096];
	int ret;
	int i=0;
	
	ret=read(this->rfds[0], buffer, 4096);
	if(ret<0)
		perror("read");
	if(ret==0)
		sleep(5);
	while(i<ret) {
		const char *name = NULL;
		struct inotify_event *event;
		event = (struct inotify_event *) &buffer[i];
		if (event->len)
			name = &buffer[i] + sizeof (struct inotify_event);
		char *path;
		asprintf(&path, "%s/%s", inotify_files[event->wd],name);
		//Translate the inotify_event structure into a human comprehensible display
		/*
		if(event->mask==IN_MOVED_FROM || event->mask==IN_DELETE) {
			printf("%s was deleted\n", path);
		}
		else if(event->mask==IN_CREATE || event->mask==IN_MOVED_TO) {
			printf("%s was created\n", path);
		} else if(event->mask==IN_DELETE_SELF) {
			printf("%s was deleted\n", inotify_files[event->wd]);
		} else if(event->mask==IN_CLOSE_WRITE) {
			printf("%s was modified\n", path);
		}*/

		i += sizeof (struct inotify_event) + event->len;
		if(event->mask!=IN_CREATE && event->mask!=IN_MOVED_TO && event->mask!=IN_CLOSE_WRITE)
			continue;

		xmlNode node(config);
		while(!!node) {
			char *folder=strdup(node["folder"]());
			if(folder[strlen(folder)-1]=='/')
				folder[strlen(folder)-1]=0;
			struct context ctx;
			memset(&ctx, 0, sizeof(ctx));
			ctx.pid=0;
			ctx.file=path;
			if(strcmp(folder, inotify_files[event->wd])==0) {
				if(!node["file"]()) 
					cmdCall(node, ctx);
				else if(regexp_match(node["file"](), name))
					cmdCall(node, ctx);
			}
			free(folder);
			++node;
		}
	}
}

InotifyEvent::~InotifyEvent() {
	int i;
	for(i=0;inotify_files[i];i++)
		free(inotify_files[i]);
	free(inotify_files);
}
