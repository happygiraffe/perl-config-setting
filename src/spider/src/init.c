/*********************************************************************
 * init.c
 *
 * Contains all the functions for setting up the process, like parsing
 * the config files and spawning modules, etc.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: init.c,v 1.19 2000/01/18 07:49:01 dom Exp $";

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
Bool	want_to_fork;
Bool	am_daemon = false;	/* true when we've forked */
int	maxfd;

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
    char *spool_dir;

    /* parse the config_file into a structure */
    config_parse_file();

    if (want_to_fork)
    {
	go_daemon();		/* put ourselves into the background */
    }

    /* Now, we must open a syslog connection, as we are blind & dumb */
    openlog (basename(fullname), LOG_NDELAY, get_facility());
    log (LOG_INFO, "Starting v%s", VERSION);

    spool_dir = config_get("Spool_Dir");
    if(chdir(spool_dir) < 0) {
        log (LOG_ERR, "Couldn't change to %s: %m", spool_dir);
        exit(1);
    }
    write_pid_file();
    atexit(rm_pid_file);

    /* Put in signal handlers for common things */
    if (signal(SIGHUP, the_end) == SIG_ERR) {
        log (LOG_WARNING, "Couldn't install signal handler for HUP: %m");
    }
    if (signal(SIGTERM, the_end) == SIG_ERR) {
        log (LOG_WARNING, "Couldn't install signal handler for TERM: %m");
    }
    if (signal(SIGCHLD, reap_child) == SIG_ERR) {
        log (LOG_WARNING, "Couldn't install signal handler for CHILD: %m");
    }
    if (signal(SIGUSR1, do_stats) == SIG_ERR) {
        log (LOG_WARNING, "Couldn't install signal handler for USR1: %m");
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
 * write_pid_file
 *
 * Puts the value of our Pid into the pid file. 
 */
void
write_pid_file(void)
{
    char *	pid_file;
    FILE *	pf;
    int		i;
    pid_t	old_pid;
    struct stat	statbuf;

    pid_file = config_get("Pid_File");
    i = stat(pid_file, &statbuf);
    if (i < 0) {
        /* If file doesn't exist */
        pf = fopen(pid_file, "w");
        if (pf == NULL) {
            log (LOG_WARNING, "Error opening pid file %s: %m", pid_file);
	} else {
	    fprintf(pf, "%ld\n", (long)getpid());
	    fclose(pf);
	}
    } else {
        /* File exists, read it to see if it is really in use */
        pf = fopen(pid_file, "r+");
        if (pf == NULL)
            log (LOG_WARNING, "Error opening pid file %s: %m", pid_file);
        fscanf(pf, "%ld", (long *)&old_pid);
        if ((old_pid > 0) && (kill(old_pid, 0) == 0)) {
            /* Eeek, spider is already running */
            log (LOG_WARNING, "Spider is already running as pid %ld",
		 (long)old_pid);
            exit (1);
        }
        /* OK, we can write our own pid in. */
        fseek(pf, 0, SEEK_SET);
	fprintf(pf, "%ld\n", (long)getpid());
	fclose(pf);
    }
    return;
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
    open_conns = emalloc (maxfd * sizeof(Connp));
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

    log (LOG_DEBUG, "start init_internal_commands");

    tmp = emalloc (sizeof(Cmd));
    tmp->name = "LOGIN";
    tmp->type = internal;
    tmp->det.in.fn = cmd_login;
    Cmd_add(tmp->name, tmp);
    log (LOG_DEBUG, "cmd %s ok", tmp->name);

    tmp = emalloc (sizeof(Cmd));
    tmp->name = "HELP";
    tmp->type = internal;
    tmp->det.in.fn = cmd_help;
    Cmd_add(tmp->name, tmp);
    log (LOG_DEBUG, "cmd %s ok", tmp->name);

    tmp = emalloc (sizeof(Cmd));
    tmp->name = "WHO";
    tmp->type = internal;
    tmp->det.in.fn = cmd_who;
    Cmd_add(tmp->name, tmp);
    log (LOG_DEBUG, "cmd %s ok", tmp->name);

    tmp = emalloc (sizeof(Cmd));
    tmp->name = "QUIT";
    tmp->type = internal;
    tmp->det.in.fn = cmd_quit;
    Cmd_add(tmp->name, tmp);
    log (LOG_DEBUG, "cmd %s ok", tmp->name);

    log (LOG_DEBUG, "end init internal_commands");
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
    char **	module_path;
    struct stat	st;

    if (name != NULL) {
	module_path = config_get("Module_Dir");
	for (i = 0 ; module_path[i] != (char*)NULL; i++)
	{
	    len = strlen(module_path[i]) + 1 + strlen(name) + 1;
	    abs_name = emalloc ((size_t)len);
	    strcpy(abs_name, module_path[i]);
	    strcat(abs_name, "/");
	    strcat(abs_name, name);

	    retval = stat(abs_name, &st);
	    /* fail if it doesn't have exec perms */
	    if (retval == -1 ||	!(st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) )
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
	    log (LOG_WARNING, "socketpair failed; module %s not loaded",
		 path);
	    return;
	}

	if ( (pid = fork()) == -1) {
	    log (LOG_WARNING, "fork failed; reason %m; module %s not loaded",
		 path);
	    return;
	} else if ( pid == 0 ) {
	    /* Child */
	    signal(SIGTRAP, SIG_IGN);
	    close(fd[0]);
	    i = dup2(fd[1], STDIN_FILENO);
	    if (i != STDIN_FILENO) {
		log (LOG_WARNING, "dup2(,stdin) failed");
		exit(1);
	    }
	    i = dup2(fd[1], 1);
	    if (i != STDOUT_FILENO) {
		log (LOG_WARNING, "dup2(,stdout) failed");
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
	    log (LOG_WARNING, "exec failed; reason %m");
	} else {
	    /* Parent */
	    close(fd[1]);
	    thisone = emalloc (sizeof(Conn));
	    thisone->eol = "\n";
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
	    
	    log (LOG_INFO, "module %s is pid %d fd %d", thisone->name,
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
	cmd = emalloc(sizeof(Cmd));
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
	tmp = emalloc (strlen(REPLY_FMT)+strlen(reason)+6);
	(void)sprintf(tmp, REPLY_FMT "\n", code, reason);
	fputs(tmp, mod->chan);
	log (LOG_DEBUG, "[%d] -> %s", fileno(mod->chan), tmp);
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
	log (LOG_DEBUG, "proc_new_cmds(%s) starting", modname);

	/* I/O with the module */
	do {
            if (buf != NULL) {
                free(buf);
            }
            buf = get_line(mod->chan);
	    log (LOG_DEBUG, "[%d] <- %s", fileno(mod->chan), buf);
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
    char **	modules;

    modules = config_get ("Module");
    for (i = 0; modules[i] != NULL; i++)
    {
	mod = find_module(modules[i]);
	if (mod == NULL)
	{
	    log (LOG_ERR, "Module %s not present or not executable",
		 modules[i]);
	    continue;		/* XXX Should we exit(1) here? */
	}
	log (LOG_DEBUG, "start init module %s", modules[i]);
	activate_module(mod);
	proc_new_cmds(basename(mod));
	log (LOG_DEBUG, "end init module %s", modules[i]);
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
    char *	user_file;
    int		lineno;
    int		len;
    Connp	usr;
    struct stat	st;

    user_file = config_get("User_File");
    if (stat(user_file, &st) == -1)
    {
	log (LOG_ERR, "Could not find user file %s.", user_file);
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
	    log (LOG_WARNING, "Problem with User File on line %d",
		 lineno);
            free(buf);
            buf = get_line(usrf);
	    lineno++;
	    continue;
	}
	usr = emalloc (sizeof(Conn));
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
