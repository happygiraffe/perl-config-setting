#define MAXCOMLEN 255
#define MAXUSERIDLEN 16
#define MAXCMDNAMELEN 6
#define MAXPARAMLEN 255

/* TODO: Check the guiness book of records */
#define MAXHUMANLEN 255

#define KILL 0
#define OK 1
#define ERROR -1

typedef struct cmd_struct {
	int number;	/* Something for the command to keep forever */
	char *name;
} Cmd;

typedef struct cmdline_struct {
	int cmd_no;
	char userid[MAXUSERIDLEN+1];
	char cmd_name[MAXCMDNAMELEN+1];
  char param[MAXPARAMLEN+1];
} Cmdline;

typedef struct cmd_reply_struct {
	char *userid;
	char tdc[4];
	char *human;
	char *buf_fn;
	FILE *buf_fp;
} Reply;

