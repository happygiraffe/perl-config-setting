/*********************************************************************
 * ds.c
 *
 * Functions to handle all the data structures that we need.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: ds.c,v 1.5 2000/01/16 12:54:43 dom Exp $";

#include <config.h>             /* autoconf */
#include <sys/types.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
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

/* Globals */
Connp * open_conns;		/* Array of connections */

/* Static Globals - hopefully, these should not need to be exposed */
static Hash	Cmd_hash;
static Hash	Usr_hash;

/*********************************************************************
 * carr_find
 *
 * Find something in the connections array.  Uses the global variable
 * maxfd, for speed (instead of calling get_max_fds each time).
 */
Connp
carr_find(char * name, Conntype type)
{
    Connp tmp = NULL;
    int i;

    if (name != NULL) {
	for (i=0, tmp=NULL; i <maxfd; i++) {
	    if (open_conns[i] != NULL) {
		if ((open_conns[i]->type == type) &&
		    (open_conns[i]->state == s_conn)) {
		    if (strcasecmp(open_conns[i]->name, name) == 0) {
			tmp = open_conns[i];
			break;
		    }
		}
	    }
	}
    }
    return tmp;
}

/*********************************************************************
 * carr_findi
 *
 * Find something in the connections array.  Uses the global variable
 * maxfd, for speed (instead of calling get_max_fds each time).
 * Returns the index of it in the connections array.
 */
int
carr_findi(char * name, Conntype type)
{
    int i = 0;

    if (name != NULL) {
	for (i=0; i <maxfd; i++) {
	    if (open_conns[i] != NULL) {
		if ((open_conns[i]->type == type) &&
		    (open_conns[i]->state == s_conn)) {
		    if (strcasecmp(open_conns[i]->name, name) == 0) {
			break;
		    }
		}
	    }
	}
    }
    return i;
}

/*********************************************************************
 * carr_del
 *
 * Zero out an entry in the connection array.  Don't touch what it
 * points at.
 *
 * XXX Should this do more work?  It's pretty obvious what needs
 * doing if the connection needs deleting...
 */
Bool
carr_del(char * name, Conntype type)
{
    Bool	ok = false;
    int 	i;

    if (name != NULL) {
	for (i=0; i <maxfd; i++) {
	    if (open_conns[i] != NULL) {
		if ((open_conns[i]->type == type) &&
		    (open_conns[i]->state == s_conn)) {
		    if (strcasecmp(open_conns[i]->name, name) == 0) {
			ok = true;
			open_conns[i] = NULL;
			break;
		    }
		}
	    }
	}
    }
    return ok;
}

/*********************************************************************
 * arr_count
 *
 * Returns the number of elements in a (char *) array.
 */
int
arr_count(char ** arr)
{
    int i = 0;

    if (arr != NULL)
	while(arr[i] != (char *)NULL)
	    i++;
    return i;
}

/*********************************************************************
 * arr_add
 *
 * Adds a string (pointer to char) onto the end an array of
 * characters, ensuring that the final element remains NULL.  Uses the
 * original string, not a copy, so the original must be dynamically
 * allocated.  Returns a new pointer to the array.  Take heed!
 *
 * eg: argv = arr_add(argv, my_arg);
 *     if (argv == NULL)
 *         perror ("arr_add");
 */
char **
arr_add(char ** arr, char * str)
{
    char ** tmp_arr = NULL;
    int i;

    if (str != NULL) {
	if (arr == NULL) {
	    tmp_arr = malloc(2 * sizeof(char *));
	    if (tmp_arr == NULL) {
		syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
		       __FILE__);
		exit(1);
	    }
	    tmp_arr[0] = str;
	    tmp_arr[1] = NULL;
	} else {
	    i = arr_count(arr);
	    tmp_arr = realloc(arr, (i+2)*sizeof(char *));
	    tmp_arr[i] = str;
	    tmp_arr[i+1] = NULL;
	}
    }

    return tmp_arr;
}

/*********************************************************************
 * arr_del
 *
 * Free up the memory taken by an array of strings.
 */
void
arr_del(char ** arr)
{
    char **	c;

    if (arr != NULL) {
	for(c = arr; *c != NULL; c++) {
	    free(*c);
	    *c = NULL;
	}
    }
    free(arr);
}

/*********************************************************************
 * Hashing Stuff.
 *
 * This is stolen from K&R, hopefully, it is general enough to be
 * useful.
 *
 * It is an open-addressing chained hash.
 *********************************************************************
 */

/*********************************************************************
 * hash_init
 *
 * Initializes a hash.  If indep is true, then keys in the hash are
 * case-insensitive.  Case independence is achieved by storing all
 * the keys as uppercase.  It's a bit of a hack, but it works...
 */
void
hash_init(Hashp h, Bool indep)
{
    int 	i;

    if (h != NULL) 
    {
	h->ignore_case = indep;
	for (i = 0; i < HASH_SIZE; i++)
	{
	    h->table[i] = NULL;
	}
    }
}

/*********************************************************************
 * hash
 *
 * Makes a hash of the string given to it.
 */
unsigned
hash(char * s, Hashp h)
{
    unsigned	hashval;

    if (s != NULL) 
    {
	if (h->ignore_case) 
	{
            /* can't use isspace(3) for some reason... */
	    for(hashval = 0; (*s != ' ') && (*s != '\0') ; s++) 
	    {
		hashval = toupper(*s) + 31 * hashval;
	    }
	} else {
	    for(hashval = 0; (*s != ' ') && (*s != '\0') ; s++) 
	    {
		hashval = *s + 31 * hashval;
	    }
	}
    }
    return hashval % HASH_SIZE;
}

/*********************************************************************
 * hash_lookup
 *
 * Find the node containing s in the hash h.  Return NULL if not
 * present.  The return data need not be cast to the correct type, as
 * it is already a void*.
 */
void *
hash_lookup(char *s, Hashp h)
{
    Nodep	np = NULL;
    char *	c;

    if (s != NULL) {
        c = skip_ws(s);
        for(np = h->table[hash(c, h)]; np != NULL; np = np->next) {
            if (cmp_token(c, 0, np->name)) {
                break;
            }
        }
    }
    /*  np may be NULL, so protect against that */
    return np ? np->data : np;
}

/*********************************************************************
 * hash_insert
 *
 * Inserts a new piece of data into the hash table, using chaining if
 * necessary.  Returns false if name already exists in the table.
 */
Bool
hash_insert(char * name, void * data, Hashp h)
{
    Nodep	np;
    unsigned	hashval;
    Bool	ok = false;

    if (name != NULL) {
        np = hash_lookup(name, h);
        if (np == NULL)
        {
            /* This entry doesn't already exist */
            np = malloc(sizeof(Node));
	    if (np == NULL) {
		syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
		       __FILE__);
		exit(1);
	    }
            np->name = copy_token(name, 0);
	    if (h->ignore_case)
	    {
		up_str(np->name);
	    }
            np->data = data;
            hashval = hash(name, h);
            /* save the existing entry om the table */
            np->next = h->table[hashval];
            if (np->next != NULL)
            {
                /* if there was an existing entry, update it's prev */
                np->next->prev = np;
            } else {
                /* if we're first, make sure we're the only one */
                np->prev = NULL;
            }
            h->table[hashval] = np;
            ok = true;
        } 
    }
    return ok;
}

/*********************************************************************
 * hash_delete
 *
 * Delete an entry from the hash table.  Returns the data stored, if
 * it exists, else NULL.
 */
void *
hash_delete(char * name, Hashp h)
{
    Nodep	np;
    unsigned	hashval;
    void *	old_data = NULL;

    if (name != NULL) {
        hashval = hash(name, h);
        for (np = h->table[hashval]; np != NULL; np = np->next) {
            if (strcasecmp(name, np->name) == 0) {
                /* zap it */
                old_data = np->data;
                if (np->prev != NULL) {
                    np->next->prev = np->prev;
                } else {
                    h->table[hashval] = NULL;
                }
                if (np->next != NULL) {
                    np->prev->next = np->next;
                }
                free(np->name);
                free(np);
                break;
            }
        }
    }
    return old_data;
}

/*********************************************************************
 * hash_stat
 *
 * Send out stats to syslog on how full a hash table is.
 */
void
hash_stat(char * name, Hashp h)
{
    int		total;          /* total filled entries in the table */
    int		tot_in_chains;
    Nodep	np;
    int		i;

    if ((h != NULL) && (name != NULL))
    {
        total = 0;
        tot_in_chains = 0;
        for (i = 0; i < HASH_SIZE; i++)
        {
            np = h->table[i]; 
            if (np != NULL)
            {
                ++total;
                for(; np != NULL; np = np->next)
                {
                    ++tot_in_chains;
                }
            }
        }

        syslog(LOG_INFO, "hash %s usage is %d/%d avg chain length is %f", 
	       name, total, HASH_SIZE, (float) tot_in_chains / total);
    }
}

/*********************************************************************
 * hash_visit
 *
 * Visits each entry in the hash, and calls a function on it.
 * 
 * Should the function called have the name passed to it as an extra
 * parameter? 
 */
void
hash_visit(void (* func)(void * data), Hashp h)
{
    int		i;
    Nodep	np;
    Nodep	tmp_np;

    for (i = 0; i < HASH_SIZE; i++) 
    {
	np = h->table[i];
	if (np != NULL) 
	{
	    while (np != NULL)
	    {
		/* record next value first, so that deleting the
		 * node inside func will have no effect. */
		tmp_np = np->next;
		(*func)(np->data);
                np = tmp_np;
	    }
	}
    }
}

/*********************************************************************
 * Here lie all the Cmd_* functions, rewritten to use the hash
 * facilities.
 *********************************************************************
 */

/*********************************************************************
 * Cmd_init
 * 
 * Initialize the command hash.
 */
void 
Cmd_init(void)
{
    hash_init(&Cmd_hash, true);
}

/*********************************************************************
 * Cmd_find
 * 
 * Find a command in tha hash & return it.
 */
Cmdp
Cmd_find(char * name)
{
    return (Cmdp)hash_lookup(name, &Cmd_hash);
}

/*********************************************************************
 * Cmd_add
 * 
 * Insert a new command into the hash of them.  It is safe to make
 * name a pointer into data, becuase a copy will be taken anyway.
 */
Bool
Cmd_add(char *name, Cmdp data)
{
    return hash_insert(name, (void *)data, &Cmd_hash);
}

/*********************************************************************
 * Cmd_delete
 * 
 * Delete an entry from the command hash.
 */
void
Cmd_delete(char * name)
{
    Cmdp	c;

    c = (Cmdp)hash_lookup(name, &Cmd_hash);
    if ((c != NULL) && (c->type = external))
    {
	c = (Cmdp)hash_delete(name, &Cmd_hash);
	/* Don't bother with the associatd module; assume it's been
	 * taken care of */
	free(c->name);
	free(c);
    }
}

/*********************************************************************
 * Cmd_stat
 * 
 * Print some stats about the hash table to syslog. 
 */
void 
Cmd_stat(void)
{
    hash_stat("Cmd", &Cmd_hash);
}

/*********************************************************************
 * Cmd_visit
 * 
 * Run a function over every command in the hash.
 */
void
Cmd_visit(void (*func)(void *))
{
    hash_visit(func, &Cmd_hash);
}

/*********************************************************************
 * Here lie all the Usr_* functions, rewritten to use the hash
 * facilities.
 *********************************************************************
 */

/*********************************************************************
 * Usr_init
 * 
 * Initialize the connection hash.
 */
void 
Usr_init(void)
{
    hash_init(&Usr_hash, true);
}

/*********************************************************************
 * Usr_find
 * 
 * Find a connection in tha hash & return it.
 */
Connp
Usr_find(char * name)
{
    return (Connp)hash_lookup(name, &Usr_hash);
}

/*********************************************************************
 * Usr_add
 * 
 * Insert a new connection into the hash of them.  It is safe to make
 * name a pointer into data, becuase a copy will be taken anyway.
 */
Bool
Usr_add(char *name, Connp data)
{
    return hash_insert(name, (void *)data, &Usr_hash);
}

/*
 * There is no Usr_del function; users are not deleted in Spider yet.
 */

/*********************************************************************
 * Usr_stat
 * 
 * Print some stats about the hash table to syslog. 
 */
void 
Usr_stat(void)
{
    hash_stat("Usr", &Usr_hash);
}

/*********************************************************************
 * Usr_visit
 * 
 * Run a function over every connection in the hash.  I don't see
 * that this will come in handy, but the facility is there...
 */
void
Usr_visit(void (*func)(void *))
{
    hash_visit(func, &Usr_hash);
}

/*
 * Functions pertaining to text buffers.
 */

/***********************************************************************
 * whole_msg: return true if the buffer contains at least one whole
 * protocol message.
 */
Bool
whole_msg(Connp c)
{
    char *ch;

    /*
     * search through the data in the buffer looking for "\n.[\r\n]"
     * in regex speak.  that's why we stop 3 chars before bufhwm.
     */
    /* XXX should this be <= instead? */
    for (ch = c->buf; ch < (c->buf + (c->bufhwm - 3)); ch++)
	if ( ch[0] == '\n' &&
	     ch[1] == '.'  &&
	    (ch[2] == '\r' || ch[2] == '\n'))
	    return true;

    return false;
}

/*
 * Functions pertaining to connection structures.
 */

/***********************************************************************
 * conn_init_buf: initialize buffer for connection.
 */
void
conn_init_buf(Connp c)
{
    int size = LARGE_BUF;

    if (c->buf == NULL) {
	/* create buf if we don't already have one */
	c->buf = malloc(size);
	if (c->buf == NULL) {
	    syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
                   __FILE__);
            exit(1);
	}
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
	newbuf = realloc(c->buf, newsize);
	if (newbuf == NULL) {
	    syslog(LOG_ERR, "malloc failed at line %d, file %s", __LINE__,
                   __FILE__);
            exit(1);
	}
	c->buf = newbuf;
	c->buflen = newsize;
    }
}

/*
 * Functions pertaining to configuration.
 */

/***********************************************************************
 * config_find: return the config item with name
 */
config_item *
config_find(const char *name)
{
    int i;

    for (i = 0; config[i].name; i++)
	if (strcasecmp(config[i].name, name) == 0)
	    return &config[i];
    return NULL;
}

/***********************************************************************
 * config_get: return the appropriate value from the config item.
 * returns NULL if not found.
 */
void *
config_get(const char *name)
{
    config_item *ci;
    void * ptr = NULL;

    ci = config_find(name);
    if (ci)
	switch (ci->type) {
	case text:
	    ptr = ci->text;
	    break;
	case ary:
	    ptr = ci->ary;
	    break;
	}
    return ptr;
}

/***********************************************************************
 * config_set: set the value of the config item, appropriately.
 */
void
config_set(config_item *ci, char *val)
{
    switch (ci->type) {
    case text:
	config_set_text(ci, val);
	break;
    case ary:
	config_append_ary(ci, val);
	break;
    }
    return;
}

/***********************************************************************
 * config_set_text: set the text value of the config item.
 */
void
config_set_text(config_item *ci, char *val)
{
    if (ci == NULL || ci->type != text) {
	free (val);
	return;
    }
    if (ci->text)
	free (ci->text);
    ci->text = val;
    return;
}

/***********************************************************************
 * config_append_ary: append the value to ci's array.
 */
void
config_append_ary(config_item *ci, char *text)
{
    char **newary;

    if (ci == NULL)
	return;
    newary = arr_add(ci->ary, text);
    if (newary)
	ci->ary = newary;
    return;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
