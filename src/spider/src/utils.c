/*********************************************************************
 * utils.c
 *
 * Assorted useful functions.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: utils.c,v 1.8 2000/01/16 23:47:43 dom Exp $";

#include <config.h>
#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>             /* malloc */
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
#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>             /* sysconf */

#include "spider.h"

/* PROTOTYPES */

static void vlog (int priority, char *msg, va_list args);

/*********************************************************************
 * mk_a_dir
 *
 * Make a directory, in a portable fashion.
 */
void
mk_a_dir(char * name, int mode)
{
#ifdef HAVE_MKDIR
    /* shouldn't be necessary, but is. */
    int		mkdir (char* name, mode_t mode);
#else
    char *	foo;
#endif 

#ifdef HAVE_MKDIR
    mkdir(name, mode);
#else
    foo = emalloc (LARGE_BUF);
    sprintf (foo, "mkdir %s", name);
    system(foo);
    free(foo);
#endif
}

#ifndef HAVE_STRDUP
/*********************************************************************
 * strdup
 *
 * Create a copy of a string.  Return NULL if no memory.
 */
char *
strdup(char * s)
{
    char * new;

    new = malloc (strlen(s) + 1);
    if (new != NULL)
	    strcpy(new, s);
    return new;
}
#endif

/*********************************************************************
 * ck_buf
 *
 * Checks a buffer to see that it is NULL terminated, one way or
 * another.  This function is particularly suited for post-processing
 * a fgets call.
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
 * basename
 *
 * Works out the name from argv[0] (ie: removes path)
 */
char *
basename(char * argv0)
{
    char * 	first = NULL;

    if (argv0 != NULL) {
	first = strrchr(argv0, '/');
	if (first == NULL)
	    first = argv0;
	else
	    ++first;
    }
    return first;
}

/*********************************************************************
 * kill_comment
 *
 * Removes anything after the comment char ('#'), by replacing it with
 * a '\0'.
 */
void
kill_comment(char * buf)
{
    char * c;

    if (buf != NULL) {
	c = strchr(buf, '#');
	if (c != NULL)
	    *c = '\0';
    }
}

/*********************************************************************
 * valid_chars
 *
 * Checks a string to see that it contains only valid characters, ie:
 * alphanumeric.  Anything else might confuse some poor client.
 */
Bool
valid_chars(char * s)
{
    char *	c;
    Bool	ok = true;

    if (s == NULL) {
	ok = false;
    }
    for(c = s; (*c != '\0') && ok; c++)
    {
	if(!isalnum((int)*c))
	    ok = false;
    }
    return ok;
}

/*********************************************************************
 * get_max_fds
 *
 * Returns the maximum available number of file descriptors on this
 * system.
 */
int
get_max_fds(void)
{
    int 	maxfd;
    
    maxfd = sysconf(_SC_OPEN_MAX);
    if (maxfd == -1)
	maxfd = MAX_OPEN_GUESS;	/* A wild guess, probably
				 * wrong */
    return maxfd;
}

/*********************************************************************
 * make_repline
 *
 * Make up a reply line.  Returns a pointer to a malloc'd  and zeroed 
 * string.
 */
char *
make_repline(int code, char * msg)
{
    int 	i;
    char * 	c = NULL;

    if (msg!= NULL) {
        i = REPLY_CODE_LEN + strlen(msg) + 1;
        c = emalloc ((size_t)i);
        sprintf(c, REPLY_FMT, code, msg);
    }
    return c;
}

/*********************************************************************
 * make_error
 *
 * Creates a complete reply for an error message.
 */
char **
make_error(int code, char * msg)
{
    char **	tmp = NULL;

    if (msg != NULL) {
        tmp = arr_add(tmp, make_repline(code, msg));
        tmp = arr_add(tmp, estrdup (END_OF_DATA));
    }
    return tmp;
}

/*********************************************************************
 * up_str
 *
 * Convert a string to uppercase.
 */
void
up_str(char * p)
{
    if (p != NULL) {
	for (; *p != '\0'; p++) {
	    *p = toupper(*p);
	}
    }
}

/*********************************************************************
 * down_str
 *
 * Convert a string to lowercase.
 */
void
down_str(char * p)
{
    if (p != NULL) {
	for (; *p != '\0'; p++) {
	    *p = tolower(*p);
	}
    }
}

/*********************************************************************
 * skip_ws
 *
 * Skips over any whitespace.  It's not very careful about bounds
 * checking. 
 */
char *
skip_ws(char * p)
{
    if (p != NULL) {
	while (isspace((int)*p)) {
	    p++;
	}
    }
    return p;
}

/*********************************************************************
 * skip_until_ws
 *
 * Skips over chars until whitespace is found.  It's not very careful
 * about bounds checking.
 */
char *
skip_until_ws(char * p)
{
    if (p != NULL) {
	while (!isspace((int)*p) && (*p != '\0')) {
	    p++;
	}
    }
    return p;
}

/*********************************************************************
 * len_token
 *
 * Counts the number of characters in the token pointed to by t.  Does
 * not include the space required for the terminating NULL
 * character!!!
 */
size_t
len_token(char * tok)
{
    char * 	c = tok;
    size_t 	i = 0;

    if (tok != NULL) {
	c = tok;
	while(!isspace((int)*c) && (*c != '\0')) {
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
	if (num_tokens(buf) > n) {
	    /* Avoid preceding ws */
	    if (isspace((int)*c)) {
		c = skip_ws(c);
	    }
	
	    for(i = 0; i < n; i++) {
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
	if (num_tokens(buf) > n) { /* num_tokens retval is +1 */
	    c = find_token(buf, n);
	    len = len_token(c);
	    word = emalloc ((size_t)len+1);
	    strncpy(word, c, len);
	    word[len]='\0';
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
	if (isspace((int)*c))
	    c = skip_ws(c);
	for(i = 0; *c != '\0'; i++) {
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

/***********************************************************************
 * log: log a message with syslog, or maybe stdout.
 */
void
log (int priority, char *msg, ...)
{
    va_list args;

    va_start (args, msg);
    vlog (priority, msg, args);
    va_end (args);
    return;
}

/***********************************************************************
 * vlog: log a message with syslog, or maybe stdout.  stdarg version.
 */
static void
vlog (int priority, char *msg, va_list args)
{
    int		saved_errno = errno;
	
    if (am_daemon) {
	(void) vsyslog (priority, msg, args);
    } else if (priority != LOG_DEBUG || debug) {
	vfprintf (stderr, msg, args);
	if (strstr (msg, "%m"))
	    fprintf (stderr, " (errno=%d)", saved_errno);
	fprintf (stderr, "\n");
    }
    return;
}

/***********************************************************************
 * err: log an error and exit.
 */
void
err (char *msg, ...)
{
    va_list args;

    va_start (args, msg);
    vlog (LOG_ERR, msg, args);
    exit (1);
    /* NOTREACHED */
    va_end (args);
}

/***********************************************************************
 * emalloc: malloc, and warn if error.  from TPOP.
 */
void *
emalloc (size_t n)
{
    void *p;

    p = malloc(n);
    if (p == NULL)
	err ("malloc of %u bytes failed: %m", n);
    return p;
}

/***********************************************************************
 * erealloc: realloc and warn if error.
 */
void *
erealloc (void *oldp, size_t n)
{
    void *newp;

    newp = realloc (oldp, n);
    if (newp == NULL)
	err ("realloc of %u bytes failed: %m", n);
    return newp;
}

/***********************************************************************
 * estrdup: strdup, and warn of error. from TPOP.
 */
char *
estrdup (char *s)
{
    char *t;

    t = (char *) malloc (strlen(s)+1);
    if (t == NULL)
	err ("estrdup(\"%.20s\") failed: %m", s);
    strcpy (t, s);
    return t;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */