class InotifyEvent : public EventManager {
	public:
		InotifyEvent();
		void Callback(xmlNode config, int fd, ETYPE event_type);
		void RefreshConfig(xmlNode config);
	private:
		char **inotify_files;
		//void inotify_cb(xmlNode config, int fd, EventManager::ETYPE event_type);
		//void inotify_conf(xmlNode config);
};

