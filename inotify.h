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
		InotifyEvent();
		void Callback(int fd, ETYPE event_type);
		void AddCfg(cfgNode cfg);
		~InotifyEvent();
	private:
		inotify_file *inotify_files;
		int n;
};

