/*********************************************************************
 * io.c
 *
 * Input/output routines.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: io.c,v 1.3 1999/03/18 23:54:04 dom Exp $";

#include <config.h>             /* autoconf */
#include <stdio.h>
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
#include "spider.h"

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
        c = malloc(size);
        if (c == NULL) {
            syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
                   __FILE__);
            exit(1);
        }
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
    char *	eol = "\n";

    if (RECEIVER_CONN->type == user) {
        eol = "\r\n";
    } else {
        eol = "\n";
    }

    if ((fp != NULL) && (msg != NULL)) {
        for (tmp = msg; *tmp != NULL; tmp++) {
            fputs(*tmp, fp);
            fputs(eol, fp);
	    debug_log("-> %s", *tmp);
        }
        /* If the line before last is not a "." */
        if (!eot(*(tmp-1))) {
            fputs(END_OF_DATA, fp);
            fputs(eol, fp);
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
