#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>

class EventManager {

	public:
		enum ETYPE {
			READ,
			WRITE,
			EXCEPTION,
			TIMEOUT
		};

		int *rfds,*wfds,*efds;
		virtual void Callback(int fd, ETYPE event_type);
		virtual void AddCfg(cfgNode cfg);
		virtual struct timeval NextTimeout();
		virtual void AddFDs(fd_set &fds, ETYPE event_type, int &max) const;
		char *name;
		virtual ~EventManager();

		cfgNode **cfg;
		int n_cfg;
};

class MainLoop {
	public:
		static void Reload();
		static EventManager *GetEM(const char *name);
		static void UnloadEM(const char *name) {};//I think you don't want to do this. Anyway it does nothing atm
		static void AddFDs(fd_set &fds, EventManager::ETYPE event_type, int &max);
		static void NextTimeout(struct timeval &tv);
		static void Callback(const fd_set& fds, EventManager::ETYPE event_type);
		static void CallTimeout();

		static void AddEM(EventManager *ev);
		static void DelEM(EventManager *ev);
	private:
		static MainLoop *Get();
		MainLoop();
		EventManager **evs;
		cfgNode root;
		int n;

};

bool regexp_match(const char *regexp, const char *val);
