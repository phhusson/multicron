class EventManager {

	public:
		enum ETYPE {
			READ,
			WRITE,
			EXCEPTION,
			TIMEOUT
		};

		int *rfds,*wfds,*efds;
		virtual void Callback(xmlNode config, int fd, ETYPE event_type);
		virtual void RefreshConfig(xmlNode config);
		virtual struct timeval NextTimeout(xmlNode config);
		virtual void AddFDs(fd_set &fds, ETYPE event_type, int &max) const;
		char *name;
		virtual ~EventManager();
};

bool regexp_match(const char *regexp, const char *val);
