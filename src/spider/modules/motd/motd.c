/*********************************************************************
 * motd.c
 *
 * A module for the Spider BBS system.  Provides a Message-Of-The-Day
 * command.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: motd.c,v 1.1 1999/03/11 15:39:49 dom Exp $";

#include <sys/stat.h>
#include <ctype.h>
#ifdef HAVE_LIMITS_H
#    include <limits.h>
#else
#    ifdef __STDC__
#        define ULONG_MAX 4294967295U
#    else
#        define ULONG_MAX 4294967295
#    endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include <mod.h>

#define MOTD_FILE "motd.txt"

Reply 	cmd_motd(char ** input);
void	read_motd(void);

/* globals */
char ** msg;			/* MOTD */

/*********************************************************************
 * main
 *
 * Everything hangs on here.  No arguments are used.  In fact, it's
 * illegal to use them. hahaha.
 */
int
main(int argc, char **argv)
{
    /* Do protocol to register our keyword. */
    mod_init("motd");
    mod_reg_cmd("MOTD", cmd_motd, NULL);
    mod_reg_cmd(NULL, NULL, NULL);

    /* And now sit in a loop */
    mod_main_loop();

    exit(0);
    return 0;
}

/*********************************************************************
 * cmd_motd
 *
 * Return the message of the day to the user.
 */
Reply
cmd_motd(char ** input)
{
    struct stat statbuf;
    static time_t	old_time = 0;
    char * 	name;
    Reply	rep = {NULL, 0};
    char ** 	tmp = NULL;
    char * 	c;
    char * 	replytxt = "Here's the news:";

    /* Check that the MOTD file hasn't changed */
    if (stat(MOTD_FILE, &statbuf) < 0) {
        /* Stop error from re-occuring */
        /* XXX - change this if your systems time_t isn't a long */
	old_time = ULONG_MAX;
	read_motd();
    } else {
        if (statbuf.st_mtime > old_time) {
            old_time = statbuf.st_mtime;
            read_motd();
        }
    }

    /* Must make sure that rep is init'd to NULL's first - see above */
    rep = new_reply(rep);

    name = copy_token(input[0], 0);
    c = make_repline(name, OK_CMD, replytxt);
    CUR_REP(rep) = arr_add(CUR_REP(rep), c);
    free(name);

    /* Kludge alert! */
    if (msg != NULL)
	for(tmp = msg; *tmp != NULL; ++tmp)
	    CUR_REP(rep) = arr_add(CUR_REP(rep), strdup(*tmp));
    CUR_REP(rep) = arr_add(CUR_REP(rep), strdup(END_OF_DATA));

    return rep;
}

/*********************************************************************
 * read_motd
 *
 * Read the message-of-the-day file into memory.
 */
void
read_motd(void)
{
    FILE *	motd;
    char *	c;
    int		i;

    arr_del(msg);
    msg = NULL;

    /* Read the message into memory. */
    motd = fopen(MOTD_FILE, "r");
    if (motd) {
	while((c = get_line(motd)) != NULL) {
	    /* Cater for finding a '.' on a line of it's own */
	    if (eot(c)) {
		i = strlen(c);
		memmove(c+1, c, i+1);
	    }
	    msg = arr_add(msg, c);
	}
	fclose(motd);
    } else {
	/* motd file doesn't exist. */
	msg = arr_add(msg, NULL);
    }
}

/*
 * Local Variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
