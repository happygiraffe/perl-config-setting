
#define FIND_ONLY 1
#define CREATE_ONLY 2
#define FIND_OR_CREATE 3

#define CHANNELS 256

/* Maximum length of soft config file line - Make sufficiently greater than
  MAXTOPICLEN to allow for it, and the channel number */
#define MAXCONFIGLEN 90
#define MAXTOPICLEN 80

typedef struct usernode {
	char *userid;
	struct usernode *prev;
	struct usernode *next;
} Usernode;

typedef struct userlist {
  Usernode *first;  /* First node */
  Usernode *last;   /* Last node - might be useful, otherwise remove */
  int nodes;
} Userlist;

typedef struct channel {
	Userlist *ulist;
	char topic[MAXTOPICLEN];
} Channel;

