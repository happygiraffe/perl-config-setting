/* PROTOTYPES */

/* talker.c */
int	join(Cmdline *);
int	synoff(Cmdline *);
int	clist(Cmdline *);

/* error.c */
int	logerr(char *, char *);

/* util.c */
void	setreply(char *, char *, char *);
void	flush_cmd(void);
int	bprintf(char *, ...);
