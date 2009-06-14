#include <string.h>
#include "cfg.h"
#include "multicron.h"
#include "input.h"

InputEvent::InputEvent(cfgNode conf) : cfg(conf) {
	printf("Called input creator!\n");
	rfds=NULL;
	wfds=NULL;
	efds=NULL;
	name=strdup("input");
}

void InputEvent::Callback(int fd, ETYPE event_type) {
	printf("Called InputEvent callback ?!?\n");
	MainLoop::DelEM(this);
}

void InputEvent::RefreshConfig() {
	printf("Called InputEvent Refreshconfig ?!?\n");
}

struct timeval InputEvent::NextTimeout() {
	struct timeval tv;
	tv.tv_sec=1e9;
	tv.tv_usec=0;
	return tv;
}

InputEvent::~InputEvent() {
	printf("Called InputEvent destructor\n");
}
