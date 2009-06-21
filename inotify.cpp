#include <iostream>
#include "cfg.h"
#include "multicron.h"
#include "inotify.h"
#ifndef BSD
	extern "C" {
	#include <sys/inotify.h>
	};
#else
	#include <sys/event.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "commands.h"

#define ALLOC_STEP 5
#define NAME "inotify"
InotifyEvent::InotifyEvent() {
	rfds=(int*)malloc(sizeof(int)*2);
#ifdef BSD
	rfds[0]=kqueue();
#else
	rfds[0]=inotify_init();
#endif
	if( rfds[0] < 0 )
		throw "Couldn't create inotify fd";
	rfds[1]=-1;
	wfds=NULL;
	efds=NULL;

	name=strdup(NAME);
	inotify_files=(inotify_file*)malloc(sizeof(inotify_file)*ALLOC_STEP);
	memset(inotify_files, 0, sizeof(inotify_file)*ALLOC_STEP);
	cfg=NULL;
	n=0;
	n_cfg=0;
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
	if(cfg) {
		for(i=0;i<n_cfg;++i)
			if(cfg[i])
				delete cfg[i];
		free(cfg);
	}
}

void InotifyEvent::AddCfg(cfgNode conf) {
	int inotify=rfds[0];
	cfgNode config(conf);
	if(!conf)
		throw "Want to add a null config";
	if(strcmp(config.getName(), NAME)!=0)
		throw "Got a non inotify-node in inotify config";

	int i;
	//Is there place before the end?
	for(i=0;cfg && cfg[i];++i);
	if(i>=(n_cfg)) {
		//No ? Ok then make some
		cfg=(cfgNode**)realloc(cfg, (n_cfg+2)*sizeof(cfgNode*));
		cfg[n_cfg+1]=NULL;
		cfg[n_cfg]=new cfgNode(conf);
		++n_cfg;
	} else {
		//Yes ? then use it.
		cfg[i]=new cfgNode(conf);
	}

	if(!config["folder"])
		throw "Inotify need to know in which folder to wait";
	char *file=strdup(config["folder"]);
	if( file[strlen(file)-1]=='/')
		file[strlen(file)-1]=0;
	int ret;
	struct stat buf;
	if(lstat(file, &buf)) {
		free(file);
		return;
	}
	if(!S_ISDIR(buf.st_mode)) {
		//Maybe say to the user he is a f***ing noob ?
		free(file);
		return;
	}

#ifdef BSD
	int f;
	f=open(file, O_RDONLY);
	struct kevent change;
	EV_SET(&change, f, EVFILT_VNODE,
			//ONESHOT or not ?
		EV_ADD | EV_ENABLE | EV_ONESHOT,
		NOTE_DELETE | NOTE_EXTEND | NOTE_WRITE | NOTE_ATTRIB,
		0, (void*)n);
	if(kevent(rfds[0], &change, 1, NULL, 0, 0)<0) {
		perror("kevent");
		return;
	}
#else
	ret=inotify_add_watch(inotify, file, IN_CREATE|IN_CLOSE_WRITE|IN_MOVED_TO|IN_DELETE|IN_MOVED_FROM);
	if(ret==-1) {
		printf("%s\n", file);
		free(file);
		perror("Mise en place de la surveillance du fichier");
		exit(0);
	}
#endif
	++n;
	if((n%ALLOC_STEP)==0)
		inotify_files=(inotify_file*)realloc(inotify_files, (ret+ALLOC_STEP)*sizeof(char*));
#ifdef BSD
	inotify_files[n-1].fd=f;
	inotify_files[n-1].wd=f;
	inotify_files[n-1].filename=file;
	time(&(inotify_files[n-1].last));//Start watching from now.
#else
	inotify_files[n-1].wd=ret;
	inotify_files[n-1].filename=file;
#endif
}

#ifdef BSD
void InotifyEvent::Callback(int fd, EventManager::ETYPE event_type) {
	if(event_type==EventManager::TIMEOUT)
		return;
	struct kevent ev;
	int ret;
	ret=kevent(rfds[0], NULL, 0, &ev, 1, NULL);
	if(ret<0) {
		perror("kevent");
		return;
	}


	inotify_file file=inotify_files[(int)ev.udata];
	cfgNode node=cfg;
	while(!!node) {
		char *folder=strdup(node["folder"]());
		if(folder[strlen(folder)-1]=='/')
			folder[strlen(folder)-1]=0;
		struct context ctx;
		memset(&ctx, 0, sizeof(ctx));
		ctx.pid=0;
		if(strcmp(folder, file.filename)!=0) {
			free(folder);
			++node;
			continue;
		}
		if(!node["file"]()) {
			time(&(file.last));
			Cmds::Call(node, ctx);
			free(folder);
			++node;
			continue;
		}
		DIR *dir=opendir(folder);
		struct dirent *file_info=NULL;
		struct stat stat_buf;
		//Does asprintf exists on BSD ?
		char path[1024];
		while((file_info=readdir(dir))!=NULL) {
			if(!regexp_match(node["file"](), file_info->d_name))
				continue;
			bzero(&path, sizeof(path));
			snprintf(path, sizeof(path), "%s/%s", folder, file_info->d_name);
			path[sizeof(path)-1]=0;
			stat(path, &stat_buf);
			if(stat_buf.st_mtime<=file.last && stat_buf.st_ctime<=file.last)
				continue;
			//Ok, file is newer than last check.
			ctx.file=path;
			Cmds::Call(node, ctx);
		}
		time(&(file.last));
		free(folder);
		++node;
	}
}
#else
void InotifyEvent::Callback(int fd, EventManager::ETYPE event_type) {
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

		i += sizeof (struct inotify_event) + event->len;
		if(event->mask!=IN_CREATE && event->mask!=IN_MOVED_TO && event->mask!=IN_CLOSE_WRITE)
			continue;

		int j;
		for(j=0;inotify_files[j].filename;j++) {
			if(inotify_files[j].wd==event->wd)
				break;
		}
		if(!inotify_files[j].filename) {
			printf("Couldn't find recorded watch descriptor!\n");
			continue;
		}
		asprintf(&path, "%s/%s", inotify_files[j].filename, name);

		char *filename=inotify_files[j].filename;
		int i;
		for(i=0;i<n_cfg;++i) {
			cfgNode node=cfg[i][0];
			if(strcmp(node.getName(), "inotify")!=0) 
				throw "Got a non inotify-node in our config nodes !";
			char *folder=strdup(node["folder"]);
			if(folder[strlen(folder)-1]=='/')
				folder[strlen(folder)-1]=0;
			struct context ctx;
			memset(&ctx, 0, sizeof(ctx));
			ctx.pid=0;
			ctx.file=path;
			if(strcmp(folder, filename)==0) {
				if(!node["file"]) 
					Cmds::Call(node, ctx);
				else if(regexp_match(node["file"], name))
					Cmds::Call(node, ctx);
			}
			free(folder);
		}
	}
	if(path)
		free(path);
}
#endif

extern "C" {
	void registerSelf() {
		MainLoop::AddEM(new InotifyEvent);
	};
};
