/*********************************************************************
 * mod.h
 *
 * Header for Spider modules' Library.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 *
 * @(#) $Id: mod.h,v 1.1 1999/03/11 15:39:49 dom Exp $
 */

#ifndef _MOD_H_
#define _MOD_H

#include <stdio.h>
/*
 * These are links to the ones in the main spider directory.
 */
#include "codes.h"
#include "config.h"

/* Defines */

#define CUR_REP(x)	(x).text[(x).num -1]

/* Data structures used in the library */

typedef enum {
    false = 0,
    true = 1
} Bool;

typedef struct {
    char ***	text;           /* Pointer to array of texts */
    unsigned	num;            /* Number of replies in above */
} Reply;

typedef struct {
    char * 	name;
    Reply	(*func)(char ** input);
    void	(*errfunc)(char ** input);
} Cmd;
typedef Cmd * Cmdp;

/*********************************************************************
 * Prototypes
 */

/* ds.c */
int 	cmd_count(Cmdp * arr);
Cmdp * 	cmd_add(Cmdp * arr, Cmdp foo);
int 	arr_count(char ** arr);
char ** arr_add(char ** arr, char * str);
void 	arr_del(char ** arr);
Reply	new_reply(Reply rep);

/* io.c */
char **	get_mesg(FILE *fp);
char *	get_line(FILE * fp);
void	put_mesg(FILE *fp, char **msg);

/* mod.c */
void 	mod_init(char * name);
Bool 	mod_reg_cmd(char * name, Reply (*func)(char ** input),
                    void (*erf)(char ** input));
void 	mod_main_loop(void);

/* utils.c */
Bool 	ck_buf(char * buf, int size);
Bool	eot(char *c);
char *	make_repline(char * name, int code, char * msg);
char *	make_unsolic(char * name, char * tag);
char **	make_error(char * name, int code, char * msg);
int	iserror(char *s);
size_t 	len_token(char * tok);
char * 	find_token(char * buf, int n);
char * 	copy_token(char * buf, int n);
int 	num_tokens(char * buf);
Bool 	cmp_token(char * buf, int n, char * s);

#endif /* _MOD_H_ */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
