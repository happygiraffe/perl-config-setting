/*********************************************************************
 * config.c
 *
 * Handling for spider's configuration file.
 *
 * Copyright 2000 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: config.c,v 1.1 2000/01/18 07:49:01 dom Exp $";

#include <config.h>             /* autoconf */
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
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

/* Globals */
char *	conf_file;		/* config file name */

/* a list of valid keywords that can appear in the config file */
static config_item config[] = {
    { "Facility",	text,	NULL,	NULL },
    { "Log_All_Cmds",	text,	NULL,	NULL },
    { "Module",		ary,	NULL,	NULL },
    { "Module_Dir",	ary,	NULL,	NULL },
    { "Pid_File",	text,	NULL,	NULL },
    { "Port",		text,	NULL,	NULL },
    { "Spool_Dir",	text,	NULL,	NULL },
    { "User_File",	text,	NULL,	NULL },
    { NULL,		text,	NULL,	NULL }
};

/* textual representation of syslog facilities */
static struct {
    const char *name;
    int val;
} facilities[] = {
#ifdef LOG_USER
    { "user",	LOG_USER },
#endif
#ifdef LOG_MAIL
    { "mail",	LOG_MAIL },
#endif
#ifdef LOG_DAEMON
    { "daemon",	LOG_DAEMON },
#endif
#ifdef LOG_AUTH
    { "auth",	LOG_AUTH },
#endif
#ifdef LOG_LPR
    { "lpr",	LOG_LPR },
#endif
#ifdef LOG_NEWS
    { "news",	LOG_NEWS },
#endif
#ifdef LOG_UUCP
    { "uucp",	LOG_UUCP },
#endif
#ifdef LOG_CRON
    { "cron",	LOG_CRON },
#endif
#ifdef LOG_FTP
    { "ftp",	LOG_FTP },
#endif
#ifdef LOG_NTP
    { "ntp",	LOG_NTP },
#endif
#ifdef LOG_LOCAL0
    { "local0",	LOG_LOCAL0 },
#endif
#ifdef LOG_LOCAL1
    { "local1",	LOG_LOCAL1 },
#endif
#ifdef LOG_LOCAL2
    { "local2",	LOG_LOCAL2 },
#endif
#ifdef LOG_LOCAL3
    { "local3",	LOG_LOCAL3 },
#endif
#ifdef LOG_LOCAL4
    { "local4",	LOG_LOCAL4 },
#endif
#ifdef LOG_LOCAL5
    { "local5",	LOG_LOCAL5 },
#endif
#ifdef LOG_LOCAL6
    { "local6",	LOG_LOCAL6 },
#endif
#ifdef LOG_LOCAL7
    { "local7",	LOG_LOCAL7 },
#endif
    { NULL, 0}
};

/* PROTOTYPES */
static void	config_check(void);

/*********************************************************************
 * config_check: check that all the variabled needed to proceed are
 * set.
 */
static void
config_check(void)
{
    char *tocheck[] = {
	"Module_Dir",
	"Module",
	"Pid_File",
	"Port",
	"User_File",
	NULL
    };
    int i;
    void *val;

    for (i = 0; tocheck[i]; ++i) {
	val = config_get(tocheck[i]);
	if (val == NULL) {
	    log (LOG_ERR, "no value for %s defined in config file",
		    tocheck[i]);
	    exit (1);
	}
    }
}

/*********************************************************************
 * get_facility: get the syslog facility from a string.
 */
int
get_facility(void)
{
    char *fac;
    int i;
    int val = LOG_LOCAL0;	/* a sane default */

    fac = config_get("Facility");
    for (i = 0; facilities[i].name != NULL; i++)
	if (strcasecmp(facilities[i].name, fac) == 0) {
	    val = facilities[i].val;
	    break;
	}
    return val;
}

/*********************************************************************
 * config_parse_file: read in the file as specified by conf_file (and
 * it's default, CONFIG_FILE) and places the values contained therein
 * into the config[] structures.
 */
void
config_parse_file(void)
{
    FILE *	cfg;
    char *	buf;
    char *	keyw;
    char *	val;
    int		i;
    int		lineno = 1;
    config_item	*ci;

    cfg = fopen(conf_file, "r");
    if(cfg == NULL) {
	perror("fopen");
	exit(1);
    }

    buf = get_line(cfg);
    while (!feof(cfg)) {
	/* Skip over comment only / blank / invalid lines */
	kill_comment(buf);
	i = num_tokens(buf);
	if (i < 2)
	{
	    if (i)
		log (LOG_WARNING, "invalid line (%d): %s", lineno, buf);
	    free(buf);
	    buf = get_line(cfg);
	    lineno++;
	    continue;
	}

	/* XXX should use strtok? */
	keyw = copy_token (buf, 0);
	val  = copy_token (buf, 1);
	ci   = config_find (keyw);
	if (ci) {
	    config_set(ci, val);
	    log (LOG_DEBUG, "config: %s %s", keyw, val);
	} else {
	    log (LOG_WARNING, "unknown keyword \"%s\" on line %d", 
		 keyw, lineno);
	    free (val);
	}

	free (keyw);
	free (buf);
	buf = get_line(cfg);
	lineno++;
    }

    /* We need all the fd's we can get... */
    fclose(cfg);
    config_check();
}

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
    if (ci == NULL)
	return;
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

    if (ci == NULL || ci->type != ary) {
	free (text);
	return;
    }
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
