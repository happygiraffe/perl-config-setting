/* Mostly by alex@area51.upsu.plym.ac.uk */

#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "command.h"
#include "main.h"

extern int module_list_cmds (void);
extern int logerr (char *error, char *extra);
extern int getcmd (char buf[]);
extern int parse (Cmdline * cmdline, char buf[]);
extern int module_do_cmd (Cmdline * cmdline);
extern void setreply (char *, char *, char *);
extern void flush_cmd (void);

extern char *module_name;

/* Reply structure */
Reply reply;

/*-| init_reply |-----------------------------------------------------------*/

int 
init_reply (Cmdline * cmdline)
{

  if (reply.buf_fn != NULL)
    {
      unlink (reply.buf_fn);
      free (reply.buf_fn);
    }

  cmdline->userid[0] = '\0';
  strcpy (reply.tdc, "xxx");
  /* We don't need to initialise the module name each time */
  reply.human = "No message";
  reply.buf_fn = tempnam (TEMPDIR, module_name);
  reply.buf_fp = NULL;
  return (1);
}

/*-| initialise |-----------------------------------------------------------*/

int 
initialise (Cmdline * cmdline)
{

  /* Check we're talking to a spider */
  if (isatty (0))
    {
#ifdef DEBUG
      printf ("Welcome, de bugger.\n");
#else
      printf ("You don't look like a spider to me..\n");
      return (0);
#endif
    }
  setvbuf (stdin, NULL, _IOLBF, 0);
  setvbuf (stdout, NULL, _IOLBF, 0);
  /* Shouldn't really use stderr, anyway. */
  setvbuf (stderr, NULL, _IOLBF, 0);

  init_reply(cmdline);

  return (1);
}

/*-| do_reply |-------------------------------------------------------------*/

int 
do_reply (Cmdline * cmdline)
{

  char buf[MAXCOMLEN + 1];

  if (cmdline->userid[0] == '\0')
    /* No-one to reply to */
    return (0);

  /* Print the reply header - 
     <userid> <three digit wassname> <info for humans>\n 
   */
  printf ("%s %s %s\n", cmdline->userid, reply.tdc,
	  reply.human);

  /*shove longoutput */

  if (reply.buf_fp != NULL)
    {
      /* We have some output */
      /* Close file for writing, open it for reading */
      fclose (reply.buf_fp);
      if ((reply.buf_fp = fopen (reply.buf_fn, "rt")) != NULL)
	{
	  while (fgets (buf, MAXCOMLEN, reply.buf_fp) != NULL)
	    {
	      if (strcmp (".\n", buf) == 0)
		fputs (". \n", stdout);
	      else
		fputs (buf, stdout);
	    }
	  fclose (reply.buf_fp);
	}
    }
  fputs (".\n", stdout);
  init_reply (cmdline);
  return (1);
}


/*-| main |-----------------------------------------------------------------*/

int 
main (void)
{

  char buf[MAXCOMLEN];
  Cmdline cmdline;
  struct timeval to;

  /* Start-up sequence */
  if (!initialise (&cmdline))
    return (0);
  if (!module_list_cmds ())
    {
      logerr ("main.c", "Couldn't list commands");
      return (1);
    }

  /* Main loop */
  do
    {
      getcmd (buf);
      /* A command? */
      if (!isdigit ((int)buf[0]))
	{
	  if (parse (&cmdline, buf))
	    /* We have something to reply to */
	    module_do_cmd (&cmdline);
	  else
	    {
	      setreply (cmdline.userid, "xxx", "Bad command.");
	      flush_cmd ();
	    }
	  to.tv_sec = 0;
          to.tv_usec = 100000;
	  select(0, NULL, NULL, NULL, &to);

	  if (!do_reply (&cmdline))
	    logerr ("main: Command dropped on the floor", buf);
	}
      else
	flush_cmd ();
    }
  while (1);			/* I'm INVINCIBLE */
}

/*--------------------------------------------------------------------------*/
