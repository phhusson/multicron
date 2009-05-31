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
		DateTask(xmlNode);
		Time when;
		xmlNode task;
};

class DateEvent : public EventManager {
	public:
		DateEvent();
		void Callback(xmlNode config, int fd, ETYPE event_type);
		struct timeval NextTimeout(xmlNode config);
		void RefreshConfig(xmlNode config);
		~DateEvent();
	private:
		DateTask **tasks;
};
