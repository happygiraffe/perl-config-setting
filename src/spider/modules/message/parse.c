#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "command.h"

/* I prefer to see externs in the C files rather than headers */
extern int module_do_cmd(Cmdline *cmdline);
extern int module_find_cmd_no(char *cmd);

extern int module_name;

/*-| getcmd |---------------------------------------------------------------*/

/* Leaves on trailing \n */
int getcmd(char buf[MAXCOMLEN]) {
  int length;
  
  if (fgets(buf, MAXCOMLEN, stdin) == NULL)
    return(0);
  length = strlen(buf);
  
  if (buf[length-1] != '\n')
    return(0);
  
  return(1);
}


/*-| parse |----------------------------------------------------------------*/

int parse(Cmdline *cmdline, char buf[]) {
  
  int clc; /* command line count */
  int count;
	char format[20];

	sprintf(format, "%%%ds %%%ds %%%dc", MAXUSERIDLEN, MAXCMDNAMELEN, 
		MAXPARAMLEN-1);
     
  clc = sscanf(buf, format, cmdline->userid, cmdline->cmd_name, 
  	cmdline->param);
	
	/* We need at least a userid and command, so minimum is 2 */
  if (clc < 2) {
  	/* Return error, leaving cmdline->cmd_no as NULL */
  	return(0);
  }

	/* Replace trailing \n which should be in cmd_param with \0.  This copes 
		with the fact that %c does not add a trailing \0 itself. */
  
	for (count=0; cmdline->param[count] != '\n'; ++count);

	cmdline->param[count] = '\0';

  if ( (cmdline->cmd_no = module_find_cmd_no(cmdline->cmd_name)) < 0)
  	return(0);
	
  return(1);
}

/*--------------------------------------------------------------------------*/
