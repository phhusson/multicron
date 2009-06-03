class UEvent : public EventManager {
	public:
		UEvent();
		void Callback(xmlNode config, int fd, ETYPE event_type);
		~UEvent();
	private:
		enum action_type {
			ADD,
			REMOVE,
			CHANGE,
			MOVE,
			ONLINE,
			OFFLINE,
			UNKNOWN
		};
		struct uev {
			action_type action;
			char *s_action;
			char *devpath;
			char *subsys;
			int seqnum;
		};
};

