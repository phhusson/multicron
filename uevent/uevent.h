namespace UEvents {
	class Event {
		friend class ::UEvent;
		public:
			virtual void SetVar(const char *name, const char *value) {};
			virtual ~Event() { };
			virtual char* getName() {return NULL;};
			virtual Event* Create() {return NULL;};//Create self
		private:
			::UEvent::action_type action;
			char *devpath;
			char *subsys;
			int seqnum;
	};
}
