class UEvent : public EventManager {
	public:
		UEvent(cfgNode conf);
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
		UEvent() { throw "No simple constructor"; };
		struct uev {
			action_type action;
			char *s_action;
			char *devpath;
			char *subsys;
			int seqnum;
		};
		cfgNode cfg;
};

