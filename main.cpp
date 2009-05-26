#include "xml.h"
#include "cnproc.h"
#include "inotify.h"
#include <iostream>
#include <string>
#include <pcreposix.h>
#include "multicron.h"
#include "commands.h"
#include <sys/select.h>

struct event_manager *inotify_module();
struct event_manager *cnproc_module();

int main(int argc, char **argv) {
	int i,j,max;
	try {
		xmlNode root("prout.xml");
		initCmds();
		struct event_manager **evs=(struct event_manager**)malloc(sizeof(void*)*3);
		evs[0]=inotify_module();
		evs[1]=cnproc_module();
		evs[2]=NULL;
		
		for(i=0;evs[i];++i) {
			if(evs[i]->refresh_config) {
				evs[i]->refresh_config(root(evs[i]->name));
			}
		}
		while(1) {
			fd_set rfds,wfds,efds;
			FD_ZERO(&rfds);
			FD_ZERO(&wfds);
			FD_ZERO(&efds);
			max=0;

			//Read FDs
			for(i=0;evs[i];++i) {
				if(evs[i]->rfds) {
					for(j=0;evs[i]->rfds[j]>=0;++j) {
						max=(max<evs[i]->rfds[j]) ? evs[i]->rfds[j] : max;
						FD_SET(evs[i]->rfds[j], &rfds);
					}
				}
			}

			//Write FDs
			for(i=0;evs[i];++i) {
				if(evs[i]->wfds) {
					for(j=0;evs[i]->wfds[j]>=0;++j) {
						max=(max<evs[i]->wfds[j]) ? evs[i]->wfds[j] : max;
						FD_SET(evs[i]->wfds[j], &wfds);
					}
				}
			}

			//Exception FDs
			for(i=0;evs[i];++i) {
				if(evs[i]->efds) {
					for(j=0;evs[i]->efds[j]>=0;++j) {
						max=(max<evs[i]->efds[j]) ? evs[i]->efds[j] : max;
						FD_SET(evs[i]->efds[j], &efds);
					}
				}
			}

			int ret=select(max+1, &rfds, &wfds, &efds, NULL);
			
			//Call timeouts
			if(ret==0)
				continue;


			//Read FDs
			for(i=0;evs[i];++i)
				if(evs[i]->rfds)
					for(j=0;evs[i]->rfds[j]>=0;++j)
						if(FD_ISSET(evs[i]->rfds[j], &rfds))
							evs[i]->callback(root(evs[i]->name), evs[i]->rfds[j], READ);

			//Write FDs
			for(i=0;evs[i];++i)
				if(evs[i]->wfds)
					for(j=0;evs[i]->wfds[j]>=0;++j)
						if(FD_ISSET(evs[i]->wfds[j], &wfds))
							evs[i]->callback(root(evs[i]->name), evs[i]->wfds[j], WRITE);

			//Exception FDs
			for(i=0;evs[i];++i)
				if(evs[i]->efds)
					for(j=0;evs[i]->efds[j]>=0;++j)
						if(FD_ISSET(evs[i]->efds[j], &efds))
							evs[i]->callback(root(evs[i]->name), evs[i]->efds[j], EXCEPTION);

		}
		//struct event_manager *ev=inotify_module();
		//ev->refresh_config(root(ev->name));
		/*
		struct event_manager *ev=cnproc_module();
		while(1) {
			ev->callback(root(ev->name[0]));
		}
		*/
	} catch(const std::string &e) {
		std::cout << e << std::endl;
	}
	return 0;
}
