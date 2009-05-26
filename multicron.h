enum ETYPE {
	READ,
	WRITE,
	EXCEPTION
};

struct event_manager {
	int *rfds,*wfds,*efds;
	void (*callback)(xmlNode config, int fd, ETYPE event_type);
	void (*refresh_config)(xmlNode config);
	const char *name;//NULL terminated string list
};

int regexp_match(const char *regexp, const char *val);
