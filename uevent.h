class UEvent : public EventManager {
	public:
		UEvent();
		enum action_type {
			ADD,
			REMOVE,
			CHANGE,
			MOVE,
			ONLINE,
			OFFLINE,
			UNKNOWN
		};
		void Callback(int fd, ETYPE event_type);
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

