class Time {
	public:
		enum DType {
			DAY,
			HOUR,
			MINUTE,
			SECOND
		};
		Time();// = now
		Time(time_t);// = now
		Time(struct tm*);// = now
		void Add(int ln, DType type);
		bool operator<=(const Time&);
		time_t toTime();
		struct tm *toTm();
		void Display();
	private:
		//Return true if we made a change
		//Meaning another changed may be necessary
		time_t t;
};

class DateTask {
	friend class DateEvent;
	private:
		DateTask(cfgNode);
		Time when;
		cfgNode task;
};

class DateEvent : public EventManager {
	public:
		DateEvent(cfgNode conf);
		void Callback(int fd, ETYPE event_type);
		struct timeval NextTimeout();
		void RefreshConfig();
		~DateEvent();
	private:
		DateEvent() { throw "No simple constructor allowed"; };
		DateTask **tasks;
		cfgNode cfg;
};
