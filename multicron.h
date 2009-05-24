struct event_manager {
	int fd;
	void (*callback)(xmlNode config);
	void (*refresh_config)(xmlNode config);
	const char *name;
};

int regexp_match(const char *regexp, const char *val);
