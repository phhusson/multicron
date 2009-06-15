#include "ezxml.h"
#include "cfg.h"
#include <string>
#include <iostream>
#include <string.h>

void cfgNode::Fill(ezxml_t arg) {
	if(arg==NULL) {
		name=NULL;
		value=NULL;
		attrs=NULL;
		next=NULL;
		child=NULL;
		return;
	}

	int i=0;
	attrs=(char**)malloc(2*sizeof(char*));
	while( (arg->attr)[i]!=0 ) {
		attrs=(char**)realloc(attrs, (i+4)*sizeof(char*));
		attrs[i]=strdup(arg->attr[i]);
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

cfgNode::cfgNode(const cfgNode &cfg, bool wholeTree) {
	if(wholeTree==true)
		throw "Copy a cfgNode as a whole isn't supported at the moment";
	int i=0;
	attrs=(char**)malloc(2*sizeof(char*));
	while( cfg.attrs && (cfg.attrs)[i]!=0 ) {
		attrs=(char**)realloc(attrs, (i+4)*sizeof(char*));
		attrs[i]=strdup(cfg.attrs[i]);
		++i;
	}
	attrs[i+1]=NULL;
	attrs[i]=NULL;
	name=strdup(cfg.name);
	value=strdup(cfg.value);
	next=cfg.next;
	child=cfg.child;
}

cfgNode::~cfgNode() {
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
		free(attrs);
	}
	attrs=NULL;
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
	int i=0;
	if(attrs) {
		for(i=0; attrs[i]!=NULL; ++i) {
			free(attrs[i]);
			attrs[i]=NULL;
		}
		free(attrs);
		attrs=NULL;
	}
	free(value);
	free(name);
	if(!next) {
		Fill(NULL);
		return;
	}

	attrs=(char**)malloc(2*sizeof(char*));
	for(i=0; (next->attrs)[i]!=0;  ++i) {
		attrs=(char**)realloc(attrs, (i+4)*sizeof(char*));
		attrs[i]=strdup(next->attrs[i]);
	}
	attrs[i+1]=NULL;
	attrs[i]=NULL;
	child=next->child;
	value=strdup(next->value);
	name=strdup(next->name);
	next=next->next;
}

void cfgNode::operator=(const cfgNode &cfg) {
	if(!cfg) {
		Fill(NULL);
		return;
	}
	int i=0;
	if(attrs) {
		for(i=0; attrs[i]!=NULL; ++i) {
			free(attrs[i]);
			attrs[i]=NULL;
		}
		free(attrs);
		attrs=NULL;
	}
	free(value);
	free(name);

	attrs=(char**)malloc(2*sizeof(char*));
	for(i=0; (cfg.attrs)[i]!=0;  ++i) {
		attrs=(char**)realloc(attrs, (i+4)*sizeof(char*));
		attrs[i]=strdup(cfg.attrs[i]);
	}
	attrs[i+1]=NULL;
	attrs[i]=NULL;
	child=cfg.child;
	value=strdup(cfg.value);
	name=strdup(cfg.name);
	next=cfg.next;
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
	if(child) {
		child->Free();
		delete child;
	}
	if(next) {
		next->Free();
		delete next;
	}
}

void cfgNode::addAttr(const char *name, const char *value) {
	int i;
	for(i=0;attrs[i];++i);
	attrs=(char**)realloc(attrs, (i+4)*sizeof(char*));
	if( (i%2)==1)
		throw "First nil attr is not a name!";
	attrs[i]=strdup(name);
	if(value)
		attrs[i+1]=strdup(value);
	else
		attrs[i+1]=strdup("");
	attrs[i+2]=NULL;
	attrs[i+3]=NULL;
}
