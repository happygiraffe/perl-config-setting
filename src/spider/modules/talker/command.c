#include <stdio.h>
#include <string.h>
#include "command.h"
#include "protos.h"

#define BUFSIZE 256

extern int broom (Cmdline *cmdline);
extern int logerr (char *, char *);
extern int bchan (Cmdline *);

Cmd commands[] = {
	{0, "bchan"},   /* broadcast to a channel */
	{1, "join"},    /* join a channel */
	{2, "synoff"},  /* leave a channel */
	{3, "clist"},   /* list users in channel */
	{(int)NULL, (char *)NULL}
};

/*-| module_listcommands |--------------------------------------------------*/
/* Probably be the same for each module, but it's here as it needs access to
  the modules command list */
/* TODO: Use externs to move this into a global file (utils.c?) */

int module_list_cmds(void) {
  int count;
  char buf[BUFSIZE+1];
  
  for (count=0; commands[count].name != NULL; ++count) {
    printf ("%s\n", commands[count].name);
    fgets(buf, BUFSIZE, stdin);
    if (strncmp(buf, "211", 3) != 0)
      logerr("Couldn't register command", commands[count].name);
  }
  printf(".\n");
  return(1);
}


/*-| module_find_cmd_no |---------------------------------------------------*/
/* Probably also the same for each module */
/* TODO: Move this into parse.c somehow */ 

int module_find_cmd_no(char *cmd) {
  int count;
  
  for (count=0; commands[count].name != NULL; ++count)
    if (strcmp(commands[count].name, cmd)==0)
      return(commands[count].number);
  return(-1);
}


/*-| module_do_cmd |-------------------------------------------------------*/

int module_do_cmd(Cmdline *cmdline) {

  switch (cmdline->cmd_no) {
    case 0 : /* broom */
      bchan(cmdline);
      return(OK);
    case 1 : /* join */
    	join(cmdline);
    	return(OK);
    case 2 : /* synoff */
    	synoff(cmdline);
    	return(OK);
    case 3 : /* clist */
    	clist(cmdline);
    	return(OK);
  }

  /* can't happen */
  return(ERROR);
}

/*--------------------------------------------------------------------------*/
