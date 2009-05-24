/*
 * This program (which doesn't works as expected because of inotify), is a sample use of inotify,
 * which get's notifications from a recursive directory
 */
#define __KERNEL_STRICT_NAMES
#define _GNU_SOURCE
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/unistd.h>
//#include <linux/inotify.h>
#include <sys/inotify.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h> 
#ifndef O_DIRECTORY
#define O_DIRECTORY    0200000 
#endif

//The next three functions are there because sometimes the equivalent un includes doesn't works good
//These ones are wrappers to call inotify related functions in the kernel
#if 0
int inotify_init() {
	long __res;
	__asm__ volatile ("int $0x80"
		: "=a" (__res)
		: "0" (__NR_inotify_init));
	return __res;
}
long inotify_add_watch(int fd, const char *path, __u32 mask) {
	long __res;
	__asm__ volatile ("int $0x80"
		: "=a" (__res)
		: "0" (__NR_inotify_add_watch),
			"b" ((long)(fd)),
			"c" ((long)(path)),
			"d" ((long)(mask)));
	return __res;
}

long inotify_rm_watch(int fd, __u32 wd) {
	long __res;
	__asm__ volatile ("int $0x80"
		: "=a" (__res)
		: "0" (__NR_inotify_rm_watch),"b" ((long)(fd)),"c" ((long)(wd)));
	return __res;
}
#endif


int inotify;
char **files;

//In this code it's to browse the folder (read dir.c for explanation)
void watch_dir(char *file) {
	int ret;
	struct stat buf;
	struct dirent *data;
	if(lstat(file, &buf))
		return;
	if(!S_ISDIR(buf.st_mode))
		return;
	//Now recursive part
	DIR *dir=opendir(file);
	if(dir==NULL)
		return;
	while((data=readdir(dir))!=NULL) {
		if(strcmp(data->d_name, ".")==0 || strcmp(data->d_name, "..")==0)
			continue;
		if(strstr(data->d_name, ".")==data->d_name)
			continue;
		char *path2;
		asprintf(&path2, "%s/%s", file, data->d_name);
		if(lstat(path2, &buf))
			continue;
		//If it's a dir then launch watch_dir himself on it
		if(S_ISDIR(buf.st_mode))
			watch_dir(path2);
	}
	closedir(dir);

	//Ok now set watch for the argument of watch_dir
	ret=inotify_add_watch(inotify, file, IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE | IN_CREATE| IN_DELETE_SELF); 
	if(ret==-1) {
		printf("%s\n", file);
		perror("Mise en place de la surveillance du fichier");
		exit(0);
	}
	if((ret%5)==0)
		files=realloc(files, (ret+5)*sizeof(char*));
	files[ret]=file;
	return;
}

int inotify_main() {
	//We first need to init inotify, it returns a poll-able fd
	inotify=inotify_init();
	if(!inotify) {
		perror("Open inotify device");
		fprintf(stderr, "Are you sure your kernel supports inotify?\n");
	}
	files=malloc(sizeof(char*)*5);
	time_t time1,time2;
	time1=time(NULL);
	//Ok launch recursive watching
	//If you let "/", be VERY patient !
	watch_dir(strdup("/home/phh/multicron/"));
	printf("Le chien est laché!\n");
	time2=time(NULL);
	//This code is to benchmark the time of the watch_dir call (some 100s seconds)
	printf("%ds de lancement\n", (int)(time2-time1));
	char buffer[4096];
	while(1) {
		int ret;
		int i=0;
		//Now read the device by big blocks
		ret=read(inotify, buffer, 4096);
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
			asprintf(&path, "%s/%s", files[event->wd],name);
			//Translate the inotify_event structure into a human comprehensible display
			if(event->mask==IN_MOVED_FROM || event->mask==IN_DELETE) {
				printf("%s/%s was deleted\n", files[event->wd],name);
			}
			else if(event->mask==IN_CREATE || event->mask==IN_MOVED_TO) {
				watch_dir(path);
				printf("%s/%s was created\n", files[event->wd],name);
			} else if(event->mask==IN_DELETE_SELF)
				printf("%s was deleted\n", files[event->wd]);
//			else
//				printf("%s:%x:%s\n", buffers[event->wd], event->mask,
//					name);
			i += sizeof (struct inotify_event) + event->len;
		}
	}
	return 0;
}
