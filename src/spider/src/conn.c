/*********************************************************************
 * conn.c
 *
 * Functions relating to the handling of connections within Spider.
 *
 * Copyright 2000 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: conn.c,v 1.1 2000/01/18 07:49:01 dom Exp $";

#include <config.h>             /* autoconf */
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

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

/***********************************************************************
 * conn_init_buf: initialize buffer for connection.
 */
void
conn_init_buf(Connp c)
{
    int size = LARGE_BUF;

    if (c->buf == NULL) {
	/* create buf if we don't already have one */
	c->buf = emalloc (size);
	c->buflen = size;	/* the allocated size of buf */
	c->bufhwm = 0;		/* the point up to which data is in buf */
    }
}

/***********************************************************************
 * conn_grow_buf: extend a connection's buffer by size bytes.
 */
void
conn_grow_buf(Connp c, int size)
{
    char *newbuf;
    size_t newsize;

    if (c->buf == NULL)
	conn_init_buf(c);
    else {
	newsize = c->buflen + size;
	newbuf = erealloc (c->buf, newsize);
	c->buf = newbuf;
	c->buflen = newsize;
    }
}

/***********************************************************************
 * conn_parse_buf: translate a buffer into an array of strings.  stops
 * at the end of the first message.
 */
char **
conn_parse_buf(Connp c)
{
    return NULL;
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
