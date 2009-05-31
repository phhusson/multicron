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

#define ALLOC_STEP 5
InotifyEvent::InotifyEvent() {
	rfds=(int*)malloc(sizeof(int)*2);
	rfds[0]=inotify_init();
	if( rfds[0] < 0 )
		throw "Couldn't create inotify fd";
	rfds[1]=-1;
	wfds=NULL;
	efds=NULL;

	name=strdup("inotify");
	inotify_files=(inotify_file*)malloc(sizeof(inotify_file)*ALLOC_STEP);
	memset(inotify_files, 0, sizeof(inotify_file)*ALLOC_STEP);
	n=0;
}

InotifyEvent::~InotifyEvent() {
	int i;
	for(i=0;inotify_files[i].filename;i++)
		free(inotify_files[i].filename);
	free(inotify_files);
	if(rfds) {
		if(rfds[0]>0)
			close(rfds[0]);
		free(rfds);
	}
	if(name)
		free(name);
}

void InotifyEvent::RefreshConfig(xmlNode config) {
	int inotify=rfds[0];
	while(!!config) {
		char *file=strdup(config["folder"]());
		if( file[strlen(file)-1]=='/')
			file[strlen(file)-1]=0;
		int ret;
		struct stat buf;
		if(lstat(file, &buf)) {
			free(file);
			return;
		}
		if(!S_ISDIR(buf.st_mode)) {
			free(file);
			return;
		}
	
		ret=inotify_add_watch(inotify, file, IN_CREATE|IN_CLOSE_WRITE|IN_MOVED_TO|IN_DELETE|IN_MOVED_FROM);
		if(ret==-1) {
			printf("%s\n", file);
			free(file);
			perror("Mise en place de la surveillance du fichier");
			exit(0);
		}
		++n;
		if((n%ALLOC_STEP)==0)
			inotify_files=(inotify_file*)realloc(inotify_files, (ret+ALLOC_STEP)*sizeof(char*));
		inotify_files[n-1].wd=ret;
		inotify_files[n-1].filename=file;
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
	char *path=NULL;
	while(i<ret) {
		const char *name = NULL;
		struct inotify_event *event;
		event = (struct inotify_event *) &buffer[i];
		if (event->len)
			name = &buffer[i] + sizeof (struct inotify_event);
		if(path) {
			free(path);
			path=NULL;
		}
		asprintf(&path, "%s/%s", inotify_files[event->wd].filename, name);
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
		int j;
		for(j=0;inotify_files[j].filename;j++) {
			if(inotify_files[j].wd==event->wd)
				break;
		}
		if(!inotify_files[j].filename) {
			printf("Couldn't find recorded watch descriptor!\n");
			continue;
		}
		char *filename=inotify_files[j].filename;
		while(!!node) {
			char *folder=strdup(node["folder"]());
			if(folder[strlen(folder)-1]=='/')
				folder[strlen(folder)-1]=0;
			struct context ctx;
			memset(&ctx, 0, sizeof(ctx));
			ctx.pid=0;
			ctx.file=path;
			if(strcmp(folder, filename)==0) {
				if(!node["file"]()) 
					Cmds::Call(node, ctx);
				else if(regexp_match(node["file"](), name))
					Cmds::Call(node, ctx);
			}
			free(folder);
			++node;
		}
	}
	if(path)
		free(path);
}

