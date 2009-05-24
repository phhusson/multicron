extern "C" {
#include "ezxml.h"
};

class xmlAttribute {
	friend class xmlNode;
	public:
		char *operator=(char*);
		const char *operator()();
	private:
		xmlAttribute(ezxml_t, const char*);
		ezxml_t node;
		const char *attr_name;
};

class xmlNode {
	public:
		xmlNode(const char *path);
		xmlNode(char *buf, int ln);
		xmlNode(int fd);

		xmlNode operator()(const char *);
		xmlAttribute operator[](const char*);
		const char *operator=(const char*);
		const char *operator()();
		bool operator!();

		xmlNode operator[](int);
		xmlNode operator++();
	private:
		xmlNode(ezxml_t);
		ezxml_t node;
};
