/*********************************************************************
 * cmds.c
 *
 * The functions for all the internal commands.  These get activated
 * before any modules, and so cannot be overridden.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: cmds.c,v 1.2 1999/03/17 07:43:53 dom Exp $";

/*
 * The following commands are implemented here:
 * 
 * HELP	- List all commands that we know about.
 * LOGIN	- Init a connection.
 * WHO	- List who's logged on to the system.
 * QUIT	- Tear down the connection.
 * 
 * The commands all have the following prototype:
 * 
 * void
 * cmd_xxx(char ** req) { }
 * 
 * req is a standard pointer to an array of pointers to strings.  The
 * function must call send_reply to return a value.
 * 
 */

#include <config.h>             /* autoconf */
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
#include <sys/types.h>          /* select */
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
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

#include "spider.h"

/* static functions are visible only within this file */
static void send_reply(char ** reply);
static void help_helper(void * cur);
static char * who_helper(Connp usr);

/* static global variables are only visible from within this file */
static char **	help_reply;

/*********************************************************************
 * send_reply
 *
 * Sends a reply to a client.  The reply is a pointer to a dynamically
 * allocated array of pointers to strings.  It is sent to the person
 * found by the global variable receiver.
 */
static void
send_reply(char ** reply)
{
    char **	c;
    Connp	usr;

    usr = RECEIVER_CONN;
    for(c = reply; *c != NULL; ++c) {
	fputs(*c, usr->chan);
	fputs("\r\n", usr->chan);
    }
}

/*********************************************************************
 * help_helper
 *
 * Adds each command onto an array, to be output for the help
 * function.  This function should only be called by cmd_tree_trav,
 * which will visit each node on the command tree with it.
 *
 * WARNING: Because of the way this uses global variables, it is 
 *          *not* thread safe.
 */
static void
help_helper(void * cur)
{
    char * 	c;
    Cmdp	foo;

    foo = (Cmdp)cur;
    c = strdup(foo->name);
    help_reply = arr_add(help_reply, c);
}

/*********************************************************************
 * cmd_help
 *
 * Print up a list of all known commands on this system.
 *
 * Uses the global variable "help_reply".
 */
void
cmd_help(char ** input)
{
    help_reply = NULL;

    /* First, make the reply code header */
    help_reply = arr_add(help_reply, make_repline(INF_HELP, INF_HELP_MSG));

    /* Find a list of commands */
    Cmd_visit(help_helper);

    /* And finish with the EOD mark */
    help_reply = arr_add(help_reply, strdup(END_OF_DATA));

    /* Now, send it out */
    send_reply(help_reply);
    arr_del(help_reply);
}

/*********************************************************************
 * cmd_login
 *
 * Login a user.  If failure, break connection.  Assumes that usr
 * struct in open_conns has already been filled in with a dummy value,
 * which allows us to communicate down that channel with stdio.
 *
 * XXX - allow for new user!
 */
void
cmd_login(char ** input)
{
    char **	reply = NULL;
    char * 	name;
    char *	passwd;
    Connp	usr;
    Bool	ok = true;

    /* If we don't need a LOGIN, then make sure we ain't got one */
    if (ok && (SENDER_CONN->state != s_init)) {
	ok = false;
        reply = make_error(ERR_LOGIN, ERR_LOGIN_MSG);
        send_reply(reply);
        arr_del(reply);
    }
    /* Correct syntax check */
    if (num_tokens(input[0]) < 3) {
        ok = false;
        reply = make_error(ERR_SYNTAX, ERR_SYNTAX_MSG);
        send_reply(reply);
        arr_del(reply);
        term_conn();
    }

    name = copy_token(input[0], 1);
    passwd = copy_token(input[0], 2);
    usr = Usr_find(name);

    /* Login failure - unknown user */
    if (ok && (strcasecmp(name, usr->name) != 0)) {
        ok = false;
        reply = make_error(ERR_BADUSR, ERR_BADUSR_MSG);
        send_reply(reply);
        arr_del(reply);
        term_conn();
    } 
    /* User already logged on once */
    if (ok && (usr->chan != NULL)) {
        ok = false;
        reply = make_error(ERR_TOOMANY, ERR_TOOMANY_MSG);
        send_reply(reply);
        arr_del(reply);
        term_conn();
    }
    /* Got user, check passwd */
    if (ok && strcmp(passwd, usr->det.usr.pwd)) {
        ok = false;
        reply = make_error(ERR_BADPWD, ERR_BADPWD_MSG);
        send_reply(reply);
        arr_del(reply);
        term_conn();
    }
    /* Yahoo, now we can try logging them in proper, like */
    if(ok) {
        usr->chan = SENDER_CHAN;
        usr->det.usr.host = SENDER_CONN->det.usr.host;
	usr->state = s_conn;
        /* Dispose of dummy entry */
        free(SENDER_CONN);
        SENDER_CONN = usr;
        usr->det.usr.last_cmd_time = usr->det.usr.login_time = time(0);
        reply = make_error(OK_HELLO, OK_HELLO_MSG);
        send_reply(reply);
        arr_del(reply);
        syslog(LOG_INFO, "user %s: login from %s", usr->name,
               usr->det.usr.host);
    }
    free(name);
    free(passwd);
}

/*********************************************************************
 * who_helper
 *
 * Assists the who_cmd function by preparing a string.
 */
static char *
who_helper(Connp usr)
{
    char *	login_time;
    char *	idle_time;
    char *	reply;
    time_t	now;
    struct tm *	tmp;
    int 	line_len = 63;
    int		hrs;
    int		mins;

    time(&now);
    login_time = malloc(6);	/* HH:MM\0 */
    if (login_time == NULL) {
	    syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
		   __FILE__);
	    exit(1);
    }
    idle_time = malloc(6);	/* HH:MM\0 */
    if (idle_time == NULL) {
	    syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
		   __FILE__);
	    exit(1);
    }
    reply = malloc((size_t)line_len+1);
    if (reply == NULL) {
	    syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
		   __FILE__);
	    exit(1);
    }

    tmp = localtime(&usr->det.usr.login_time);
    sprintf(login_time, "%02d:%02d", tmp->tm_hour, tmp->tm_min);

    tmp = localtime(&usr->det.usr.last_cmd_time);
    hrs = tmp->tm_hour;
    mins = tmp->tm_min;
    tmp = localtime(&now);
    sprintf(idle_time, "%02d:%02d", tmp->tm_hour - hrs,
	    abs(tmp->tm_min - mins));

    sprintf(reply, "%-.20s %5.5s %5.5s %-.30s", usr->name,
            login_time, idle_time, usr->det.usr.host);
    free(login_time);
    free(idle_time);

    return reply;
}

/*********************************************************************
 * cmd_who
 *
 * Print up a list of all known users on this system.
 *
 * Like cmd_help, makes use of a global variable and a helper
 * function.
 */
void
cmd_who(char ** input)
{
    char **	reply = NULL;
    char *	header = "Name Login_Time Idle_Time Host";
    int		i;

    /* First, make the reply code header */
    reply = arr_add(reply, make_repline(OK_CMD, OK_CMD_MSG));

    /* Print a header */
    reply = arr_add(reply, strdup(header));

    /* Flush out the active users */
    for(i = 0; i < maxfd; ++i) {
	if (open_conns[i] != NULL) {
	    if ((open_conns[i]->type == user) &&
		(open_conns[i]->state == s_conn)) {
		reply = arr_add(reply, who_helper(open_conns[i]));
	    }
	}
    }

    /* And finish with the EOD mark */
    reply = arr_add(reply, strdup(END_OF_DATA));

    /* Now, send it out */
    send_reply(reply);
    arr_del(reply);
}

/*********************************************************************
 * cmd_quit
 *
 * Tear down a connection and remove user from BBS.  Pretty safe to
 * ignore the buffer here, as this don't take arguments.
 */
void
cmd_quit(char ** input)
{
    char **	reply;

    reply = make_error(OK_BYE, OK_BYE_MSG);
    send_reply(reply);
    arr_del(reply);
    term_conn();
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
