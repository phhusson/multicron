/*(C) 2009 HUSSON Pierre-Hugues <phhusson@free.fr>
 * Original code for connector handling stolen from examples, copyrights follows:
 */
 /*
 * (C) 2007 Sebastian Krahmer <krahmer@suse.de> , original netlink handling
 * stolen from an proc-connector example, copyright folows:
 */
/*
 *
 * Copyright (C) Matt Helsley, IBM Corp. 2005
 * Derived from fcctl.c by Guillaume Thouvenin
 * Original copyright notice follows:
 *
 * Copyright (C) 2005 BULL SA.
 * Written by Guillaume Thouvenin <guillaume.thouvenin@bull.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <linux/connector.h>
#include <linux/netlink.h>
#include <linux/cn_proc.h>
#include <pcreposix.h>

#include <string>

#include "xml.h"
#include "multicron.h"
#include "commands.h"
#include "cnproc.h"

#define SEND_MESSAGE_LEN (NLMSG_LENGTH(sizeof(struct cn_msg) + \
				       sizeof(enum proc_cn_mcast_op)))
#define RECV_MESSAGE_LEN (NLMSG_LENGTH(sizeof(struct cn_msg) + \
				       sizeof(struct proc_event)))

#define SEND_MESSAGE_SIZE    (NLMSG_SPACE(SEND_MESSAGE_LEN))
#define RECV_MESSAGE_SIZE    (NLMSG_SPACE(RECV_MESSAGE_LEN))

#define max(x,y) ((y)<(x)?(x):(y))
#define min(x,y) ((y)>(x)?(x):(y))

#define BUFF_SIZE (max(max(SEND_MESSAGE_SIZE, RECV_MESSAGE_SIZE), 1024))
#define MIN_RECV_SIZE (min(SEND_MESSAGE_SIZE, RECV_MESSAGE_SIZE))

#define PROC_CN_MCAST_LISTEN (1)
#define PROC_CN_MCAST_IGNORE (2)

enum {
	/* Use successive bits so the enums can be used to record
	 * sets of events as well
	 */
	PROC_EVENT_NONE = 0x00000000,
	PROC_EVENT_FORK = 0x00000001,
	PROC_EVENT_EXEC = 0x00000002,
	PROC_EVENT_UID  = 0x00000004,
	PROC_EVENT_GID  = 0x00000040,
	/* "next" should be 0x00000400 */
	/* "last" is the last process event: exit */
	PROC_EVENT_EXIT = 0x80000000
};

/*
 * SIGINT causes the program to exit gracefully 
 * this could happen any time after the LISTEN message has
 * been sent
 */
#define INTR_SIG SIGINT
sigjmp_buf g_jmp;

static void handle_msg (struct cn_msg *cn_hdr, xmlNode config)
{
	char cmdline[1024], fname1[1024], fname2[1024], file[1024];
	int cmdline_sz, fd, i;
	struct proc_event *ev = (struct proc_event *)cn_hdr->data;

	/* Weird opens, to keep race as short as possible */
	snprintf(fname1, sizeof(fname1), "/proc/%d/cmdline", ev->event_data.exec.process_pid);
	snprintf(fname2, sizeof(fname2), "/proc/%d/exe", ev->event_data.exec.process_pid);

	fd = open(fname1, O_RDONLY);
	memset(&cmdline, 0, sizeof(cmdline));
	memset(&file, 0, sizeof(file));

	if (fd > 0) {
		cmdline_sz = read(fd, cmdline, sizeof(cmdline));
		close(fd);

		for (i = 0; cmdline_sz > 0 && i < cmdline_sz; ++i) {
			if (cmdline[i] == 0)
				cmdline[i] = ' ';
		}
	}

	readlink(fname2, file, 1024);
	
	/*
	switch(ev->what){
	case PROC_EVENT_FORK:
		printf("FORK:parent(pid,tgid)=%d,%d\tchild(pid,tgid)=%d,%d\t[%s]\n",
		       ev->event_data.fork.parent_pid,
		       ev->event_data.fork.parent_tgid,
		       ev->event_data.fork.child_pid,
		       ev->event_data.fork.child_tgid, cmdline);
		break;
	case PROC_EVENT_EXEC:
		printf("EXEC:pid=%d,tgid=%d\t\t[%s]\n",
		       ev->event_data.exec.process_pid,
		       ev->event_data.exec.process_tgid, cmdline);
		break;
	case PROC_EVENT_EXIT:
		printf("EXIT:pid=%d,%d\texit code=%d\n",
		       ev->event_data.exit.process_pid,
		       ev->event_data.exit.process_tgid,
		       ev->event_data.exit.exit_code);
		break;
	default:
		break;
	}*/
	if(ev->what!=PROC_EVENT_EXEC)
		return;
	
	xmlNode node(config);
	while(!!node) {
		const char *afile=node["file"]();
		if(afile)
			if(!regexp_match(afile, file)) {
#ifdef DEBUG
				printf("Didn't matched file:'%s'VS'%s'\n", file, afile);
#endif
				++node;
				continue;
			}
#ifdef DEBUG
		printf("Matched file:%s\n", file);
#endif
		const char *match_cmd=node["cmdline"]();
		if(!match_cmd && !afile)
			throw std::string("Wait... you want to match every processes ? I prefer saying no.");
		if(match_cmd)
			if(!regexp_match(match_cmd, cmdline)) {
				++node;
				continue;
			}
#ifdef DEBUG
		printf("Matched one!\n");
#endif
		//Ok we matched everything, let's call command.
		struct context ctx;
		ctx.pid=ev->event_data.exec.process_pid;
		ctx.file=NULL;
		cmdCall(node, ctx);
		++node;
	}
}


int cnprocInit() {
	//Return associated fd
	static int sk_nl=-1;
	//Need to manage multithreading...
	if(sk_nl>=0)
		return sk_nl;
	int err;
	struct sockaddr_nl my_nla, kern_nla;
	int rc = -1;
	struct nlmsghdr *nl_hdr;
	struct cn_msg *cn_hdr;
	enum proc_cn_mcast_op *mcop_msg;
	char buff[BUFF_SIZE];

	/*
	 * Create an endpoint for communication. Use the kernel user
	 * interface device (PF_NETLINK) which is a datagram oriented
	 * service (SOCK_DGRAM). The protocol used is the connector
	 * protocol (NETLINK_CONNECTOR)
	 */
	sk_nl = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
	if (sk_nl == -1) {
		perror("socket sk_nl error");
		return rc;
	}
	bzero(&my_nla, sizeof(my_nla));
	my_nla.nl_family = AF_NETLINK;
	my_nla.nl_groups = CN_IDX_PROC;
	my_nla.nl_pid = getpid();

	kern_nla.nl_family = AF_NETLINK;
	kern_nla.nl_groups = CN_IDX_PROC;
	kern_nla.nl_pid = 1;

	err = bind(sk_nl, (struct sockaddr *)&my_nla, sizeof(my_nla));
	if (err == -1) {
		perror("binding sk_nl error");
		close(sk_nl);
		sk_nl=-1;
		return sk_nl;
	}
	nl_hdr = (struct nlmsghdr *)buff;
	cn_hdr = (struct cn_msg *)NLMSG_DATA(nl_hdr);
	mcop_msg = (enum proc_cn_mcast_op*)&cn_hdr->data[0];

	memset(buff, 0, sizeof(buff));
	*mcop_msg =(proc_cn_mcast_op) PROC_CN_MCAST_LISTEN;
	/* fill the netlink header */
	nl_hdr->nlmsg_len = SEND_MESSAGE_LEN;
	nl_hdr->nlmsg_type = NLMSG_DONE;
	nl_hdr->nlmsg_flags = 0;
	nl_hdr->nlmsg_seq = 0;
	nl_hdr->nlmsg_pid = getpid();
	/* fill the connector header */
	cn_hdr->id.idx = CN_IDX_PROC;
	cn_hdr->id.val = CN_VAL_PROC;
	cn_hdr->seq = 0;
	cn_hdr->ack = 0;
	cn_hdr->len = sizeof(enum proc_cn_mcast_op);
	if (send(sk_nl, nl_hdr, nl_hdr->nlmsg_len, 0) != nl_hdr->nlmsg_len) {
		printf("failed to send proc connector mcast ctl op!\n");
		close(sk_nl);
		sk_nl=-1;
		return sk_nl;
	}

	printf("sent\n");
	if (*mcop_msg == PROC_CN_MCAST_IGNORE) {
		rc = 0;
		close(sk_nl);
		sk_nl=-1;
		return sk_nl;
	}
	return sk_nl;
}

void CNProcEvent::Callback(xmlNode config, int fd, EventManager::ETYPE event_type) {
	if(event_type==EventManager::TIMEOUT)
		return;
	(void)fd;
	(void)event_type;

	char buff[BUFF_SIZE];
	int sk_nl=cnprocInit();
	socklen_t from_nla_len;
	struct sockaddr_nl kern_nla, from_nla;
	size_t recv_len = 0;
	struct cn_msg *cn_hdr;

	memset(buff, 0, sizeof(buff)), from_nla_len = sizeof(from_nla);
	struct nlmsghdr *nlh = (struct nlmsghdr*)buff;
	memcpy(&from_nla, &kern_nla, sizeof(from_nla));
	recv_len = recvfrom(sk_nl, buff, BUFF_SIZE, 0,
			(struct sockaddr*)&from_nla, &from_nla_len);
	if (recv_len < 1)
		return;
	while (NLMSG_OK(nlh, recv_len)) {
		cn_hdr = (struct cn_msg*)NLMSG_DATA(nlh);
		if (nlh->nlmsg_type == NLMSG_NOOP)
			continue;
		if ((nlh->nlmsg_type == NLMSG_ERROR) ||
		    (nlh->nlmsg_type == NLMSG_OVERRUN))
			break;
		handle_msg(cn_hdr, config);
		if (nlh->nlmsg_type == NLMSG_DONE)
			break;
		nlh = NLMSG_NEXT(nlh, recv_len);
	}
}

CNProcEvent::CNProcEvent() {
	rfds=(int*)malloc(sizeof(int)*2);
	rfds[0]=cnprocInit();
	rfds[1]=-1;
	wfds=NULL;
	efds=NULL;

	name=strdup("cnproc");
}
