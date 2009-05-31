typedef struct {
	char *filename;
	int wd;
} inotify_file;

class InotifyEvent : public EventManager {
	public:
		InotifyEvent();
		void Callback(xmlNode config, int fd, ETYPE event_type);
		void RefreshConfig(xmlNode config);
		~InotifyEvent();
	private:
		inotify_file *inotify_files;
		int n;
};

