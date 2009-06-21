class CNProcEvent : public EventManager {
	public:
		CNProcEvent();
		~CNProcEvent();
		void Callback(int fd, ETYPE event_type);
	private:
		void handle_msg(struct cn_msg*);
};

