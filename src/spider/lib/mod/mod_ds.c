/*********************************************************************
 * ds.c
 *
 * Data structure manipulation routines for libmod.  Some of this is
 * filched from Spider's own ds.c routines.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 */

static const char rcsid[]="@(#) $Id: mod_ds.c,v 1.1 2000/01/05 08:09:57 dom Exp $";

#include <stdlib.h>
#include "mod.h"

/*********************************************************************
 * cmd_count
 *
 * Returns the number of elements in a Cmdp array.
 */
int
cmd_count(Cmdp * arr)
{
    int i = 0;

    if (arr != NULL)
	while(arr[i] != (Cmdp)NULL)
	    i++;
    return i;
}

/*********************************************************************
 * cmd_add
 *
 * Adds a command (typedef Cmdp) onto the end an array of commands,
 * ensuring that the final element remains NULL.  Uses the original
 * Cmdp, not a copy, so the original must be dynamically allocated.
 * Returns a new pointer to the array.  Take heed!
 *
 * eg: argv = arr_add(argv, my_arg);
 *     if (argv == NULL)
 *         perror ("arr_add");
 */
Cmdp *
cmd_add(Cmdp * arr, Cmdp foo)
{
    Cmdp * tmp_arr;
    int i;

    if (arr == NULL)
    {
	tmp_arr = calloc(2, sizeof(Cmdp));
	tmp_arr[0] = foo;
	tmp_arr[1] = (Cmdp)NULL;
    } else{
	i = cmd_count(arr);
	tmp_arr = realloc(arr, (i+2)*sizeof(Cmdp));
	tmp_arr[i] = foo;
	tmp_arr[i+1] = (Cmdp)NULL;
    }

    return tmp_arr;
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
            tmp_arr = calloc(2, sizeof(char *));
            tmp_arr[0] = str;
            tmp_arr[1] = NULL;
        } else {
            i = arr_count(arr);
            /* Musn't forget to include the NULL as well as the new str */
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
 * new_reply
 *
 * Just allocate memory for storing a series of replies. 
 */
Reply
new_reply(Reply rep)
{
    if ((rep.text == NULL) && (rep.num == 0)) {
        rep.text = calloc(2, sizeof(char **));
        rep.text[0] = NULL;
        rep.text[1] = NULL;
        rep.num = 1;
    } else {
        rep.text = realloc(rep.text, (rep.num+2) * sizeof(char ***));
        rep.text[rep.num] = NULL;
        rep.text[rep.num+1] = NULL;
        rep.num++;
    }

    return rep;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
