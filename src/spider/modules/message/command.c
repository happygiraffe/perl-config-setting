#include <stdio.h>
#include <string.h>
#include "command.h"
#include "posts.h"

#define MODULE_NAME msg

extern int putmsg(Cmdline *cmdline);
extern int getmsg(int type, Cmdline *cmdline);
extern int getareas(Cmdline *cmdline);
extern int logerr(char *, char *);

Cmd commands[] = {
	{0, "putmsg"},
	{1, "getmsg"},
	{2, "gethed"},
	{3, "getars"},
	{(int)NULL, (char *)NULL}
};

/*-| module_listcommands |--------------------------------------------------*/
/* Probably be the same for each module, but it's here as it needs access to
  the modules command list */

int module_list_cmds(void) {
  int count;
  char buf[BUFSIZE+1];
  
  for (count=0; commands[count].name != NULL; ++count) {
    printf ("%s\n", commands[count].name);
  
    fgets(buf, BUFSIZE, stdin);
    if (strncmp(buf, "211", 3)!=0)
      logerr("Couldn't register command", commands[count].name);
  }  
  printf(".\n");
  return(1);
}


/*-| module_find_cmd_no |---------------------------------------------------*/
/* Probably also the same for each module */

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
    case 0 : /* putmsg */
      putmsg(cmdline);
      return(OK);
    case 1 : /* getmsg */
      getmsg(FULLMSG, cmdline);
      return(OK);
    case 2 : /* gethed */
      getmsg(HEADERS, cmdline);
      return(OK);
    case 3 : /* getars */
      getareas(cmdline);
      return(OK);
  }

  /* can't happen */
  return(ERROR);
}

/*--------------------------------------------------------------------------*/
