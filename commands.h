struct context{
	pid_t pid;
	char *file;
};

typedef struct context context_t;

struct cmd {
	char *name;
	void (*callback)(xmlNode, context_t);
};

void initCmds();
void cmdCall(xmlNode, context_t);
