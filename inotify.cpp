#include <iostream>
#include "xml.h"
#include "multicron.h"
#include <pcreposix.h>
extern "C" {
#include <sys/inotify.h>
#include <string.h>
};
#include "commands.h"

static void inotify_cb(xmlNode config);
static void inotify_conf(xmlNode config);
static char **inotify_files;
struct event_manager *inotify_module() {
	static struct event_manager *ret=NULL;
	if(ret!=NULL)
		return ret;
	ret=new event_manager;

	ret->fd=inotify_init();
	ret->callback=inotify_cb;
	ret->name=strdup("inotify");
	ret->refresh_config=inotify_conf;
	inotify_files=(char**)malloc(sizeof(char*)*5);
	memset(inotify_files, 0, sizeof(char*)*5);

	return ret;
}

static void inotify_conf(xmlNode config) {
	int inotify=inotify_module()->fd;
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

static void inotify_cb(xmlNode config) {
	char buffer[4096];
	int ret;
	int i=0;
	//Now read the device by big blocks
	ret=read(inotify_module()->fd, buffer, 4096);
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
		if(event->mask==IN_MOVED_FROM || event->mask==IN_DELETE) {
			printf("%s was deleted\n", path);
		}
		else if(event->mask==IN_CREATE || event->mask==IN_MOVED_TO) {
			printf("%s was created\n", path);
		} else if(event->mask==IN_DELETE_SELF) {
			printf("%s was deleted\n", inotify_files[event->wd]);
		} else if(event->mask==IN_CLOSE_WRITE) {
			printf("%s was modified\n", path);
		}

		if(event->mask!=IN_CREATE && event->mask!=IN_MOVED_TO && event->mask!=IN_CLOSE_WRITE)
			continue;
		i += sizeof (struct inotify_event) + event->len;

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
				regex_t preg;
				if(node["match"]()) {
					int ret=regcomp(&preg, node["match"](), REG_UTF8);
					regmatch_t match;
					if(ret!=0) {
						char *errbuf=(char*)malloc(512);
						memset(errbuf,0,512);
						regerror(ret, &preg, errbuf, 512);
						throw std::string("Pattern won't compile :")+errbuf;
					}
					ret=regexec(&preg, name, 1, &match, 0);
					if(ret==0 && match.rm_so==0 && match.rm_eo==strlen(name)) {
						printf("%s matched !!!\n", path);
						cmdCall(node, ctx);
					} 
				} else if(node["file"]()) {
					if(strcmp(node["file"](), name)) {
						printf("%s matched!!!\n", path);
					}
				}

			}
			++node;
		}
	}
}
