/*********************************************************************
 * mod.c
 *
 * Library file for Spider modules.  Note that the code in here is a
 * bit verbose and probably does too much error checking for
 * situations it wouldn't know how to handle anyway.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: mod.c,v 1.2 2000/01/06 21:57:57 dom Exp $";

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include "mod.h"

/* static helper functions */
static Bool validate_msg(char **msg);

/* globals */
/* This is the list of all commands that the module provides. */
static Cmdp * cmds_arr;

/*********************************************************************
 * mod_init
 *
 * Initializes a module.  ATM, only turns off buffering on stdin,
 * stdout & stderr and fires up syslog.  The name parameter is used
 * for syslog.
 */
void
mod_init(char * name)
{

    if (getenv("DEBUG")) {
	debug = true;
    }
    if (debug) {
	if (isatty(STDIN_FILENO)) {
	    fprintf(stderr, "%s: Spider module, not an interactive program\n",
		    name);
	    exit(1);
	}
    }

#ifdef SETVBUF_REVERSED
    setvbuf(stdin, _IOLBF, NULL, 0);
    setvbuf(stdout, _IOLBF, NULL, 0);
#else 
    setvbuf(stdin, NULL, _IOLBF, 0);
    setvbuf(stdout, NULL, _IOLBF, 0);
#endif
#ifndef DEBUG
    /* Shouldn't be using stderr. */
    freopen("/dev/null", "r+", stderr);
#endif /* DEBUG */

    openlog(name, LOG_PID, SPIDER_FACIL);
    debug_log("mod_init(%s) complete", name);

    /* If you need to attach a debugger to a module, best uncomment
     * these two lines. */
/*     if (debug) */
/* 	(void)sleep(10); */
}

/*********************************************************************
 * mod_reg_cmd
 *
 * Registers a command with Spider.  This is basic, needs to be made
 * so that accesses to the command are very efficient.
 *
 * The second argument is a function which returns char ** and has a
 * parameter of type "char **".  You'll see why later.
 */
Bool
mod_reg_cmd(char * name, Reply (*func)(char ** input),
            void (*erf)(char ** inp))
{
    Bool 	ok;
    Cmdp	cmd;
    int		i;
    char * 	c;

    if ((name == NULL) || (func == NULL)) {
	/* This is a kludge to finish doing command registers */
	puts(END_OF_DATA);
	ok = true;
	debug_log("mod_reg_cmd: end");
    } else {
	/* Really register the command */
	fputs(name, stdout);
	fputs("\n", stdout);
	debug_log("-> %s", name);
        c = get_line(stdin);
	if (c == NULL) {
	    syslog(LOG_WARNING, "reg_cmd(%s) error with fgets: %m",
		   name);
	    exit(1);
	}
	debug_log("<- %s", c);
	i = atoi(c);
	if (i != OK_CMD) {
	    char * d = c;
	    while (!isspace((int)*d))
                d++;
	    d++;
	    syslog(LOG_WARNING, "reg_cmd(%s) failed: %s", name, d);
	    ok = false;
	} else {
	    cmd = calloc(1, sizeof(Cmd));
	    cmd->name = name;
	    cmd->func = func;
	    cmd->errfunc = erf;
	    cmds_arr = cmd_add(cmds_arr, cmd);
	    if (cmds_arr == NULL) {
		syslog(LOG_CRIT, "reg_cmd(%s): Out of memory", name);
		exit(1);
	    }
	    ok = true;
	    debug_log("reg_cmd(%s) succeeded", name);
	}
    }
    return ok;
}

/*********************************************************************
 * mod_main_loop
 *
 * Sits tight in a loop, and reads in commands for processing.  Then
 * calls the appropriate functions to deal with them.
 */
void
mod_main_loop(void)
{
    char ** 	inp;		/* Command input */
    Reply	out;		/* Command output */
    char *	cmd;		/* Command needed */
    Cmdp	lastcmd = NULL; /* Last cmd processed */
    Cmdp *	cp;
    int		i;

    debug_log("mod_main_loop started");

    while(true) {
	inp = NULL;
	out.text = NULL;
        out.num = 0;

	/* Get the command and it's data */
	inp = get_mesg(stdin);
        if (validate_msg(inp) == false) {
            arr_del(inp);
            continue;
        }
        if (iserror(inp[0])) {
            /* Must be a an ERR_BADUSR by now. */
            if (lastcmd->errfunc != NULL) {
                (*lastcmd->errfunc)(inp);
            }
        } else {
            /* Find out what command was called for and call it */
            cmd = copy_token(inp[0], 1);
            for(cp=cmds_arr; *cp != NULL; cp++) {
                i = strcasecmp((*cp)->name, cmd);
                if(i == 0) {
                    /* Bletch.  Improvements on a postcard, please */
                    out = (*(*cp)->func)(inp);
                    lastcmd = *cp;
                    break;
                }
            }

            /* Sanity check - if we don't know about this command, then
             * throw it away. */
            if (*cp == NULL) {
                syslog(LOG_DEBUG, "strange command: %s", inp[0]);
                arr_del(inp);
                continue;
            }

            /* Return the output */
            for (i = 0; (out.text[i] != NULL) && (i < out.num); i++) {
                put_mesg(stdout, out.text[i]);
                arr_del(out.text[i]);
                /* XXX Should provide a slight pause between outputs */
                sleep(1);
            }
        }

	/* Clean up */
	arr_del(inp);
        if (out.text != NULL) {
            free(out.text);
        }
    }

    /* We don't exit here, any EOF shuts us down in validate_msg */
}

/*********************************************************************
 * validate_msg
 *
 * Check out that a message is OK.  One exception: If it's a bad
 * username (ERR_BADUSR), then just pass it on to the last called fn
 * to see what it wants to do with it.
 */
static Bool
validate_msg(char **msg)
{
    Bool 	ok = true;
    int		i;

    if (msg == NULL) {
        /* EOF */
        exit(0);
    }
    i = iserror(msg[0]);
    if ((i != 0) && (i != ERR_BADUSR)) {
        syslog(LOG_WARNING, "got error: %s", msg[0]);
        arr_del(msg);
        ok = false;
    }
    if (num_tokens(msg[0]) < 2) {
        syslog(LOG_DEBUG, "strange input: %s", msg[0]);
        arr_del(msg);
        ok = false;
    }
    return ok;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
