namespace UEvents {
	class Event {
		public:
			virtual void SetVar(const char *name, const char *value) {};
			Event() {
				devpath=NULL;
				subsys=NULL;
			};
			virtual ~Event() {
				if(devpath) { free(devpath);devpath=NULL;}
				if(subsys) { free(subsys);subsys=NULL;}
			};
			virtual bool Match(cfgNode) { return false;};

			//Damn i hate letting these as public, but i can't see how to do otherwise
			::UEvent::action_type action;
			char *devpath;
			char *subsys;
			int seqnum;
	};
}
