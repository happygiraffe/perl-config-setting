#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "command.h"

extern Reply reply;

/*-| bprintf |--------------------------------------------------------------*/

/* Write to temporary output buffer */
int bprintf(char *fmt, ...) {
  va_list args;
  
  /* TODO: Set an error condition here? */
  if (reply.buf_fp == NULL)
    if ((reply.buf_fp = fopen(reply.buf_fn, "w"))==NULL)
    	return(0);
  
  va_start(args, fmt);

	if (vfprintf(reply.buf_fp, fmt, args) == EOF) {
	 	va_end(args);
   	return(0);
  }

  va_end(args);
  
  return(1);
}

/*-| setreply |-------------------------------------------------------------*/

void setreply(char *userid, char *tdc, char *human) {
  reply.userid = userid;
  strncpy(reply.tdc, tdc, 3);
  if (strlen(human) <= MAXHUMANLEN)
    reply.human = human;
}

/*-| flush_cmd |------------------------------------------------------------*/

void flush_cmd(void) {
	char buf[MAXCOMLEN+1];

	while (strcmp(".\n", fgets(buf, MAXCOMLEN, stdin)) != 0);
}

/*--------------------------------------------------------------------------*/
