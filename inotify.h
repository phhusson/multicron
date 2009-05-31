class InotifyEvent : public EventManager {
	public:
		InotifyEvent();
		void Callback(xmlNode config, int fd, ETYPE event_type);
		void RefreshConfig(xmlNode config);
		~InotifyEvent();
	private:
		char **inotify_files;
};

