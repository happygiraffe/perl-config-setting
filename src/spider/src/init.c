/*********************************************************************
 * init.c
 *
 * Contains all the functions for setting up the process, like parsing
 * the config files and spawning modules, etc.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: init.c,v 1.13 2000/01/15 12:18:26 dom Exp $";

#include <config.h>             /* autoconf */

#include <sys/types.h>
#include <sys/stat.h>		/* umask, stat */
/* This ugliness recommended by autoconf for portability */
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
#include <sys/types.h>
#include <sys/socket.h>         /* socketpair */
#include <ctype.h>		/* isspace */
#include <errno.h>		/* perror */
#include <signal.h>             /* signalling stuff */
#include <stdio.h>		/* STDIO. */
#include <stdlib.h>		/* malloc, atoi */
/* This ugliness recommended by autoconf for portability */
#if STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
          char *strchr (), *strrchr ();
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif
#include <syslog.h>		/* openlog, syslog, etc */
#include <unistd.h>		/* POSIXy stuff like fork */
#include "spider.h"		/* local definitions */

/* Globals */
char *	conf_file;		/* config file name */
Bool	want_to_fork;
Bool	am_daemon = false;	/* true when we've forked */
char *  pid_file;       
char *  user_file;      
char ** module_path;  
char ** modules;      
int	maxfd;
unsigned short	port;
char *	spool_dir;
Bool	log_all_cmds = false;   /* default value */
int	facility = LOG_LOCAL0;

/* textual representation of syslog facilities */
static struct
{
    const char *name;
    int val;
} facilities[] = {
#ifdef LOG_USER
    { "user",	LOG_USER },
#endif
#ifdef LOG_MAIL
    { "mail",	LOG_MAIL },
#endif
#ifdef LOG_DAEMON
    { "daemon",	LOG_DAEMON },
#endif
#ifdef LOG_AUTH
    { "auth",	LOG_AUTH },
#endif
#ifdef LOG_LPR
    { "lpr",	LOG_LPR },
#endif
#ifdef LOG_NEWS
    { "news",	LOG_NEWS },
#endif
#ifdef LOG_UUCP
    { "uucp",	LOG_UUCP },
#endif
#ifdef LOG_CRON
    { "cron",	LOG_CRON },
#endif
#ifdef LOG_FTP
    { "ftp",	LOG_FTP },
#endif
#ifdef LOG_NTP
    { "ntp",	LOG_NTP },
#endif
#ifdef LOG_LOCAL0
    { "local0",	LOG_LOCAL0 },
#endif
#ifdef LOG_LOCAL1
    { "local1",	LOG_LOCAL1 },
#endif
#ifdef LOG_LOCAL2
    { "local2",	LOG_LOCAL2 },
#endif
#ifdef LOG_LOCAL3
    { "local3",	LOG_LOCAL3 },
#endif
#ifdef LOG_LOCAL4
    { "local4",	LOG_LOCAL4 },
#endif
#ifdef LOG_LOCAL5
    { "local5",	LOG_LOCAL5 },
#endif
#ifdef LOG_LOCAL6
    { "local6",	LOG_LOCAL6 },
#endif
#ifdef LOG_LOCAL7
    { "local7",	LOG_LOCAL7 },
#endif
    { NULL, 0}
};


/*********************************************************************
 * spider_init()
 *
 * Organizes the setting up of the program.  Sets up daemonizing,
 * signals and then proceeds to parse & act upon the configuration
 * file.  Any particular order, really.
 */
void
spider_init(void)
{
    /* Parse the config_file into the global variables */
    parse_cfg_file();

    if (want_to_fork)
    {
	go_daemon();		/* put ourselves into the background */
    }

    /* Now, we must open a syslog connection, as we are blind & dumb */
    openlog(basename(fullname), LOG_NDELAY, facility);
    syslog(LOG_INFO, "Starting v%s", VERSION);

    if(chdir(spool_dir) < 0) {
        syslog(LOG_ERR, "Couldn't change to %s: %m", spool_dir);
        exit(1);
    }
    write_pid_file();
    atexit(rm_pid_file);

    /* Put in signal handlers for common things */
    if (signal(SIGHUP, the_end) == SIG_ERR) {
        syslog(LOG_WARNING, "Couldn't install signal handler for HUP: %m");
    }
    if (signal(SIGTERM, the_end) == SIG_ERR) {
        syslog(LOG_WARNING, "Couldn't install signal handler for TERM: %m");
    }
    if (signal(SIGCHLD, reap_child) == SIG_ERR) {
        syslog(LOG_WARNING, "Couldn't install signal handler for CHILD: %m");
    }
    if (signal(SIGUSR1, do_stats) == SIG_ERR) {
        syslog(LOG_WARNING, "Couldn't install signal handler for USR1: %m");
    }

    /* Initialize data structures */
    init_data();

    /* Read in the user database for quick & easy access */
    init_users();

    /* Initialize the commands that are provided by us */
    init_internal_cmds();

    /* Set up the modules & connections to them.  This will also fill
     * up the commands tree. */
    init_modules();
}

/*********************************************************************
 * go_daemon
 *
 * Exits parent, removes controlling terminal and sets up to be a long
 * lived process.
 */
void
go_daemon(void)
{
    pid_t pid;
    int i;

    pid = fork();
    switch(pid) {
    case -1:
	perror("fork");
	exit(1);
	break;			/* Just for stylistic reasons */
    case 0:			/* Child process */
	pid = getpid();
	break;
    default:			/* Parent process */
	exit(0);
    }

    /* Become a session leader - and also become pgrp leader at the
     * same time.  */
    setsid();
    umask(0);

    /* Close unnecessary fd's */
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
    maxfd = get_max_fds();
    for  (i = 0; i < maxfd; ++i) {
	close(i);
    }

    am_daemon = true;
}

/*********************************************************************
 * ck_config
 *
 * Checks that all the configuration variables are set, so that we may
 * proceed correctly.
 */
void
ck_config(void)
{
    /* Sanity check, again */
    if (module_path == NULL) {
	syslog(LOG_ERR, "No Module_Path defined");
	exit(1);
    }
    if (modules == NULL) {
	syslog(LOG_ERR, "No Modules defined");
	exit(1);
    }
    if (pid_file == NULL) {
	syslog(LOG_ERR, "No Pid_File defined");
	exit(1);
    }
    if (port == 0) {
	syslog(LOG_ERR, "No Port defined");
	exit(1);
    }
    if (user_file == NULL) {
	syslog(LOG_ERR, "No User_File defined");
	exit(1);
    }
}

/*********************************************************************
 * set_facility
 *
 * Set the syslog facility from a string.
 */
void
set_facility(char *fac)
{
    int i;

    if (fac != NULL)
	for (i = 0; facilities[i].name != NULL; i++)
	    if (strcasecmp(facilities[i].name, fac) == 0) {
		facility = facilities[i].val;
		break;
	    }
    return;
}


/*********************************************************************
 * parse_cfg_file
 *
 * Reads in the file as specified by conf_file (and it's default,
 * CONFIG_FILE) and places the values contained therein into global
 * variables.
 */
void
parse_cfg_file(void)
{
    FILE *	cfg;
    char *	keyw;
    char * 	buf;
    char *	name;
    int 	i,
    		lineno = 1;

    /* Initialize variables to sane values */
    pid_file = NULL;
    user_file = NULL;
    module_path = NULL;
    modules = NULL;
    port = NULL;
    spool_dir = NULL;
    log_all_cmds = false;

    cfg = fopen(conf_file, "r");
    if(cfg == NULL) {
	perror("fopen");
	exit(1);
    }

    buf = get_line(cfg);
    while (!feof(cfg)) {
	/* Skip over comment only / blank lines */
	kill_comment(buf);
	i = num_tokens(buf);
	if (i == 0)
	{
            free(buf);
            buf = get_line(cfg);
	    lineno++;
	    continue;
	}

	/*
         * Parse it!  Remember that tokens are specified as zero-based
         * integers, like strings subscripts.  However, also like
         * string subscripts strlen/len_token returns a 1-based
         * value.
         */

	/* Please excuse the horrid indentation. */

	if (cmp_token(buf, 0, "Module_Dir") == true) {
	    name = copy_token(buf, 1);
	    debug_log("Module_Dir %s", name);
	    module_path = arr_add(module_path, name);
	} else if (cmp_token(buf, 0, "Module") == true) {
	    name = copy_token(buf, 1);
	    debug_log("Module %s", name);
	    modules = arr_add(modules, name);
	} else if (cmp_token(buf, 0, "Pid_File") == true) {
	    name = copy_token(buf, 1);
	    debug_log("Pid_File %s", name);
	    pid_file = name;
	} else if (cmp_token(buf, 0, "User_File") == true) {
	    name = copy_token(buf, 1);
	    debug_log("User_File %s", name);
	    user_file = name;
	} else if (cmp_token(buf, 0, "Port") == true) {
	    name = find_token(buf, 1);
	    debug_log("Port %s", name);
	    port = atoi(name);
	} else if (cmp_token(buf, 0, "Spool_Dir") == true) {
	    name = copy_token(buf, 1);
	    debug_log("Spool_Dir %s", name);
	    spool_dir = name;
	} else if (cmp_token(buf, 0, "Log_All_Cmds") == true) {
	    debug_log("Log_All_Cmds");
	    log_all_cmds = true;
	} else if (cmp_token(buf, 0, "Facility") == true) {
	    name = find_token(buf, 1);
	    debug_log("Facility %s", name);
	    set_facility(name);
	} else {
            keyw = copy_token(buf, 0);
	    syslog(LOG_WARNING, "Unknown keyword \"%s\" on line %d", 
                   keyw, lineno);
            free(keyw);
	}

        free(buf);
        buf = get_line(cfg);
	lineno++;
    } /* End of "while(!feof(cfg)) {" */

    /* We need all the fd's we can get... */
    fclose(cfg);
    ck_config();
}

/*********************************************************************
 * write_pid_file
 *
 * Puts the value of our Pid into the pid file. 
 */
void
write_pid_file(void)
{
    FILE *	pf;
    int		i;
    pid_t	old_pid;
    struct stat	statbuf;

    i = stat(pid_file, &statbuf);
    if (i < 0) {
        /* If file doesn't exist */
        pf = fopen(pid_file, "w");
        if (pf == NULL) {
            syslog(LOG_WARNING, "Error opening pid file %s: %m", pid_file);
	} else {
	    fprintf(pf, "%ld\n", (long)getpid());
	    fclose(pf);
	}
    } else {
        /* File exists, read it to see if it is really in use */
        pf = fopen(pid_file, "r+");
        if (pf == NULL)
            syslog(LOG_WARNING, "Error opening pid file %s: %m", pid_file);
	/* Not sure if the cast is the correct thing... */
        fscanf(pf, "%ld", (long *)&old_pid);
        if ((old_pid > 0) && (kill(old_pid, 0) == 0)) {
            /* Eeek, spider is already running */
            syslog(LOG_WARNING, "Spider is already running as pid %ld",
                   (long)old_pid);
            exit (1);
        }
        /* OK, we can write our own pid in. */
        fseek(pf, 0, SEEK_SET);
	fprintf(pf, "%ld\n", (long)getpid());
	fclose(pf);
    }
}

/*********************************************************************
 * init_data
 *
 * Sets up the array of Connections, which are indexed by low-level
 * file descriptor.  Also inits the fd array used by select.
 */
void
init_data(void)
{
    maxfd = get_max_fds();
    open_conns = malloc(maxfd * sizeof(Connp));
    if (open_conns == NULL) {
	syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
	       __FILE__);
	exit(1);
    }
    (void)memset(open_conns, '\0', (size_t)(maxfd * sizeof(Connp)));
    Usr_init();
    Cmd_init();
    FD_ZERO(&wait_on);
}

/*********************************************************************
 * init_internal_cmds
 *
 * Initialize the commands which we provide ourselves, without having
 * to call a module for.
 *
 * NB: Make all command names uppercase, for consistency.
 */
void
init_internal_cmds(void)
{
    Cmdp	tmp;

    debug_log("start init internal_commands");

    tmp = malloc(sizeof(Cmd));
    if (tmp == NULL) {
	syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
	       __FILE__);
	exit(1);
    }
    tmp->name = "LOGIN";
    tmp->type = internal;
    tmp->det.in.fn = cmd_login;
    Cmd_add(tmp->name, tmp);
    debug_log("cmd %s ok", tmp->name);

    tmp = malloc(sizeof(Cmd));
    if (tmp == NULL) {
	syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
	       __FILE__);
	exit(1);
    }
    tmp->name = "HELP";
    tmp->type = internal;
    tmp->det.in.fn = cmd_help;
    Cmd_add(tmp->name, tmp);
    debug_log("cmd %s ok", tmp->name);

    tmp = malloc(sizeof(Cmd));
    if (tmp == NULL) {
	syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
	       __FILE__);
	exit(1);
    }
    tmp->name = "WHO";
    tmp->type = internal;
    tmp->det.in.fn = cmd_who;
    Cmd_add(tmp->name, tmp);
    debug_log("cmd %s ok", tmp->name);

    tmp = malloc(sizeof(Cmd));
    if (tmp == NULL) {
	syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
	       __FILE__);
	exit(1);
    }
    tmp->name = "QUIT";
    tmp->type = internal;
    tmp->det.in.fn = cmd_quit;
    Cmd_add(tmp->name, tmp);
    debug_log("cmd %s ok", tmp->name);

    debug_log("end init internal_commands");
}

/*********************************************************************
 * find_module
 *
 * Searches through module_path[] to find an instance of name.
 */
char *
find_module(char * name)
{
    int 	i;
    int 	len;
    int		retval;
    char * 	abs_name = NULL;
    struct stat	st;

    if (name != NULL) {
	for (i = 0 ; module_path[i] != (char*)NULL; i++)
	{
	    len = strlen(module_path[i]) + 1 + strlen(name) + 1;
	    abs_name = malloc((size_t)len);
	    if (abs_name == NULL) {
		syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
		       __FILE__);
		exit(1);
	    }
	    strcpy(abs_name, module_path[i]);
	    strcat(abs_name, "/");
	    strcat(abs_name, name);

	    retval = stat(abs_name, &st);
	    /* fail if it doesn't have exec perms */
	    if (retval == -1 ||	!(st.mode & (S_IXUSR|S_IXGRP|S_IXOTH)) )
	    {
		/* Module doesn't exist at this location */
		free(abs_name);
		abs_name = NULL;
	    } else {
		/* We have found the module */
		break;
	    }
	}
    }

    return abs_name;
}

/*********************************************************************
 * activate_module
 *
 * Spawns a module, attaching it's stdout/stdin to a two-way pipe.
 * Then, it will proceed to initialize commands from the module until
 * it is done, then return.
 */
void
activate_module(char * path)
{
    int 	fd[2];
    pid_t 	pid;
    int 	i;
    Connp	thisone;
    struct stat	s;
    char *	name;

    if (path != NULL) {
	name = basename(path);

	i = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
	if (i == -1) {
	    syslog(LOG_WARNING, "socketpair failed; module %s not loaded",
		   path);
	    return;
	}

	if ( (pid = fork()) == -1) {
	    syslog(LOG_WARNING, "fork failed; reason %m; module %s not loaded",
		   path);
	    return;
	} else if ( pid == 0 ) {
	    /* Child */
	    signal(SIGTRAP, SIG_IGN);
	    close(fd[0]);
	    i = dup2(fd[1], STDIN_FILENO);
	    if (i != STDIN_FILENO) {
		syslog(LOG_WARNING, "dup2(,stdin) failed");
		exit(1);
	    }
	    i = dup2(fd[1], 1);
	    if (i != STDOUT_FILENO) {
		syslog(LOG_WARNING, "dup2(,stdout) failed");
		exit(1);
	    }
	    /* Make sure that the child gets it's own directory */
	    i = stat(name, &s);
	    if (i == -1) {
		mk_a_dir(name, 0755);
		chdir(name);
	    } else {
		if (S_ISDIR(s.st_mode)) {
		    chdir(name);
		} else {
		    unlink(name);
		    mk_a_dir(name, 0755);
		    chdir(name);
		}
	    }
	    execl(path, basename(path), NULL);
	    syslog(LOG_WARNING, "exec failed; reason %m");
	} else {
	    /* Parent */
	    close(fd[1]);
	    thisone = malloc(sizeof(Conn));
	    if (thisone == NULL) {
		syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
		       __FILE__);
		exit(1);
	    }
	    thisone->chan = fdopen(fd[0], "r+");
	    thisone->name = basename(path);
	    thisone->type = module;
	    /* this must be s_conn, not s_init, because of carr_find() */
	    thisone->state = s_conn;
	    thisone->det.mod.pid = pid;
	    /* Make sure that any writes don't wait */
	    setvbuf(thisone->chan, NULL, _IOLBF, 0);

	    /* Add into the array of connections. */
	    open_conns[fd[0]] = thisone;
            /* Update the wait_on set */
            FD_SET(fd[0], &wait_on);
            SET_WAIT_MAX(fd[0]);
	    
	    syslog(LOG_INFO, "module %s is pid %d fd %d", thisone->name,
		   thisone->det.mod.pid, fd[0]);
	}
    }
}

/*********************************************************************
 * neg_cmd (Negotiate)
 *
 * Sees whether or not the command that has just been given to it's
 * Father (proc_new_cmds) is valid or not, and reply.
 */
void
neg_cmd(char * buf, Connp mod)
{
    Cmdp	cmd;
    Bool	ok = true;
    char *	tmp;
    char *	reason = ERR_SYNTAX_MSG;
    int		code = ERR_SYNTAX;

    if ((buf != NULL) && (mod != NULL)) {
	/* Setup a new command structure */
	cmd = malloc(sizeof(Cmd));
	if (cmd == NULL) {
	    syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
		   __FILE__);
	    exit(1);
	}
	cmd->name = NULL;
	cmd->type = external;
	cmd->det.ex.mod = mod;

	/* Parse the result */
	if (num_tokens(buf) != 1)
	{
	    ok = false;
	    code = ERR_SYNTAX;
	    reason = ERR_SYNTAX_MSG;
	}
	/* Next, get the command name itself */
	if (ok)
	{
	    cmd->name = copy_token(buf, 0);
	    if (valid_chars(cmd->name) == false)
	    {
		ok = false;
		code = ERR_BADNAME;
		reason = ERR_BADNAME_MSG;
	    }
	}
	/* Now, reply */
	if (ok)
	{
	    up_str(cmd->name);
	    ok = Cmd_add(cmd->name, cmd);
	    if (!ok)
	    {
		code = ERR_PREVDEF;
		reason = ERR_PREVDEF_MSG;
	    }
	}
	if (ok)
	{
	    code = OK_CMD;
	    reason = OK_CMD_MSG;
	}
	tmp = malloc(strlen(REPLY_FMT)+strlen(reason)+6);
	if (tmp == NULL) {
	    syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
		   __FILE__);
	    exit(1);
	}
	(void)sprintf(tmp, REPLY_FMT "\n", code, reason);
	fputs(tmp, mod->chan);
	debug_log("[%d] -> %s", fileno(mod->chan), tmp);
	free(tmp);
    }
}

/*********************************************************************
 * proc_new_cmds
 *
 * Do protocol on new commands from a module.
 */
void
proc_new_cmds(char *modname)
{
    Connp	mod;
    char * 	buf = NULL;
    Bool	ok = true;

    if (modname != NULL) {
	/* Find out whom we're dealing with */
	mod = carr_find(modname, module);
	debug_log("proc_new_cmds(%s) starting", modname);

	/* I/O with the module */
	do {
            if (buf != NULL) {
                free(buf);
            }
            buf = get_line(mod->chan);
	    debug_log("[%d] <- %s", fileno(mod->chan), buf);
	    /* Solaris stdio needs this; probably others, too.  If it
             * doesn't get it, then we end up writing our last input
             * back out! */
	    fflush(mod->chan);
	    if ((buf == NULL) || ((buf[0] == '.') && (strlen(buf) == 1)))
		ok = false;
	    else
		neg_cmd(buf, mod);
	} while (ok);
    }
}

/*********************************************************************
 * init_modules
 *
 * For each of the modules in modules[], traverse module_path[] to
 * find it.  Then, load the thing into memory, setting up a pipe to
 * it.
 */
void
init_modules(void)
{
    int 	i;
    char * 	mod;		/* Absolute name of the module */

    for (i = 0; modules[i] != NULL; i++)
    {
	mod = find_module(modules[i]);
	if (mod == NULL)
	{
	    syslog(LOG_ERR, "Module %s not present or not executable",
		   modules[i]);
	    continue;		/* XXX Should we exit(1) here? */
	}
	debug_log("start init module %s", modules[i]);
	activate_module(mod);
	proc_new_cmds(basename(mod));
	debug_log("end init module %s", modules[i]);
    }
}

/*********************************************************************
 * init_users
 *
 * Reads a list of all the users into memory.
 */
void
init_users(void)
{
    FILE * 	usrf;
    char *	buf;
    int		lineno;
    int		len;
    Connp	usr;
    struct stat	st;

    if (stat(user_file, &st) == -1)
    {
	syslog(LOG_ERR, "Could not find user file %s.", user_file);
	exit(1);
    } 

    usrf = fopen(user_file, "r");
    buf = get_line(usrf);
    lineno = 1;
    while (!feof(usrf))
    {
	kill_comment(buf);
	len = num_tokens(buf);
	if (len == 0)	/* Empty line */
	{
            free(buf);
            buf = get_line(usrf);
	    lineno++;
	    continue;
	}
	if (len != 2)
	{
	    syslog(LOG_WARNING, "Problem with User File on line %d",
		   lineno);
            free(buf);
            buf = get_line(usrf);
	    lineno++;
	    continue;
	}
	usr = malloc(sizeof(Conn));
	if (usr == NULL) {
	    syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
		   __FILE__);
	    exit(1);
	}
	/* Assume that name has already been scanned for valid
	   entries, before being entered into this file. */
	usr->chan = NULL;
	usr->name = copy_token(buf, 0);
	usr->type = user;
	usr->state = s_not;
	usr->det.usr.pwd = copy_token(buf, 1);
	Usr_add(usr->name, usr);
        free(buf);
        buf = get_line(usrf);
	lineno++;
    }

    fclose(usrf);
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
