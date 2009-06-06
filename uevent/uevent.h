namespace UEvents {
	class Event {
		public:
			virtual void SetVar(const char *name, const char *value) {};
			virtual ~Event() { };
			virtual bool Match(xmlNode) { return false;};

			//Damn i hate letting these as public, but i can't see how to do otherwise
			::UEvent::action_type action;
			char *devpath;
			char *subsys;
			int seqnum;
	};
}
