/*********************************************************************
 * io.c
 *
 * Input/output routines For the Spider Module Library.  Once again,
 * this is mostly nicked from Spider itself...
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[] = "@(#) $Id: mod_io.c,v 1.1 1999/03/22 07:59:06 dom Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include "mod.h"

/*********************************************************************
 * get_mesg
 *
 * Gets a whole load of lines, until a ".\n" is read.
 */
char **
get_mesg(FILE *fp)
{
    char **	tmp = NULL;
    char *	c;
    Bool 	ok = true;

    if (fp == NULL) {
        ok = false;
    }

    while (ok) {
        c = get_line(fp);
	debug_log("<- %s", c);
        if (c == NULL) {
            /* EOF */
            arr_del(tmp);
            tmp = NULL;
            ok = false;
            break;
        }
        tmp = arr_add(tmp, c);
        if (eot(c)) {
            ok = false;
        }
    }

    return tmp;
}

/*********************************************************************
 * get_line
 *
 * Allow input of arbitrarily long lines.  Returns NULL on EOF.
 */
char *
get_line(FILE * fp)
{
    int 	size;
    int 	start;
    char *	c = NULL;
    char *	input = NULL;
    Bool	ok = true;

    if (fp != NULL) {
        size = LARGE_BUF;
        start = 0;
        c = calloc(1, size);
        input = fgets(c, size, fp);
        if (input == NULL) {
            if (!feof(fp)) {
                syslog(LOG_WARNING, "get_line: %m");
            }
            ok = false;
        }
        while (ok && (ck_buf(c, size) == false)) {
            /* More input available */
            size += LARGE_BUF;
            start += LARGE_BUF;
            c = realloc(c, size);
            fgets(&c[start], LARGE_BUF, fp);
            if (input == NULL) {
                if (!feof(fp)) {
                    syslog(LOG_WARNING, "get_line: %m");
                }
                ok = false;
            }
        }
        /* Trim off any excess memory used */
        if (ok) {
            input = strdup(c);
        }
        free(c);
    }
    return input;
}

/*********************************************************************
 * put_mesg
 *
 * Puts a whole message out onto the file handle given.
 */
void
put_mesg(FILE *fp, char **msg)
{
    char **	tmp = NULL;

    if ((fp != NULL) && (msg != NULL)) {
        for (tmp = msg; *tmp != NULL; tmp++) {
            fputs(*tmp, fp);
            fputs("\n", fp);
	    debug_log("-> %s", *tmp);
        }
        /* If the line before last is not a "." */
        if (!eot(*(tmp-1))) {
            fputs(END_OF_DATA, fp);
            fputs("\n", fp);
	    debug_log("-> .");
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
