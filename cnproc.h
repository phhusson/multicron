class CNProcEvent : public EventManager {
	public:
		CNProcEvent(cfgNode conf);
		void Callback(int fd, ETYPE event_type);
	private:
		CNProcEvent();
		~CNProcEvent();
		cfgNode cfg;
};

