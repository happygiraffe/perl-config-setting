/*********************************************************************
 * term.c
 *
 * Handle terminating connections and shutting down in various ways.
 * This includes signal handlers.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: term.c,v 1.1 1999/03/11 15:39:49 dom Exp $";

#include <sys/types.h>
#include <config.h>
#include <sys/socket.h>
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
/* autoconf recommended portability ugliness */
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include "spider.h"

/* Globals */
/*
 * caught_sig *must* be initialized to zero for correct functioning of
 * Spider's main loop...
 */
int	caught_sig = 0;
int	child_status;
pid_t	child_pid;

/* static helper functions */
static void term_user(void);
static void term_module(void);
static void undo_cmds(void *);

/*********************************************************************
 * term_conn
 *
 * Terminates the current connection (as specified by sender).
 */
void
term_conn(void)
{
    Connp 	tmp;
    
    tmp = SENDER_CONN;
    if (tmp->type == user) {
        shutdown(sender, 2);
    }
    fclose(tmp->chan);
    tmp->chan = NULL;
    tmp->state = s_not;
    if (tmp->type == user) {
        term_user();
    } else {
        term_module();
    }
    SENDER_CONN = NULL;
    FD_CLR(sender, &wait_on);
    CLR_WAIT_MAX(sender);
}

/*********************************************************************
 * term_user
 *
 * Performs any cleaning up when a user logs off
 */
static void
term_user(void)
{
    Connp	tmp;

    tmp = SENDER_CONN;
    if (tmp->name != NULL) {
        syslog(LOG_INFO, "user %s: logout", tmp->name);
        free(tmp->det.usr.host);
        tmp->det.usr.host = NULL;
        tmp->det.usr.login_time = 0;
        tmp->det.usr.last_cmd_time = 0;
    } else {
        /* Incomplete user from aborted login */
        free(tmp->det.usr.host);
        free(tmp);
    }
}

/*********************************************************************
 * term_module
 *
 * Performs any cleaning up when a module dies
 */
static void
term_module(void)
{
    Connp	tmp;

    tmp = SENDER_CONN;

    /* Remove all the commands that depend on this module */
    Cmd_visit(undo_cmds);
    
    /* Log what happened to the damn child. */
    if (WIFEXITED(child_status)) {
        /* normal exit */
        syslog(LOG_INFO, "module %s exited with status %d", tmp->name,
               WEXITSTATUS(child_status));
    }
    if (WIFSIGNALED(child_status)) {
        syslog(LOG_INFO, "module %s killed by signal %d", tmp->name,
               WTERMSIG(child_status));
    }
    child_status = 0;
    child_pid = 0;
    caught_sig = 0;

    free(tmp->name);
    free(tmp);
}
 
/*********************************************************************
 * undo_cmds
 *
 * Remove all commands which originate from a certain module.
 */
static void
undo_cmds(void * cmd)
{
    Cmdp	tmp;

    tmp = (Cmdp)cmd;
    if (tmp->type == external)
    {
	if (cmp_token(tmp->det.ex.mod->name, 0, SENDER_NAME))
	{
	    Cmd_delete(tmp->name);
	}
    }
}

/*********************************************************************
 * the_end
 *
 * Just used as a signal handler to terminate the program.
 */
RETSIGTYPE
the_end(int i)
{
    caught_sig = i;
}

/*********************************************************************
 * reap_child
 *
 * Tidy up and acknowledge a child processes termination.
 */
RETSIGTYPE
reap_child(int i)
{
    signal(SIGCHLD, reap_child);
    child_pid = waitpid(0, &child_status, WNOHANG);
}

/*********************************************************************
 * the_end
 *
 * Just used as a signal handler to terminate the program.
 */
RETSIGTYPE
do_stats(int i)
{
    Cmd_stat();
    Usr_stat();
}

/*********************************************************************
 * rm_pid_file
 *
 * Remove the pid file.  This function should not be called
 * directly. only by the code associated with 'atexit'.
 */
void
rm_pid_file(void)
{
    /* Don't worry if it fails; there's probably a good reason for it */
    unlink(pid_file);
    syslog (LOG_INFO, "Stopping");
}

/*********************************************************************
 * shutdown_all
 *
 * Doom, gloom & the end of the program.
 */
void
shutdown_all(void)
{
    int		i;

    signal(SIGCHLD, SIG_IGN);
    
    for (i = 0; i < maxfd; i++) {
        if (open_conns[i] != NULL) {
            fclose (open_conns[i]->chan);
        }
    }
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
