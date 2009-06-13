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

class MainLoop {
	public:
		static MainLoop *Get(); //Singleton, maybe private method ?
		static void Reload();
		static void AddEM(EventManager *ev);
		static void AddFDs(fd_set &fds, EventManager::ETYPE event_type, int &max);
		static void NextTimeout(struct timeval &tv);
		static void Callback(const fd_set& fds, EventManager::ETYPE event_type);
		static void CallTimeout();
	private:
		MainLoop();
		EventManager **evs;
		xmlNode root;
		int n;

};

bool regexp_match(const char *regexp, const char *val);
