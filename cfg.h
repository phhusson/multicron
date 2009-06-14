#include "ezxml.h"
class cfgNode {
	public:
		cfgNode(const char *path);
		cfgNode(char *buf, int ln);
		cfgNode(int fd);
		cfgNode(ezxml_t tree);
		cfgNode();

		const char *operator[](const char*) const;

		const char *operator()() const;
		bool operator!() const;
		bool operator!=(const char *arg) const;

		void operator++();
		const char *getName() { return name; };
		cfgNode getChild() { if(child) return *child; return cfgNode(); };

		//Warning: dangerous function.
		void Free();
	private:
		void Fill(ezxml_t arg);
		cfgNode *next;
		cfgNode *child;//First child
		char **attrs;//Same as ezxml: [ name, value, name, value...., NULL, NULL ]
		char *value;
		char *name;
};
