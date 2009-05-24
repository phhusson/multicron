#include "xml.h"
#include "cnproc.h"
#include "inotify.h"
#include <iostream>
#include <string>
#include <pcreposix.h>
#include "multicron.h"

struct event_manager *inotify_module();

int main(int argc, char **argv) {
	try {
		xmlNode root("prout.xml");
		struct event_manager *ev=inotify_module();
		ev->refresh_config(root(ev->name));
		while(1) {
			ev->callback(root(ev->name));
		}
	} catch(const std::string &e) {
		std::cout << e << std::endl;
	}
	return 0;
}
