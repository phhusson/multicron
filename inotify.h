typedef struct {
#ifdef BSD
	int fd;
	int wd;
	char *filename;
	time_t last;
#else
	char *filename;
	int wd;
#endif
} inotify_file;

class InotifyEvent : public EventManager {
	public:
		InotifyEvent(cfgNode cfg);
		void Callback(int fd, ETYPE event_type);
		void RefreshConfig();
		~InotifyEvent();
	private:
		InotifyEvent();
		inotify_file *inotify_files;
		int n;
		cfgNode cfg;
};

