#include "xml.h"
#include <string>
#include <iostream>
#include <string.h>

char *xmlAttribute::operator=(char *val) {
	ezxml_set_attr(node, strdup(attr_name), val);
	return val;
}

const char *xmlAttribute::operator()() {
	return ezxml_attr(node, attr_name);
}

xmlAttribute::xmlAttribute(ezxml_t anode, const char* name) :
	node(anode),
	attr_name(name)
{ }

xmlNode::xmlNode(ezxml_t arg) : node(arg) { }

xmlNode::xmlNode(const char *path) {
	node=ezxml_parse_file(path);
	if(!node)
		throw std::string("Can't access requested file ")+path;
}

xmlNode::xmlNode(char *buf, int ln) {
	node=ezxml_parse_str(buf, ln);
	if(!node)
		throw std::string("Can't parse this string");
}

xmlNode::xmlNode(int fd) {
	node=ezxml_parse_fd(fd);
	if(!node)
		throw std::string("Can't parse this file");
}

xmlNode xmlNode::operator()(const char* name) {
	ezxml_t tree=ezxml_child(node, name);
	if(!tree)
		throw std::string("Node '")+name+"' not found"; 
	return xmlNode(tree);
}

xmlAttribute xmlNode::operator[](const char *name) {
	return xmlAttribute(node, name);
}

const char *xmlNode::operator=(const char *val) {
	ezxml_set_txt(node, val);
	return val;
}

const char *xmlNode::operator()() {
	return ezxml_txt(node);
}

xmlNode xmlNode::operator[](int arg) {
	ezxml_t tmp=node;
	while( (arg--)>0 ) {
		if(tmp==NULL)
			return NULL;
		tmp=tmp->next;
	}
	if(tmp==NULL)
		return NULL;
	return xmlNode(tmp);
}

xmlNode xmlNode::operator++() {
	node=node->next;
	return (*this);
}

bool xmlNode::operator!() {
	if(node && node->name)
		return false;
	return true;
}

int main() {
	xmlNode t("prout.xml");
	xmlNode a=t("branch");
	while(!!a) {
		printf("%s\n", a["arg"]());
		++a;
	}
}