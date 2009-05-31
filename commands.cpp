#define _GNU_SOURCE
#include "xml.h"
#include "commands.h"
#include <string>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <linux/unistd.h>
#include <sys/syscall.h>

/* Copy from linux/ioprio.h*/
enum {
	IOPRIO_CLASS_NONE,
	IOPRIO_CLASS_RT,
	IOPRIO_CLASS_BE,
	IOPRIO_CLASS_IDLE,
};

enum {
	IOPRIO_WHO_PROCESS = 1,
	IOPRIO_WHO_PGRP,
	IOPRIO_WHO_USER,
};

struct cmd *Cmds::cmds=NULL;

#define IOPRIO_BITS             (16)
#define IOPRIO_CLASS_SHIFT      (13)
#define IOPRIO_PRIO_MASK        ((1UL << IOPRIO_CLASS_SHIFT) - 1)

#define IOPRIO_PRIO_CLASS(mask) ((mask) >> IOPRIO_CLASS_SHIFT)
#define IOPRIO_PRIO_DATA(mask)  ((mask) & IOPRIO_PRIO_MASK)
#define IOPRIO_PRIO_VALUE(clas, data)  (((clas) << IOPRIO_CLASS_SHIFT) | data)

void fill_context(context_t &ctx) {
	if(ctx.pid==0)
		ctx.pid=getpid();
}

void Cmds::Call(xmlNode arg, context_t context) {
	fill_context(context);
	int i=0;
	while(cmds[i].name!=NULL) {
		xmlNode t(arg(cmds[i].name));
		while(!!t) {
			cmds[i].callback(t, context);
			++t;
		}
		i++;
	}
}

void ioniceCall(xmlNode arg, const context_t& context) {
	printf("ionice called\n");
	const char *nice_class=arg["class"]();
	int ioprio;
	if(!nice_class)
		ioprio=IOPRIO_CLASS_BE;
	else if(strcmp(nice_class, "realtime")==0)
		ioprio=IOPRIO_CLASS_RT;
	else if(strcmp(nice_class, "idle")==0)
		ioprio=IOPRIO_CLASS_IDLE;
	else
		ioprio=IOPRIO_CLASS_BE;
	const char *prio=arg["priority"]();
	if(prio) {
		int a=atoi(prio);
		if(a<0 || a>7)
			a=4;
		ioprio=IOPRIO_PRIO_VALUE(ioprio, a);
	} else 
		ioprio=IOPRIO_PRIO_VALUE(ioprio, 4);

	syscall(SYS_ioprio_set, IOPRIO_WHO_PROCESS, context.pid, ioprio);
}

void cpuniceCall(xmlNode arg, const context_t& context) {
	printf("cpunice called\n");
}

void killCall(xmlNode arg, const context_t& context) {
	printf("kill called\n");
	const char *sig=arg["signal"]();
	int sigid;
	if(sig!=NULL)
		sigid=atoi(sig);
	else
		sigid=10;//SIGUSR1
	const char *pid=arg["pid"]();
	int p;
	if(pid)
		p=atoi(pid);
	else
		p=context.pid;
	printf("kill(%d,%d)\n", p, sigid);
	return;
	if(kill(p, sigid)!=0) {
		perror("kill");
	}
}

void cmdCall(xmlNode arg, const context_t& context) {
	if(arg()) {
		int pid=fork();
		if(pid>0)
			return;
		if(pid<0)
			throw "Couldn't fork!";
		char *p;
		asprintf(&p, "%d", context.pid);
		setenv("M_PID", p, 1);
		if(context.file)
			setenv("M_FILE", p, 1);
		else
			setenv("M_FILE", "(null)", 1);
		exit(system(arg()));
	}
}

extern bool reload;
void reloadCall(xmlNode arg, const context_t &ctx) {
	reload=true;
}

void Cmds::Update() {
	if(cmds) {
		int i;
		for(i=0;cmds[i].name;i++)
			free(cmds[i].name);
		free(cmds);
	}
	cmds=(struct cmd*)malloc(sizeof(*cmds)*10);
	memset(cmds, 0, sizeof(*cmds)*10);
	cmds[0].name=strdup("ionice");
	cmds[0].callback=ioniceCall;
	cmds[1].name=strdup("cpunice");
	cmds[1].callback=cpuniceCall;
	cmds[2].name=strdup("kill");
	cmds[2].callback=killCall;
	cmds[3].name=strdup("cmd");
	cmds[3].callback=cmdCall;
	cmds[4].name=strdup("reload");
	cmds[4].callback=reloadCall;
}

