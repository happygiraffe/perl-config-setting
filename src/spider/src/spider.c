/*********************************************************************
 * spider.c
 *
 * The main BBS multiplexer.  Maybe it's easier in C? (HAHAHAHAHA)
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: spider.c,v 1.1 1999/03/11 15:39:49 dom Exp $";

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
#include <sys/socket.h>
#include <netinet/in.h>         /* ntohs */
#include <arpa/inet.h>          /* inet_ntoa */
#include <ctype.h>
#include <errno.h>
#include <netdb.h>              /* host lookups */
#include <signal.h>
#include <stdio.h>		/* STDIO. */
#include <stdlib.h>
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
#include <syslog.h>
#include <time.h>
#include <unistd.h>             /* POSIX kernel functions */
#include "spider.h"		/* local definitions */

/* GLOBALS */
/* argv[0], basically. */
char *	fullname;
/* File descriptor of where the msg came from & is going to.  Can
 *  obtain name from open_conns[sender]->name XXX This is most
 * un-thread-safe. */
int 	sender;
int 	receiver;
/* This is a list of all the fds we wish to select on */
fd_set 	wait_on;
/* Highest numbered fd in wait_on */
int	wait_on_max;

/*********************************************************************
 * main()
 *
 * Perform initialization, and then sit in a loop, waiting for
 * connections/requests and then processing them.
 */
int
main(int argc, char **argv)
{
    char **	msg = NULL;
    Cmdp	cmd = NULL;
    Connp	usr = NULL;
    fd_set	tmp_set;
    int		l_sock;         /* listening socket */
    struct sockaddr_in	server;
    int		opt;		/* option flag */
    int 	sel_val;	/* return value from select() */
    Event	ev;
    char *	c;

    /* Initialize variables */
    fullname = argv[0];
    conf_file = CONFIG_FILE;
    want_to_fork = true;

    /* Parse options */
    while((opt = getopt(argc, argv, "c:n")) != -1)
    {
	switch(opt)
	{
	case 'c':
	    conf_file = strdup(optarg);
	    break;
	case 'n':
	    want_to_fork = false;
	    break;
	default:
	    exit(1);
	    break;		/* Just for style */
	}
    }

    /* General initialization */
    spider_init();

    /* Socket initialization */
    l_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (l_sock < 0) {
        syslog(LOG_ERR, "socket(2): %m");
        exit(1);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = ntohs(port);
    if (bind(l_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        syslog(LOG_ERR, "bind(2): %m");
        exit(1);
    }
    listen(l_sock, 5);
    FD_SET(l_sock, &wait_on);
    SET_WAIT_MAX(l_sock);

    while(1) {
	if (caught_sig)
	{
	    /* Fake an interrupted select */
	    sel_val = -1;
	    errno = EINTR;
	} else {
	    /* select destroys the set, so we select on a copy */
	    memcpy(&tmp_set, &wait_on, sizeof(wait_on));
	    sel_val = select(wait_on_max+1, &tmp_set, NULL, NULL, NULL);
	}
	ev = figure_out_event_type(sel_val, &tmp_set, l_sock, caught_sig);

	/* sort out where replies should go; for signal based events,
         * it's the signal number instead */
	receiver = sender = ev.fd;

	switch (ev.type) {
	case new_connect:
	    new_conn(ev.fd);
	    break;
	case client_msg:
	    msg = get_mesg(SENDER_CHAN);
            /* This takes care of sending error messages */
            if (validate_user(msg) != true) {
                break;
            }
            if (log_all_cmds && (SENDER_CONN->state == s_conn)) {
                syslog(LOG_INFO, "user %s: %s", SENDER_NAME, msg[0]);
            }
            /* Update time of last command */
            SENDER_CONN->det.usr.last_cmd_time = time(0);
	    /* Uses token_* to look at the first token only. */
            cmd = Cmd_find(msg[0]);
            switch (cmd->type) {
            case external:
                receiver = carr_findi(cmd->det.ex.mod->name, module);
                add_username(msg);
                put_mesg(cmd->det.ex.mod->chan, msg);
                break;
            case internal:
                (*cmd->det.in.fn)(msg);
                break;
            }
	    arr_del(msg);
	    msg = NULL;
            break;
        case module_msg:
	    msg = get_mesg(SENDER_CHAN);
            /* This takes care of sending error messages */
            if (validate_mod(msg) != true) {
                break;
            }
            c = copy_token(msg[0], 0);
            usr = carr_find(c, user);
            free(c);
            receiver = carr_findi(usr->name, user);
            del_username(msg);
            put_mesg(usr->chan, msg);
	    arr_del(msg);
	    msg = NULL;
            break;
	case client_death:
	    /* I don't know how I should handle this yet (even if I
             * can).  At the moment, it is handled in two places: the
             * QUIT command and verify_user (for EOFs). */
	    break;
	case module_death:
	    /* close down modules commands and connection module is
             * already dead. */
	    term_conn();
	    break;
	case server_death:
	    shutdown_all();
	    exit(0);
	    break;
        }
    }

    return 0;			/* To keep the compiler happy */
}

/*********************************************************************
 * new_conn
 *
 * Accept a new connection from the listening socket.  Fill in a dummy
 * usr structure and tell everybody that the next command must be a
 * login.
 */
Bool
new_conn(int l_sock)
{
    Bool	ok = true;
    struct sockaddr_in	client;
    struct hostent *	hp;
    char *	c;
    int 	client_len;
    
    /* Accept & Fill in a dummy structure */
    client_len = sizeof(client);
    sender = accept(l_sock, (struct sockaddr *) &client,
                      &client_len);
    if (sender < 0) {
        syslog(LOG_WARNING, "accept(2): %m");
        ok = false;
    }
    /* Whoever thought of the params for this should be shot */
    if (ok) {
        hp = gethostbyaddr((char *) &client.sin_addr.s_addr,
                           sizeof(client.sin_addr.s_addr), AF_INET);
        if (hp == NULL) {
            c = strdup(inet_ntoa(client.sin_addr));
        } else {
            c = strdup(hp->h_name);
        }
        if (SENDER_CONN != NULL) {
            syslog(LOG_WARNING, "socket %d already occupied!",
                   sender);
            close(sender);
            free(c);
            ok = false;
        }
    }
    if (ok) {
        SENDER_CONN = calloc(1, sizeof(Conn));
        SENDER_CHAN = fdopen(sender, "r+");
#ifdef SETVBUF_REVERSED
        setvbuf(SENDER_CHAN, _IOLBF, NULL, 0);
#else
        setvbuf(SENDER_CHAN, NULL, _IOLBF, 0);
#endif
        SENDER_CONN->type = user;
        SENDER_CONN->det.usr.host = c;
        FD_SET(sender, &wait_on);
        SET_WAIT_MAX(sender);
        /* Yes, this does need to be SENDER_CHAN instead of RECEIVER_CHAN */
        fputs(INITIAL_GREETING "\r\n", SENDER_CHAN);
	SENDER_CONN->state = s_init;
    }

    return ok;
}

/*********************************************************************
 * validate_user
 *
 * Validate the input that has just been received, based on where it
 * came from.  The originator is found through the use of the global
 * "sender" variable.
 */
Bool
validate_user(char ** input)
{
    char **	tmp;
    Cmdp 	cmd = NULL;
    Connp	usr = NULL;
    Bool	ok = true;

    /* Check that all the input went ok */
    if (input == NULL) {
        if (!feof(SENDER_CHAN)) {
            syslog(LOG_INFO, "Problem with user %s: %m (user logged off)",
                   SENDER_NAME);
        }
        term_conn();
        ok = false;
    }
    /* Check for minimum # tokens */
    if (ok && (num_tokens(input[0]) < 1)) {
        tmp = make_error(ERR_SYNTAX, ERR_SYNTAX_MSG);
        put_mesg(SENDER_CHAN, tmp);
        arr_del(tmp);
        ok = false;
    }
    /* Check that command actually exists */
    if (ok) {
        cmd = Cmd_find(input[0]);
	if (cmd == NULL)
	{
	    tmp = make_error(ERR_NOTCMD, ERR_NOTCMD_MSG);
	    put_mesg(SENDER_CHAN, tmp);
	    arr_del(tmp);
	    ok = false;
	}
    }
    /* If we need a login, "Make It So" */
    if (ok && (SENDER_CONN->state == s_init)) {
        if (strcasecmp(cmd->name, "LOGIN") != 0) {
            tmp = make_error(ERR_ILOGIN, ERR_ILOGIN_MSG);
            put_mesg(SENDER_CHAN, tmp);
            arr_del(tmp);
            usr = SENDER_CONN;
            term_conn();
            free(usr);
            ok = false;
        }
    }

    return ok;
}

/*********************************************************************
 * validate_mod
 *
 * Validate the input that has just been received, based on where it
 * came from.  The originator is found through the use of the global
 * "sender" variable.
 */
Bool
validate_mod(char ** input)
{
    char **	tmp = NULL;
    char *	c;
    Bool	ok = true;
    Connp	usr;

    /* Check that all went well with the input */
    if (input == NULL) {
        if (feof(SENDER_CHAN)) {
            syslog(LOG_INFO, "EOF from module %s.  Removing it",
                   SENDER_NAME);
        } else {
            syslog(LOG_INFO, "Problem (%m) with module %s.  Removing it",
                   SENDER_NAME);
        }
        term_conn();
        ok = false;
    }
    /* Check for correct # tokens */
    if (ok && (num_tokens(input[0]) < 2)) {
        tmp = make_error(ERR_SYNTAX, ERR_SYNTAX_MSG);
        put_mesg(SENDER_CHAN, tmp);
        ok = false;
    }
    /* Check for a reply code in 2nd place */
    if (ok) {
        c = find_token(input[0], 1);
        if (!isdigit(c[0]) || !isdigit(c[1]) || !isdigit(c[2])) {
            tmp = make_error(ERR_SYNTAX, ERR_SYNTAX_MSG);
            put_mesg(SENDER_CHAN, tmp);
            ok = false;
        }
    }
    /* Check that we actually have somewhere to send the message to */
    if (ok) {
        c = copy_token(input[0], 0);
        usr = carr_find(c, user);
        if (usr == NULL) {
            tmp = arr_add(tmp, make_repline(ERR_BADUSR, ERR_BADUSR_MSG));
            tmp = arr_add(tmp, c);
            tmp = arr_add(tmp, strdup(END_OF_DATA));
            ok = false;
        } else {
            free(c);
        }
    }
    return ok;
}

/*********************************************************************
 * add_username
 *
 * Adds the current username to the beginning of line 0 of msg, in
 * preparation for it to be dispatched to a module.
 */
void
add_username(char ** msg)
{
    char *	c;
    int 	i;

    if (msg != NULL) {
        i = strlen(SENDER_NAME) + 1 + strlen(msg[0]) + 1;
        c = calloc(1, i);
        c = strcpy(c, SENDER_NAME);
        c = strcat(c, " ");
        c = strcat(c, msg[0]);
        free(msg[0]);
        msg[0] = c;
    }
    
}

/*********************************************************************
 * del_username
 *
 * Deletes the current username from the beginning of line 0 of msg, in
 * preparation for it to be dispatched to a user.
 */
void
del_username(char ** msg)
{
    char *	c;
    char *	d;

    if (msg != NULL) {
        d = find_token(msg[0], 1);
        c = calloc(1, strlen(d) + 1);
        c = strcpy(c, d);
        free(msg[0]);
        msg[0] = c;
    }
    
}

/*********************************************************************
 * get_active_fd
 *
 * Find the first set file descriptor in the passed set.  The way in
 * which it does this increases the probability that a lower numbered
 * fd will get attention first.  If this is a problem for you *and*
 * you have a better way of doing this, let me know.
 */
int
get_active_fd(fd_set * set)
{
    int 	i;

    for (i = 0; i < maxfd; i++) {
        if (FD_ISSET(i, set)) {
            break;
        }
    }

    return i;
}

/*********************************************************************
 * figure_out_event_type
 *
 * Work out what just happened, based on the data given to us.
 */
Event
figure_out_event_type(int sel_val, fd_set * tmp_set, int listener, int sig)
{
    Event	what;

    /* First, check that select() exited OK */
    if (sel_val == -1)
    {
	/* We got a signal */
	if (errno == EINTR)
	{
	    switch (sig) {
	    case SIGTERM:
	    case SIGHUP:
		what.fd = sig;
		what.type = server_death;
		break;
	    case SIGCHLD:
		what.fd = sig;
		what.type = module_death;
		break;
	    default:
		/* ignore */
	    }
	} else {
	    /* file a complaint */
	    syslog(LOG_WARNING, "select error - %m");
	}
    } else {
	/* XXX How do we determine a Child Death event? */
	what.fd = get_active_fd(tmp_set);
	if (what.fd == listener)
	    what.type = new_connect;
	else {
	    switch (open_conns[what.fd]->type) {
	    case module:
		what.type = module_msg;
		break;
	    case user:
		what.type = client_msg;
		break;
	    }
	}
    }
    return what;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
