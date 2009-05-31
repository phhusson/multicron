struct context{
	pid_t pid;
	char *file;
};

typedef struct context context_t;

struct cmd {
	char *name;
	void (*callback)(xmlNode, const context_t&);
};

void initCmds();
class Cmds {
	public:
		static void Call(xmlNode, context_t);
		static void Update();
	private:
		static struct cmd *cmds;
};
