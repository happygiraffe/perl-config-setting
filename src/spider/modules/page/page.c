/*********************************************************************
 * page.c
 *
 * Provides a "page" command for spider.  Yummy.  Also sits as a
 * testbed for my version of the hashing functions demo'd in K&R
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: page.c,v 1.2 2000/01/12 21:21:48 dom Exp $";

#include <stdlib.h>
#include <string.h>
/* Module specific includes */
#include <mod.h>

/* This is sent after an OK_UNSOLIC code */
#define PAGER_TAG	"page"
/* This defines how big the table of hashes is */
#define HASH_SIZE	101

typedef struct node * Nodep;
typedef struct node {
    Nodep	next;
    Nodep	prev;
    char *	name;
} Node;

/* global variables */
static Nodep hashtab[HASH_SIZE];

Reply	cmd_page(char ** msg);
void	err_page(char ** msg);
unsigned hash(char *s);
Nodep	lookup(char *s);
Bool	usr_list_find(char * name);
void	usr_list_add(char * name);
void	usr_list_del(char * name);

/*********************************************************************
 * main
 *
 * Everything hangs on here.  No arguments are used.  In fact, it's
 * illegal to use them. hahaha.
 */
int
main(int argc, char ** argv)
{
    /* Do protocol to register our keyword. */
    mod_init("page");
    mod_reg_cmd("PAGE", cmd_page, err_page);
    mod_reg_cmd(NULL, NULL, NULL);

    /* And now sit in a loop */
    mod_main_loop();

    exit(0);
    return 0;
  
}

/*********************************************************************
 * cmd_page
 *
 * Does one of two things: Either (dis)activates paging for a single
 * user, or sends a message to a user (with confirmation to sender).
 */
Reply
cmd_page(char ** msg)
{
    Reply	rep = {NULL, 0};
    char *	name;
    char *	recip;
    char * 	cmd;
    Bool	ok = true;

    /* setup */
    rep = new_reply(rep);
    name = copy_token(msg[0], 0);
    cmd = copy_token(msg[0], 2);
    recip = copy_token(msg[0], 3);

    /* Do we have a command? */
    if (ok && (cmd == NULL)) {
        CUR_REP(rep) = make_error(name, ERR_SYNTAX, ERR_SYNTAX_MSG);
        ok = false;
    }

    /* Enable paging for this user */
    if (ok && (strcasecmp(cmd, "ON") == 0)) {
        usr_list_add(name);
        CUR_REP(rep) = arr_add(CUR_REP(rep),
                               make_repline(name, OK_CMD, "Paged in"));
        CUR_REP(rep) = arr_add(CUR_REP(rep), strdup(PAGER_TAG));
        CUR_REP(rep) = arr_add(CUR_REP(rep), strdup(END_OF_DATA));
        ok = false;
    }
    /* Disable pageing for this user */
    if (ok && (strcasecmp(cmd, "OFF") == 0)) {
        usr_list_del(name);
        CUR_REP(rep) = make_error(name, OK_CMD, "Paged out");
        ok = false;
    }
    /* Send a message... */
    if (ok && (strcasecmp(cmd, "TO") == 0)) {
        /* If the command format is ok... */
        if (recip == NULL) {
            CUR_REP(rep) = make_error(name, ERR_SYNTAX, ERR_SYNTAX_MSG);
            ok = false;
        } 
        if (arr_count(msg) < 3) {
            CUR_REP(rep) = make_error(name, ERR_SYNTAX,
                                      strdup("No NULL pages, please"));
            ok = false;
        }
        /* And the user actually *wants* to receive messages */
#ifndef DEBUG
        if (ok && (usr_list_find(recip) == false)) {
            CUR_REP(rep) = make_error(name, ERR_BADUSR, ERR_BADUSR_MSG);
            ok = false;
        }
#endif /* DEBUG */
        if (ok) {
            CUR_REP(rep) = make_error(name, OK_CMD, OK_CMD_MSG);
            /* start a reply to the person being paged */
            rep = new_reply(rep);
            CUR_REP(rep) = arr_add(CUR_REP(rep),
                                   make_unsolic(recip, PAGER_TAG));
            /* Send the name of the sender.  This puts management of
               the earlier allocated "name" into the hands the
               replying mechanisms. */
            CUR_REP(rep) = arr_add(CUR_REP(rep), name);
            /* only send 1-line pages, truncate the rest */
            CUR_REP(rep) = arr_add(CUR_REP(rep), strdup(msg[1]));
            CUR_REP(rep) = arr_add(CUR_REP(rep), strdup(END_OF_DATA));
            ok = false;
        }
    }
    if (ok) {
        CUR_REP(rep) = make_error(name, ERR_SYNTAX, ERR_SYNTAX_MSG);
        ok = false;
    }

    if (cmd != NULL) {
        free(cmd);
    }
    if (recip != NULL) {
        free(recip);
    }
    return rep;
}

/*********************************************************************
 * err_page
 *
 * Handles the unlikely event of an error occuring...
 */
void
err_page(char ** input)
{
    char *	c;

    /* So far, we only have to deal with ERR_BADUSR, the name of which
       is contained in msg[1]. */
    if (arr_count(input) >= 3) {
        if (num_tokens(input[1]) > 0) {
            c = copy_token(input[1], 0);
            usr_list_del(c);
            free(c);
        }
    }
}

/*********************************************************************
 * hash
 *
 * Makes a hash of the string given to it.
 */
unsigned
hash(char * s)
{
    unsigned	hashval;

    for(hashval = 0; *s != '\0'; s++) {
        hashval = *s + 31 * hashval;
    }
    return hashval % HASH_SIZE;
}

/*********************************************************************
 * lookup
 *
 * Find the node containing s.  Return NULL if not present.
 */
Nodep
lookup(char *s)
{
    Nodep	np = NULL;

    if (s != NULL) {
        for(np = hashtab[hash(s)]; np != NULL; np = np->next) {
            if (strcasecmp(s, np->name) == 0) {
                break;
            }
        }
    }
    return np;
}

/*********************************************************************
 * usr_list_find
 *
 * Returns true if a user is in the list
 */
Bool
usr_list_find(char * name)
{
    return (lookup(name) == NULL) ? false : true;
}

/*********************************************************************
 * usr_list_add
 *
 * Adds a user into the supposed list which is really a hash behind
 * the scenes.  If the user already exists, good.
 */
void
usr_list_add(char * name)
{
    Nodep	np;
    unsigned	hashval;

    if (name != NULL) {
        if ((np = lookup(name)) == NULL) {
            np = calloc(1, sizeof(Node));
            np->name = strdup(name);
            hashval = hash(name);
            np->next = hashtab[hashval]; /* so what if its NULL? :) */
            /* This little "if" saves *so* much trouble later... */
            if (np->next != NULL) {
                np->next->prev = np;
            } else {
                np->prev = NULL;
            }
            hashtab[hashval] = np;
        } 
    }
}

/*********************************************************************
 * usr_list_del
 *
 * Delete a user from the (ahem) list.  Doesn't matter if they don't
 * exist.
 */
void
usr_list_del(char * name)
{
    Nodep	np;
    unsigned	hashval;

    if (name != NULL) {
        hashval = hash(name);
        for (np = hashtab[hashval]; np != NULL; np++) {
            if (strcasecmp(name, np->name) == 0) {
                /* zap it */
                if (np->prev != NULL) {
                    np->next->prev = np->prev;
                } else {
                    hashtab[hashval] = NULL;
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
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
