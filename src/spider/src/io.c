/*********************************************************************
 * io.c
 *
 * Input/output routines.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: io.c,v 1.10 2000/01/16 23:04:22 dom Exp $";

#include <config.h>             /* autoconf */

#include <sys/types.h>
#include <errno.h>
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
#include <unistd.h>

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
        if (c == NULL) {
            /* EOF */
            arr_del(tmp);
            tmp = NULL;
            ok = false;
            break;
        }
	log (LOG_DEBUG, "[%d] <- %s", fileno(fp), c);
        tmp = arr_add(tmp, c);
        if (eot(c)) {
            ok = false;
        }
    }

    /* Solaris requires this. */
    fflush(fp);

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
            log (LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
		 __FILE__);
            exit(1);
        }
        input = fgets(c, size, fp);
        if (input == NULL) {
            if (!feof(fp)) {
                log (LOG_WARNING, "get_line: %m");
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
                    log (LOG_WARNING, "get_line: %m");
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
 * input_data
 *
 * Pulls in whatever comes next from an fd.  Stores it in the
 * connection's buffer.  This may block.  Only call if we are sure
 * that Connection's fd is readable.
 */
Bool
input_data(Connp c)
{
    Bool	ok;
    char *	cp;
    ssize_t	bytes;

    /* If we don't already have a buffer, create one. */
    if (c->buf == NULL) {
	c->buf = malloc(LARGE_BUF);
	if (c->buf == NULL) {
	    log (LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
		 __FILE__);
            exit(1);
        }
	c->buflen = LARGE_BUF;
	c->bufhwm = 0;
    }

    /* Check that we actually have some buffer left... */
    if (c->bufhwm == c->buflen) {
	cp = realloc (c->buf, c->buflen + LARGE_BUF);
	if (cp == NULL){
	    log (LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
		 __FILE__);
            exit(1);
        }
	c->buf = cp;
    }
    
    /* Read into however many bytes we have left in our buffer */
    bytes = read(c->fd, (void *)(c->buf + c->bufhwm),
		 (size_t)(c->buflen - c->bufhwm));

    /* Update the connection if successful */
    if (bytes < 0) {
	ok = false;
    } else {
	ok = true;
	c->bufhwm += bytes;
    }

    return ok;
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
	    log (LOG_DEBUG, "[%d] -> %s", fileno(fp), *tmp);
        }
        /* If the line before last is not a "." */
        if (!eot(*(tmp-1))) {
            fputs(END_OF_DATA, fp);
            fputs(eol, fp);
	    log (LOG_DEBUG, "[%d] -> .", fileno(fp));
        }
    }
}

/***********************************************************************
 * single_read_conn: read in data from conn into it's buffer.  once
 * only.
 */
int
conn_single_read(Connp c)
{
    int bytes;
    size_t bufspace;

    if (c->buf == NULL)
	conn_init_buf(c);

    bufspace = c->buflen - c->bufhwm;
    bytes = read(c->fd, c->buf, bufspace);
    if (bytes >= 0)
	c->bufhwm += bytes;
    else if (bytes == -1 && errno == EINTR) {
/* 	apres_sig(); */
	/* restart the read */
	bytes = conn_single_read(c);
    }

    return bytes;
}

/***********************************************************************
 * read_conn: read in data from fd into a buffer.
 * XXX must be careful to avoid blocking at all costs.  */
int
conn_read(Connp c)
{
    int bytes, totbytes = 0;
    size_t bufspace;

    do {
	bufspace = c->buflen - c->bufhwm;
	if (bufspace == 0) {
	    conn_grow_buf(c, LARGE_BUF);
	    bufspace += LARGE_BUF;
	}
	bytes = conn_single_read(c);
	if (bytes == 0)
	    return 0;		/* EOF */
	totbytes += bytes;
    } while (bytes == bufspace);

    return totbytes;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
