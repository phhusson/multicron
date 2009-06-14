#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include "cfg.h"
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
	int max;
	reload=true;
	try {
		while(1) {
			if(reload) {
				MainLoop::Reload();
				reload=false;
			}

			while(waitpid(-1, NULL, WNOHANG)>0);
		
			fd_set rfds,wfds,efds;
			FD_ZERO(&rfds);
			FD_ZERO(&wfds);
			FD_ZERO(&efds);
			max=0;

			MainLoop::AddFDs(rfds, EventManager::READ, max);
			MainLoop::AddFDs(wfds, EventManager::WRITE, max);
			MainLoop::AddFDs(efds, EventManager::EXCEPTION, max);

			struct timeval tv;
			MainLoop::NextTimeout(tv);

			int ret=select(max+1, &rfds, &wfds, &efds, &tv);
			
			//Call timeouts
			if(ret==0) {
				MainLoop::CallTimeout();

				continue;
			}
			MainLoop::Callback(rfds, EventManager::READ);
			MainLoop::Callback(wfds, EventManager::WRITE);
			MainLoop::Callback(efds, EventManager::EXCEPTION);

		}
	} catch(const char *e) {
		printf("%s\n", e);
	} catch(...) {
		printf("Got unknown exception!\n");
	}
	return 0;
}

void EventManager::Callback(int fd, ETYPE event_type) {
}

void EventManager::RefreshConfig() {
}

struct timeval EventManager::NextTimeout() {
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

MainLoop::MainLoop() : root(cfgNode("multicron.xml")) {
	//Do nothing, everything is done in Reload();
	//But needed for singleton
	evs=NULL;
}

MainLoop *MainLoop::Get() {
	static MainLoop *ptr=NULL;
	if(ptr==NULL)
		ptr=new MainLoop;
	return ptr;
}

void MainLoop::Reload() {
	int i;
	printf("Let's reload folks!\n");
	MainLoop *self=Get();
	self->root.Free();
	Cmds::Update();
	while(1) {
		try {
			self->root=cfgNode("multicron.xml");
			break;
		} catch(...) {
			//Let vim/whatever enough time to write the file correctly
			sleep(1);
		}
	}
	if(self->evs) {
		for(i=0;self->evs[i];++i)
			delete self->evs[i];
		free(self->evs);
	}
	self->n=0;
	self->evs=NULL;
#if 0
#ifndef BSD
	self->evs=(EventManager**)malloc(5*sizeof(EventManager*));
	self->evs[0]=new InotifyEvent;
	self->evs[1]=new DateEvent;
	self->evs[2]=new CNProcEvent;
	self->evs[3]=new UEvent;
	self->evs[4]=NULL;
	self->n=4;
#else
	self->evs=(EventManager**)malloc(3*sizeof(EventManager*));
	self->evs[0]=new InotifyEvent;
	self->evs[1]=new DateEvent;
	self->evs[2]=NULL;
	self->n=2;
#endif
#endif

	AddEM(new InotifyEvent(self->root));
	AddEM(new DateEvent(self->root));
#ifndef BSD
	AddEM(new CNProcEvent(self->root));
	AddEM(new UEvent(self->root));
#endif
	
	for(i=0;self->evs[i];++i)
		self->evs[i]->RefreshConfig();
}

void MainLoop::AddEM(EventManager *ev) {
	if(ev==NULL)
		throw "Want to add a null event manager";
	MainLoop *self=Get();
	self->evs=(EventManager**)realloc(self->evs, (self->n+2)*sizeof(EventManager*));
	self->evs[self->n+1]=NULL;
	self->evs[self->n]=ev;
	++(self->n);
}

void MainLoop::AddFDs(fd_set &fds, EventManager::ETYPE event_type,  int &max) {
	MainLoop *self=Get();
	int i;
	for(i=0;i<(self->n); ++i) 
		self->evs[i]->AddFDs(fds, event_type, max);
}

void MainLoop::NextTimeout(struct timeval &tv) {
	struct timeval tv2;
	MainLoop *self=Get();
	int i;
	tv.tv_sec=1e9;
	tv.tv_usec=0;
	for(i=0;i<(self->n);++i) {
		tv2=self->evs[i]->NextTimeout();
		if(tv2.tv_sec < tv.tv_sec) {
			tv.tv_sec=tv2.tv_sec;
			tv.tv_usec=tv2.tv_usec;
		} else if(tv2.tv_sec==tv.tv_sec) 
			if(tv2.tv_usec<tv.tv_usec)
				tv.tv_usec=tv2.tv_usec;

	}
}

void MainLoop::CallTimeout() {
	MainLoop *self=Get();
	int i;
	for(i=0;i<(self->n);++i)
		self->evs[i]->Callback(
				/*self->evs[i]->cfg*/ /*(self->evs[i]->name),*/
				-1,
				EventManager::TIMEOUT);
}

void MainLoop::Callback(const fd_set &fds, EventManager::ETYPE event_type) {
	MainLoop *self=Get();
	int i,j;
	switch(event_type) {
		case EventManager::READ:
			for(i=0;self->evs[i];++i)
				if(self->evs[i]->rfds)
					for(j=0;self->evs[i]->rfds[j]>=0;++j)
						if(FD_ISSET(self->evs[i]->rfds[j], &fds))
							self->evs[i]->Callback(
									/*self->evs[i]->cfg*//*(self->evs[i]->name),*/
									self->evs[i]->rfds[j],
									EventManager::READ);
			break;
		case EventManager::WRITE:
			for(i=0;i<(self->n);++i)
				if(self->evs[i]->wfds)
					for(j=0;self->evs[i]->wfds[j]>=0;++j)
						if(FD_ISSET(self->evs[i]->wfds[j], &fds))
							self->evs[i]->Callback(
									/*self->evs[i]->cfg*//*(self->evs[i]->name),*/
									self->evs[i]->wfds[j],
									EventManager::WRITE);
			break;
		case EventManager::EXCEPTION:
			for(i=0;i<(self->n);++i)
				if(self->evs[i]->efds)
					for(j=0;self->evs[i]->efds[j]>=0;++j)
						if(FD_ISSET(self->evs[i]->efds[j], &fds))
							self->evs[i]->Callback(
									/*self->evs[i]->cfg*//*root(self->evs[i]->name),*/
									self->evs[i]->efds[j],
									EventManager::EXCEPTION);
			break;
		case EventManager::TIMEOUT:
			CallTimeout();
			break;
	};
}
