#include "ezxml.h"
#include "cfg.h"
#include <string>
#include <iostream>
#include <string.h>

void cfgNode::Fill(ezxml_t arg) {
	if(arg==NULL) {
		name=NULL;
		return;
	}

	int i=0;
	attrs=(char**)malloc(2*sizeof(char*));
	while( (arg->attr)[i]!=0 ) {
		attrs=(char**)realloc(attrs, (i+4)*sizeof(char*));
		if(arg->attr[i]!=NULL)
			attrs[i]=strdup(arg->attr[i]);
		else
			attrs[i]=NULL;
		++i;
	}
	attrs[i+1]=NULL;
	attrs[i]=NULL;
	name=strdup(arg->name);
	value=strdup(arg->txt);
	next=NULL;
	child=NULL;
	//There will be many cascaded creators... maybe not a good idea ?
	if(arg->ordered)
		next=new cfgNode(arg->ordered);
	if(arg->child)
		child=new cfgNode(arg->child);
}

cfgNode::cfgNode() {
	Fill(NULL);
}

cfgNode::cfgNode(ezxml_t arg) {
	Fill(arg);
}

cfgNode::cfgNode(const char *path) {
	ezxml_t node=ezxml_parse_file(path);
	if(!node)
		throw "Can't access requested config file";
	Fill(node->child);
	ezxml_free(node);
}

cfgNode::cfgNode(char *buf, int ln) {
	ezxml_t node=ezxml_parse_str(buf, ln);
	if(!node)
		throw "Can't parse this string";
	Fill(node->child);
	ezxml_free(node);
}

cfgNode::cfgNode(int fd) {
	ezxml_t node=ezxml_parse_fd(fd);
	if(!node)
		throw "Can't parse this file";
	Fill(node->child);
	ezxml_free(node);
}


const char *cfgNode::operator[](const char *name) const {
	int i=0;
	while(attrs[2*i]) {
		if(strcmp(attrs[2*i], name)==0)
			return attrs[2*i+1];
		++i;
	}
	return NULL;
}

const char *cfgNode::operator()() const {
	return value;
}

void cfgNode::operator++() {
	if(!next) {
		name=NULL;
		return;
	}
	child=next->child;
	attrs=next->attrs;
	value=next->value;
	name=next->name;
	next=next->next;
}

bool cfgNode::operator!() const {
	if(name)
		return false;
	return true;
}

bool cfgNode::operator!=(const char *arg) const {
	if(name && strcmp(name, arg)==0)
		return true;
	return false;
}

void cfgNode::Free() {
	if(name)
		free(name);
	if(value)
		free(value);

	int i=0;
	if(attrs) {
		while( attrs[i]!=0 ) {
			free(attrs[i]);
			attrs[i]=NULL;
			++i;
		}
	}
	free(attrs);
	attrs=NULL;
	if(child) {
		child->Free();
		delete child;
	}
	if(next) {
		next->Free();
		delete next;
	}
}
