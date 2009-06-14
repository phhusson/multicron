struct context{
	pid_t pid;
	char *file;
};

typedef struct context context_t;

struct cmd {
	char *name;
	void (*callback)(const cfgNode&, const context_t&);
};

void initCmds();
class Cmds {
	public:
		static void Call(const cfgNode&, context_t);
		static void Update();
	private:
		static struct cmd *cmds;
};
