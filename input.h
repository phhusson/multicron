class InputEvent : public EventManager {
	public:
		InputEvent(cfgNode conf);
		void Callback(int fd, ETYPE event_type);
		struct timeval NextTimeout();
		~InputEvent();
	private:
		void CheckForEvent();
		InputEvent();
		cfgNode config;
		bool todelete;
};


class InputGlobalEvent : public EventManager {
	public:
		InputGlobalEvent();
		~InputGlobalEvent();
		void AddCfg(cfgNode cfg);
};
