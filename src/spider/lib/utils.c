/*********************************************************************
 * utils.c
 *
 * Assorted useful functions for the Spider module library.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: utils.c,v 1.2 1999/03/18 23:37:37 dom Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <syslog.h>
#include "mod.h"

/*GLOBALS*/

Bool	debug;

/* static functions */
static char * 	skip_ws(char * p);
static char * 	skip_until_ws(char * p);

/*********************************************************************
 * ck_buf
 *
 * Checks a buffer to see that it is NULL terminated, one way or
 * another.  This function is particularly suited for post-processing
 * an fgets call.
 */
Bool
ck_buf(char * buf, int size)
{
    int 	i;
    Bool 	ok = false;

    if (buf != NULL) {
        for (i = 0, ok = false; i < size && !ok; i++) {
            /* Find a NULL & all's well */
            if (buf[i] == '\0') {
                ok = true;
                break;
            }
            /* Find a NL or CR & this is the end, too */
            if ((buf[i] == '\n') || (buf[i] == '\r')) {
                buf[i] = '\0';
                ok = true;
                break;
            }
        }
    }
    return ok;
}

/*********************************************************************
 * eot
 *
 * Sees if the string just passed is an End-Of-Transmission string.
 */
Bool
eot(char * c)
{
    Bool ok = false;
    
    if ((c[0] == END_OF_DATA[0]) && (c[1] == '\0')) {
        ok = true;
    }
    return ok;
}

/*********************************************************************
 * make_repline
 *
 * Make up a reply line.  Returns a pointer to a calloc'd string.
 */
char *
make_repline(char *name, int code, char * msg)
{
    int 	i;
    char * 	c = NULL;

    if (msg!= NULL) {
        i = strlen(name) + 1 + REPLY_CODE_LEN + strlen(msg) + 1;
        c = calloc(1, i);
        sprintf(c, "%s " REPLY_FMT, name, code, msg);
    }
    return c;
}

/*********************************************************************
 * make_unsolic
 *
 * Make up an unsolicited response.  Returns a pointer to a calloc'd
 * string.
 */
char *
make_unsolic(char * name, char *tag)
{
    int 	i;
    char * 	c = NULL;

    if (tag != NULL) {
        i = strlen(name) + 1 + REPLY_CODE_LEN + 1 + strlen(tag) + 1;
	i += strlen(OK_UNSOLIC_MSG) + 1;
        c = calloc(1, i);
        /* abuse of REPLY_FMT a bit, this */
        sprintf(c, "%s " REPLY_FMT " %s", name, OK_UNSOLIC, tag, 
		OK_UNSOLIC_MSG);
    }
    return c;
}

/*********************************************************************
 * make_error
 *
 * Creates a complete reply for an error message.
 */
char **
make_error(char * name, int code, char * msg)
{
    char **	tmp = NULL;

    if (msg != NULL) {
        tmp = arr_add(tmp, make_repline(name, code, msg));
        tmp = arr_add(tmp, strdup(END_OF_DATA));
    }
    return tmp;
}

/*********************************************************************
 * iserror
 *
 * Tests a message to see if it's an error code.
 */
int
iserror(char *s)
{
    int 	i = 0;

    if (s != NULL) {
        if (!isdigit(s[0])||!isdigit(s[1])||!isdigit(s[2])) {
            i = atoi(s);
            /* Make sure it's a response code */
            if (i > 999) {
                i = 0;
            }
        }
    }
    return i;
}

/*********************************************************************
 * skip_ws
 *
 * Skips over any whitespace.  It's not very careful about bounds
 * checking. 
 */
static char *
skip_ws(char * p)
{
    if (p != NULL) {
        while (isspace(*p))
            p++;
    }
    return p;
}

/*********************************************************************
 * skip_until_ws
 *
 * Skips over chars until whitespace is found.  It's not very careful
 * about bounds checking.
 */
static char *
skip_until_ws(char * p)
{
    if (p != NULL) {
        while (!isspace(*p) && (*p != '\0'))
            p++;
    }
    return p;
}

/*********************************************************************
 * len_token
 *
 * Counts the number of characters in the token pointed to by t.  Does
 * not include the space required for the terminating NULL
 * character!!!  Will return 0 if tok is invalid.
 */
size_t
len_token(char * tok)
{
    char *	c = tok;
    size_t 	i = 0;

    if (tok != (char *)NULL) {
	while(!isspace(*c) && (*c != '\0')) {
	    i++;
	    c++;
	}
    }
    return i;
}

/*********************************************************************
 * find_token
 *
 * Returns a pointer to the beginning of the nth ws-delimited token in
 * buf.  n is zero-based.  Returns NULL if n is invalid.
 */
char *
find_token(char * buf, int n)
{
    char *	c = buf;
    int 	i;

    if (buf != NULL) {
        if (num_tokens(buf) > n)
        {
            /* Avoid preceding ws */
            if (isspace(*c))
                c = skip_ws(c);
	
            for(i = 0; i < n; ++i)
            {
                c = skip_until_ws(c);
                c = skip_ws(c);
            }
        } else {
            c = NULL;
        }
    }
    return c;
}

/*********************************************************************
 * copy_token
 *
 * Returns a *copy* of the nth ws-delimited token in buf.  n is
 * zero-based, ie: copy_token(buf, 0) will retrieve the first token.
 * The return value *is* NULL terminated.  The return value is NULL,
 * if n is greater than the number of tokens in the buffer.
 */
char *
copy_token(char * buf, int n)
{
    char *	c;
    char *	word = NULL;
    size_t 	len;

    if (buf != NULL) {
        if (num_tokens(buf) > n) /* num_tokens retval is +1 */
        {
            c = find_token(buf, n);
            len = len_token(c);
            word = calloc(1, len+1);
            strncpy(word, c, len); /* Already NULL-terminated */
        } else {
            word = NULL;
        }
    }
    return word;
}

/*********************************************************************
 * num_tokens
 *
 * Returns the number of tokens in buf.  The return value from here is
 * one based, not zero based, like the other fns.
 */
int
num_tokens(char * buf)
{
    char * 	c;
    int 	i = 0;

    if (buf != NULL) {
        c = buf;
        if (isspace(*c))
            c = skip_ws(c);
        for(i = 0; *c != '\0'; i++)
        {
            c = skip_until_ws(c);
            c = skip_ws(c);
        }
    }
    return i;
}

/*********************************************************************
 * cmp_token
 *
 * Compares the token in buf with the string given.  Returns true if
 * they are the same, else false.
 *
 * This function is case-independent.
 */
Bool
cmp_token(char * buf, int n, char * s)
{
    char * 	c;
    Bool	ok = false;
    int 	i;

    if ((buf != NULL) && (s != NULL)) {
	c = find_token(buf, n);
	/* Only perform a comparison if both strings are of equal length */
	if ((i = len_token(c)) == strlen(s)) {
	    /* Because otherwise strncmp might find a substring of the
	     * other. */
	    if (strncasecmp(c, s, i) == 0) {
		ok = true;
	    }
	}
    }
    return ok;
}

/*********************************************************************
 * debug_log
 *
 * Log a message if in debugging mode.
 */
void
debug_log(char *msg, ...)
{
    va_list args;

    if (debug) {
	va_start (args, msg);
	(void)vsyslog (LOG_DEBUG, msg, args);
	va_end (args);
    }
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
