class CNProcEvent : public EventManager {
	public:
		CNProcEvent();
		void Callback(xmlNode config, int fd, ETYPE event_type);
	private:
		~CNProcEvent();
};

