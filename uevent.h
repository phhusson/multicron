class UEvent : public EventManager {
	public:
		enum action_type {
			ADD,
			REMOVE,
			CHANGE,
			MOVE,
			ONLINE,
			OFFLINE,
			UNKNOWN
		};
		UEvent();
		void Callback(xmlNode config, int fd, ETYPE event_type);
		~UEvent();
	private:
		struct uev {
			action_type action;
			char *s_action;
			char *devpath;
			char *subsys;
			int seqnum;
		};
};

