#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include "xml.h"
#include "multicron.h"
#include "date.h"
#include "commands.h"
#include <string.h>
#include <time.h>

DateEvent::DateEvent() {
	rfds=NULL;wfds=NULL;efds=NULL;
	name=strdup("date");
	tasks=NULL;
}

DateEvent::~DateEvent() {
	if(!tasks)
		return;

	int i;
	for(i=0;tasks[i];++i)
		delete tasks[i];
	free(tasks);
	tasks=NULL;
	if(name)
		free(name);
}

void DateEvent::Callback(xmlNode config, int fd, ETYPE event_type) {
	if(event_type!=TIMEOUT)
		throw "Ouch, got a non-timeout event!";
	if(!tasks)
		return;
	int i;
	Time now;
	struct context ctx;
	bzero(&ctx, sizeof(ctx));
	for(i=0;tasks[i];++i) {
#ifdef DEBUG
		printf("When=");
		tasks[i]->when.Display();
		printf("Now=");
		now.Display();
#endif
		if(tasks[i]->when<=now) {
			xmlNode cur=tasks[i]->task;
			Cmds::Call(cur, ctx);
			delete tasks[i];
			tasks[i]=new DateTask(cur);
		}
	}
}

void DateEvent::RefreshConfig(xmlNode config) {
	if(tasks) {
		int i;
		for(i=0;tasks[i];++i)
			delete tasks[i];
		free(tasks);
	}
	tasks=NULL;
	int n=0;
	while(!!config) {
		tasks=(DateTask**)realloc(tasks, sizeof(DateTask*)*(n+2));
		tasks[n]=new DateTask(config);
		++n;
		++config;
	}
	tasks[n]=NULL;
}

struct timeval DateEvent::NextTimeout(xmlNode config) {
	Time min;
	min.Add(10, Time::HOUR);
	int i;
	for(i=0;tasks[i];i++) {
		if(tasks[i]->when<=min)
			min=tasks[i]->when;
	}
	struct timeval tv;
	Time now;
	tv.tv_sec=min.toTime()-now.toTime();
	tv.tv_usec=0;
	if(tv.tv_sec<=0)
		tv.tv_sec=1;
	return tv;
}

DateTask::DateTask(xmlNode config) 
 : task(config) {
	//Here we parse the xml node to know how often to execute the commands
	int hour=-1,min=-1,sec=-1,d;
	bool got=false;
	Time now;
	if(config["hour"]()) {
		hour=atoi(config["hour"]());
		if(hour<0 || hour >=24)
			throw "Unsupported hour (must be 0-23)";
		d=(hour-when.toTm()->tm_hour)%24;
		if(!got) {
			got=true;
			if(d<0)
				d+=24;
		}
		when.Add(d, Time::HOUR);
	}

	if(config["min"]()) {
		min=atoi(config["min"]());
		if(min<0 || min >=60)
			throw "Unsupported minute (must be 0-59)";
		d=(min-when.toTm()->tm_min)%60;
		if(!got) {
			got=true;
			if(d<0)
				d+=60;
		}
		when.Add(d, Time::MINUTE);
	}

	if(config["sec"]()) {
		sec=atoi(config["sec"]());
		if(sec<0 || sec >=60)
			throw "Unsupported seconds (must be 0-59)";
		d=(sec-when.toTm()->tm_sec)%60;
		if(!got) {
			got=true;
			if(d<0)
				d+=60;
		}
		when.Add(d, Time::SECOND);
	}
	if(when<=now) {
		if(!config["min"]() && config["sec"]())
			when.Add(1, Time::MINUTE);
		else if(!config["hour"]() && config["min"]())
			when.Add(1, Time::HOUR);
		else if(!config["day"]() && config["hour"]())
			when.Add(1, Time::DAY);
	}
#ifdef DEBUG
	printf("New task:");
	when.Display();
#endif
}

Time::Time() {
	time(&t);
}

Time::Time(time_t val) {
	t=val;
}

Time::Time(struct tm *tm) {
	t=mktime(tm);
}

time_t Time::toTime() {
	return t;
}

void Time::Add(int ln, DType type) {
	switch(type) {
		case DAY:
			t+=ln*3600*24;
			break;
		case HOUR:
			t+=ln*3600;
			break;
		case MINUTE:
			t+=ln*60;
			break;
		case SECOND:
			t+=ln;
			break;
	};
}

bool Time::operator<=(const Time& a) {
	return t<=a.t;
}

struct tm *Time::toTm() {
	return localtime(&t);
}

#ifdef DEBUG
void Time::Display() {
	printf("HH:MM:SS=%02d:%02d:%02d\n", (t%86400)/3600, (t%3600)/60, t%60);
}
#endif
