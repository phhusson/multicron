extern "C" {
#include "ezxml.h"
};

class xmlAttribute {
	friend class xmlNode;
	public:
#ifdef WRITE_XML
		char *operator=(char*);
#endif
		const char *operator()() const;
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

		xmlNode operator()(const char *) const;
		xmlAttribute operator[](const char*) const;
#ifdef WRITE_XML
		char *operator=(const char*);
#endif
		const char *operator()() const;
		bool operator!() const;
		bool operator!=(const char *arg) const;

		xmlNode operator[](int) const;
		xmlNode operator++();
		//Warning: dangerous function.
		void Free();
	private:
		xmlNode(ezxml_t);
		ezxml_t node;
};
