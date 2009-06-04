#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include "xml.h"
#include "multicron.h"
#include "cnproc.h"
#include "inotify.h"
#include "uevent.h"
#include "date.h"
/*
#include <iostream>
#include <string>
*/
#include "commands.h"

struct event_manager *inotify_module();
struct event_manager *cnproc_module();

bool reload;

int main(int argc, char **argv) {
	int i,j,max;
	reload=true;
	xmlNode root("multicron.xml");
	EventManager **evs=NULL;
	try {
		while(1) {
			if(reload) {
				printf("Let's reload folks!\n");
				reload=false;
				root.Free();
				Cmds::Update();
				while(1) {
					try {
						root=xmlNode("multicron.xml");
						break;
					} catch(...) {
						//Let vim/whatever enough time to write the file
						sleep(1);
					}
				}
				if(evs) {
					for(i=0;evs[i];++i)
						delete evs[i];
					free(evs);
				}
				evs=(EventManager**)malloc(5*sizeof(EventManager*));
				evs[0]=new InotifyEvent;
				evs[1]=new DateEvent;
#ifndef BSD
				evs[2]=new CNProcEvent;
				evs[3]=new UEvent;
#else
				evs[2]=NULL;
				evs[3]=NULL;
#endif
				evs[4]=NULL;
				
				for(i=0;evs[i];++i)
					evs[i]->RefreshConfig(root(evs[i]->name));
			}

			while(waitpid(-1, NULL, WNOHANG)>0);
		
			fd_set rfds,wfds,efds;
			FD_ZERO(&rfds);
			FD_ZERO(&wfds);
			FD_ZERO(&efds);
			max=0;

			//Read FDs
			for(i=0;evs[i];++i)
				evs[i]->AddFDs(rfds, EventManager::READ, max);

			//Write FDs
			for(i=0;evs[i];++i)
				evs[i]->AddFDs(rfds, EventManager::WRITE, max);

			//Exception FDs
			for(i=0;evs[i];++i)
				evs[i]->AddFDs(rfds, EventManager::EXCEPTION, max);

			struct timeval tv,tv2;
			tv.tv_sec=1e9;
			tv.tv_usec=0;
			for(i=0;evs[i];++i) {
				tv2=evs[i]->NextTimeout(root(evs[i]->name));
				if(tv2.tv_sec <= tv.tv_sec)
					tv.tv_sec=tv2.tv_sec;

			}

			int ret=select(max+1, &rfds, &wfds, &efds, &tv);
			
			//Call timeouts
			if(ret==0) {
				for(i=0;evs[i];++i)
					evs[i]->Callback(root(evs[i]->name), -1, EventManager::TIMEOUT);

				continue;
			}


			//Read FDs
			for(i=0;evs[i];++i)
				if(evs[i]->rfds)
					for(j=0;evs[i]->rfds[j]>=0;++j)
						if(FD_ISSET(evs[i]->rfds[j], &rfds))
							evs[i]->Callback(root(evs[i]->name), evs[i]->rfds[j], EventManager::READ);

			//Write FDs
			for(i=0;evs[i];++i)
				if(evs[i]->wfds)
					for(j=0;evs[i]->wfds[j]>=0;++j)
						if(FD_ISSET(evs[i]->wfds[j], &wfds))
							evs[i]->Callback(root(evs[i]->name), evs[i]->wfds[j], EventManager::WRITE);

			//Exception FDs
			for(i=0;evs[i];++i)
				if(evs[i]->efds)
					for(j=0;evs[i]->efds[j]>=0;++j)
						if(FD_ISSET(evs[i]->efds[j], &efds))
							evs[i]->Callback(root(evs[i]->name), evs[i]->efds[j], EventManager::EXCEPTION);

		}
	} catch(const char *e) {
		printf("%s\n", e);
	}
	return 0;
}

void EventManager::Callback(xmlNode config, int fd, ETYPE event_type) {
}

void EventManager::RefreshConfig(xmlNode config) {
}

struct timeval EventManager::NextTimeout(xmlNode config) {
	struct timeval tv;
	tv.tv_sec=1e9;
	tv.tv_usec=0;
	return tv;
}

void EventManager::AddFDs(fd_set &fds, ETYPE event_type, int &max) const {
	int j;
	int *tfds=NULL;
	switch(event_type) {
		case READ:
			tfds=rfds;
			break;
		case WRITE:
			tfds=wfds;
			break;
		case EXCEPTION:
			tfds=efds;
			break;
		case TIMEOUT:
		default:
			break;
	};

	if(!tfds)
		return;
	for(j=0;tfds[j]>=0;++j) {
		max=(max<tfds[j]) ? tfds[j] : max;
		FD_SET(tfds[j], &fds);
	}

}

EventManager::~EventManager() {
}
