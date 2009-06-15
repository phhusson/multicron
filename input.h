class InputEvent : public EventManager {
	public:
		InputEvent(cfgNode cfg);
		void Callback(int fd, ETYPE event_type);
		void RefreshConfig();
		struct timeval NextTimeout();
		~InputEvent();
	private:
		void CheckForEvent();
		InputEvent();
		cfgNode cfg;
};

