/*********************************************************************
 * spider.h
 *
 * All global variables, definitions, &c for spider.c
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 *
 * @(#) $Id: spider.h,v 1.5 2000/01/14 23:26:23 dom Exp $
 */

#ifndef _SPIDER_H_
#define _SPIDER_H_

#include <config.h>             /* autoconf */
#include <sys/types.h>          /* For fd_set */
#include <stdio.h>              /* For FILE */

/* All of this, just for time_t */
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "codes.h"

/*********************************************************************
 * Defines
 */

/* For use in determining the number of open descriptors */
#define MAX_OPEN_GUESS	256

/* Used a lot in the internal commands and main loop */
#define SENDER_NAME open_conns[sender]->name
#define SENDER_TYPE open_conns[sender]->type
#define SENDER_CHAN open_conns[sender]->chan
#define SENDER_CONN open_conns[sender]
#define RECEIVER_NAME open_conns[receiver]->name
#define RECEIVER_TYPE open_conns[receiver]->type
#define RECEIVER_CHAN open_conns[receiver]->chan
#define RECEIVER_CONN open_conns[receiver]

/* Used to maintain the highest descriptor number in wait_on */
#define SET_WAIT_MAX(x)	wait_on_max = ((x) > wait_on_max) ? (x) : wait_on_max
#define CLR_WAIT_MAX(x)	\
wait_on_max = ((x) == wait_on_max) ? wait_on_max-1 : wait_on_max

/* This number works pretty well, according to my testing, especially
 * for small numbers of table entries.  Change at your own risk. */
#define HASH_SIZE	101

/*********************************************************************
 * Data structures
 */

typedef enum {
    false = 0,
    true = 1
} Bool;

/*
 * Events are used to say what has happened in the main loop.
 */
typedef struct event_struct {
    int 	fd;		/* associated file descriptor */
    enum {
	new_connect,
	module_msg,
	client_msg,
	module_death,
	client_death,
	server_death
    }		type;
} Event;

/*
 * Nodes are used as hash table entries.
 */
typedef struct node * Nodep;
typedef struct node {
    Nodep	next;
    Nodep	prev;
    /* name should be independent of anything contained in data, so
     * that we may free it without causing problems. */
    char *	name;           
    void *	data;
} Node;

/*
 * Hash tables are used for quick access to both commands and
 * connections.
 */
typedef struct hash * Hashp;
typedef struct hash {
    Nodep	table[HASH_SIZE];
    Bool	ignore_case;    /* are the keys case independent? */
} Hash;

/*
 * Connections are either active connections to a client, active
 * connections to a module, or finally, may be used as an inactive
 * description of a user.
 */
typedef struct conn_struct *Connp;
typedef enum {module, user} Conntype;
typedef enum {s_not, s_init, s_conn} Connstate;
typedef struct conn_struct {
    FILE *	chan;		/* stdio channel; lowlev is arr index */
    char * 	name;
    Conntype 	type;		/* Type of connection */
    Connstate	state;		/* State of connection */
    int		fd;		/* file descriptor */
    char *	buf;		/* data waiting to be output */
    int		buflen;		/* length of buffer */
    int		bufhwm;		/* high water mark of our data in buffer */
    char *	eol;		/* end-of-line to use for output. */
    union {
	struct {
	    pid_t pid;		/* Pid of module */
	} mod;
	struct {
	    char * pwd;		/* Users password */
            char * host;
	    time_t login_time;
	    time_t last_cmd_time; /* Used to calc idle time */
	} usr;
    } det;
} Conn;

/*
 * Cmds are commands which are available to clients in the protocol.
 * There are 2 types; internal & external. Internal ones are defined
 * in cmds.c, whereas external ones are provided by modules.
 */
typedef struct cmd_struct * Cmdp;
typedef enum {internal, external} Cmdtype;
typedef struct cmd_struct {
    char *	name;		/* Nearly forgot this one! */
    Cmdtype	type;
    union {
	struct {
	    void (*fn)(char **);
	} in;			/* Internal command */
	struct {
            /* Should change this to be just an fd, for quicker access
               to the module. */
	    Connp mod;
	} ex;			/* External command */
    } det;			/* details */
} Cmd;

/*********************************************************************
 * Globals
 */

/* spider.c */
/* A pointer to argv[0] */
extern char * 	fullname;
extern Bool	debug;
/* filedesc of where the msg came from */
extern int 	sender;
extern int	receiver;
/* A list of fds to select(2) on */
extern fd_set	wait_on;
/* Highest numbered fd in wait_on */
extern int	wait_on_max;
/* See if we have had a signal */
extern int	caught_sig;

/* init.c */
extern char *	conf_file;
extern Bool	want_to_fork;
extern Bool	am_daemon;
extern char * 	pid_file;
extern char * 	user_file;
extern char **  module_path;
extern char **  modules;
extern int	maxfd;
extern unsigned short	port;
extern char *	spool_dir; 
extern Bool	log_all_cmds;

/* term.c */
int	child_status;
pid_t	child_pid;

/* ds.c */
/* Array of connections */
extern Connp *	open_conns;

/*********************************************************************
 * Prototypes
 */

/* cmds.c */
void 	cmd_help(char ** input);
void 	cmd_login(char ** input);
void 	cmd_who(char ** input);
void 	cmd_quit(char ** input);

/* ds.c */
Connp 	carr_find(char * name, Conntype type);
int 	carr_findi(char * name, Conntype type);
Bool 	carr_del(char * name, Conntype type);
int 	arr_count(char ** arr);
char ** arr_add(char ** arr, char * str);
void 	arr_del(char ** arr);
void	hash_init(Hashp h, Bool indep);
unsigned	hash(char * s, Hashp h);
void *	hash_lookup(char *s, Hashp h);
Bool	hash_insert(char * name, void * data, Hashp h);
void *	hash_delete(char * name, Hashp h);
void	hash_stat(char * name, Hashp h);
void	hash_visit(void (* func)(void * data), Hashp h);
void 	Cmd_init(void);
Cmdp	Cmd_find(char * name);
Bool	Cmd_add(char *name, Cmdp data);
void	Cmd_delete(char * name);
void 	Cmd_stat(void);
void	Cmd_visit(void (*func)(void *));
void 	Usr_init(void);
Connp	Usr_find(char * name);
Bool	Usr_add(char *name, Connp data);
void 	Usr_stat(void);
void	Usr_visit(void (*func)(void *));
void	conn_init_buf(Connp);
void	conn_grow_buf(Connp, int);

/* init.c */
void 	spider_init(void);
void 	go_daemon(void);
void 	ck_config(void);
void	set_facility(char *);
void 	parse_cfg_file(void);
void	init_data(void);
void	init_internal_cmds(void);
char * 	find_module(char * name);
void 	activate_module(char * path);
void 	neg_cmd(char * buf, Connp mod);
void 	proc_new_cmds(char *modname);
void 	init_modules(void);
void 	init_users(void);
void 	write_pid_file(void);

/* io.c */
void	put_mesg(FILE *fp, char **msg);
char **	get_mesg(FILE *fp);
Bool	input_data(Connp c);
char *	get_line(FILE * fp);
int	conn_single_read(Connp c);
int	conn_read(Connp c);

/* net.c */
int	spider_listen(int port);
void	spider_accept(int fd, void *data);
void	spider_read(int fd, void *data);

/* spider.c */
Bool	new_conn(int l_sock);
Bool	validate_user(char ** input);
Bool	validate_mod(char ** input);
void	add_username(char ** msg);
void	del_username(char ** msg);
int	get_active_fd(fd_set * set);
Event	figure_out_event_type(int sel_val, fd_set * tmp_set, int listener, int sig);

/* term.c */
void 	rm_pid_file(void);
RETSIGTYPE	the_end(int i);
void	term_conn(void);
void	shutdown_all(void);
RETSIGTYPE	reap_child(int i);
RETSIGTYPE	do_stats(int i);

/* utils.c */
void	mk_a_dir(char * name, int mode);
#ifndef HAVE_STRDUP
char *	strdup(char * s);
#endif
Bool 	ck_buf(char * buf, int size);
Bool	eot(char * c);
char * 	basename(char * argv0);
void 	kill_comment(char * buf);
Bool 	valid_chars(char * s);
int	get_max_fds(void);
char *	make_repline(int code, char * msg);
char ** make_error(int code, char * msg);
void	up_str(char * p);
void	down_str(char * p);
char * 	skip_ws(char * p);
char * 	skip_until_ws(char * p);
size_t 	len_token(char * tok);
char * 	find_token(char * buf, int n);
char * 	copy_token(char * buf, int n);
int 	num_tokens(char * buf);
Bool	cmp_token(char * buf, int n, char * s);
void	debug_log(char * msg, ...);

#endif /* _SPIDER_H_ */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
