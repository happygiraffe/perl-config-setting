/*********************************************************************
 * test.c
 *
 * Example module for Spider.
 *
 * Copyright 1996 Dominic Mitchell
 */

static const char rcsid[]="@(#) $Id: test.c,v 1.1 1999/03/11 15:39:49 dom Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <mod.h>

/* Local command functions */
Reply	cmd_foo(char ** input);
Reply	cmd_bar(char ** input);
Reply	cmd_date(char ** input);
Reply	cmd_qotd(char ** input);

int
main(int argc, char ** argv)
{
#ifndef DEBUG
    if ((argc > 1) || (isatty(STDIN_FILENO))) {
	perror("$s: spider module, not an interactive program");
	exit(1);
    }
#endif

    /* Initialize our environment */
    mod_init("test");

    /* Register commands */
    mod_reg_cmd("FOO", cmd_foo, NULL);
    mod_reg_cmd("BAR", cmd_bar, NULL);
    mod_reg_cmd("DATE", cmd_date, NULL);
    mod_reg_cmd("QOTD", cmd_qotd, NULL);
    /* Critical to finish entering commands and start processing them */
    mod_reg_cmd(NULL, NULL, NULL);

    mod_main_loop();

    exit(0);
    return 0;
}

/*********************************************************************
 * cmd_foo
 *
 * Process a "foo" command.
 */
Reply
cmd_foo(char ** input)
{
    char *	name;
    Reply	rep = {NULL, 0};
    char * 	reply = "BAR, my little sheep";
    char *	c;

    /* Initialize the structure properly */
    rep = new_reply(rep);

    /* Prepare a reply */
    name = copy_token(input[0], 0);
    c = make_repline(name, OK_CMD, OK_CMD_MSG);
    CUR_REP(rep) = arr_add(CUR_REP(rep), c);
    free(name);
    CUR_REP(rep) = arr_add(CUR_REP(rep), reply);
    /* Finalize the result */
    CUR_REP(rep) = arr_add(CUR_REP(rep), strdup(END_OF_DATA));

    return rep;
}

/*********************************************************************
 * cmd_bar
 *
 * Process a "bar" command.
 */
Reply
cmd_bar(char ** input)
{
    char *	name;
    Reply	rep = {NULL, 0};
    char * 	reply = "FOO, O wondrous moggie";
    char *	c;

    /* Initialize the structure properly */
    rep = new_reply(rep);

    /* Prepare a reply */
    name = copy_token(input[0], 0);
    c = make_repline(name, OK_CMD, OK_CMD_MSG);
    free(name);
    CUR_REP(rep) = arr_add(CUR_REP(rep), c);
    CUR_REP(rep) = arr_add(CUR_REP(rep), reply);
    /* Finalize the result */
    CUR_REP(rep) = arr_add(CUR_REP(rep), strdup(END_OF_DATA));

    return rep;
}

/*********************************************************************
 * cmd_date
 *
 * Process a "date" command.  And return the BBS (== systems) notion
 * of the current date & time.
 */
Reply
cmd_date(char ** input)
{
    char *	name;
    Reply	rep = {NULL, 0};
    time_t	now;
    char *	c;

    /* Initialize the structure properly */
    rep = new_reply(rep);

    /* Prepare a reply */
    name = copy_token(input[0], 0);
    c = make_repline(name, OK_CMD, OK_CMD_MSG);
    free(name);
    CUR_REP(rep) = arr_add(CUR_REP(rep), c);

    /* Get the date. */
    time(&now);
    c = strdup(ctime(&now));
    /* Remove trailing '\n' */
    ck_buf(c, strlen(c));
    CUR_REP(rep) = arr_add(CUR_REP(rep), c);

    /* Finalize the result */
    CUR_REP(rep) = arr_add(CUR_REP(rep), strdup(END_OF_DATA));

    return rep;
}

/*********************************************************************
 * cmd_qotd
 *
 * Return a fortune cookie.  Just does a popen to the standard
 * 'fortune' program.
 */
#define FORTUNE_PROG "/usr/games/fortune -s"
Reply
cmd_qotd(char ** input)
{
    char *	name;
    Reply	rep = {NULL, 0};
    FILE *	fortune;
    char *	c;

    /* Initialize the structure properly */
    rep = new_reply(rep);

    /* Prepare a reply */
    name = copy_token(input[0], 0);
    c = make_repline(name, OK_CMD, OK_CMD_MSG);
    free(name);
    CUR_REP(rep) = arr_add(CUR_REP(rep), c);

    /* Get a cookie */
    fortune = popen(FORTUNE_PROG, "r");
    c = get_line(fortune);
    while (c != NULL) {
        CUR_REP(rep) = arr_add(CUR_REP(rep), c);
        c = get_line(fortune);
    }
    pclose(fortune);

    /* Finalize the result */
    CUR_REP(rep) = arr_add(CUR_REP(rep), strdup(END_OF_DATA));

    return rep;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
