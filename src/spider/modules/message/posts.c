#include <sys/types.h>

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "command.h"
#include "config.h"
#include "posts.h"
#include "replies.h"

/* Externs */
extern int logerr (char *error, char *extra);
extern int bprintf (char *,...);
extern int setreply (char *, char *, char *);
extern void flush_cmd (void);

/* Global */
char *module_name;


/*-| gettype |-------------------------------------------------------------*/

int 
gettype (char *sptr)
{
  if (strncmp ("post", sptr, 4) == 0)
    return (POST);

  if (strncmp ("mail", sptr, 4) == 0)
    return (MAIL);

  return (0);			/* No match */
}


/*-| valid_msgid |---------------------------------------------------------*/

int 
valid_msgid (char *msgid)
{
  int count;
  int i = 1;

  if (strlen (msgid) != MSGIDLEN)
    {
      return (0);
    }
  for (count = 0; (count < MSGIDLEN) && i; ++count)
    if (!isdigit ((int)msgid[count]))
      i = 0;
  return (i);
}


/*-| displaymsg |----------------------------------------------------------*/

int 
displaymsg (int flag, char *fn)
{

  FILE *fp;

  char buf[BUFSIZE + 1];

  if ((fp = fopen (fn, "r")) == NULL)
    return (0);


  if (flag == HEADERS)
    while (1)
      {
	fgets (buf, BUFSIZE, fp);
	if (buf[0] == '\n')
	  break;
	if (feof (fp))
	  break;
	bprintf ("%s", buf);
      }
  else
    {
      fgets (buf, BUFSIZE, fp);
      while (!feof (fp))
	{
	  /* Stop "\n"s being output, as that means the end of message. */
	  if (buf[0] == '\n')
	    bprintf (" \n");
	  /* Similar with ".\n"s, that means end of output */
	  else if (strncmp (buf, ".\n", 2) == 0)
	    bprintf (". \n");
	  else
	    bprintf ("%s", buf);
	  fgets (buf, BUFSIZE, fp);
	}
    }
  bprintf ("\n");
  fclose (fp);
  return (1);
}


/*-| getnewmesgid |--------------------------------------------------------*/

int 
getnewmesgid (void)
{

  struct dirent *dent;
  DIR *dp;
  int msgid;

  msgid = 0;

  if ((dp = opendir (".")) == NULL)
    return (0);

  while ((dent = readdir (dp)) != NULL)
    if ((dent->d_name[0] != '.') && (msgid < atoi (dent->d_name)))
      msgid = atoi (dent->d_name);

  return (msgid + 1);
}



/*-| writemsg |------------------------------------------------------------*/

int 
writemsg (struct header_struct *header, int type)
{

  int count;
  char fn[MSGIDLEN + 1];
  FILE *fp;
  char buf[BUFSIZE + 1];

  sprintf (fn, "%8d", getnewmesgid ());

  for (count = 0; fn[count] == ' '; fn[count] = '0', ++count);

  header->msgid = fn;

  if ((fp = fopen (fn, "w")) == NULL)
    return (0);


  /* Write header info */

  if (type == POST)
    fprintf (fp, "Area: %s\n", header->area);
  else
    fprintf (fp, "To: %s\n", header->area);
  fprintf (fp, "Id: %s\n", header->msgid);
  fprintf (fp, "From: %s\n", header->author);
  fprintf (fp, "Subject: %s\n", header->subject);

  fprintf (fp, "\n");

  /* Read & write message body - do not return error after this point, 
     or the next persons command could be wiped */

  while (strncmp (".\n", fgets (buf, BUFSIZE, stdin), 2) != 0) 
      fprintf (fp, "%s", buf);

  fclose (fp);

  return (1);
}


/*-| putmsg |--------------------------------------------------------------*/
/*
   Command syntax:
   putmsg <type>, <userid/board area>, <subject>
   <long input>
 */
int 
putmsg (Cmdline * cmdline)
{

  int type;
  struct header_struct header;
  /*char *sptr; unused */
  char area[MAXAREALEN + 1];
  char *subject;
  char format[20];
  int count;
  /*  struct dirent **namelist; */

  /* Allocate clean memory so char array is automatically null-terminated */
  subject = (char *) calloc (1, MAXSUBJECTLEN + 1);

  /* find type & record as int */
  type = gettype (cmdline->param);

  if (type == 0)
    {
      setreply (cmdline->userid, "500", "H_BAD_PARAM");
      flush_cmd ();
      return (0);
    }


  sprintf (format, "%%*%ds %%%ds %%%dc", TYPELEN, MAXAREALEN, MAXSUBJECTLEN);
  count = sscanf (cmdline->param, format, area, subject);

  if (count < 2)
    {
      setreply (cmdline->userid, "500", H_BAD_PARAM);
      flush_cmd ();
      return (0);
    }

  header.area = area;
  header.subject = subject;

  /* Change into the right directory */

  if (type == POST)
    {
      if (chdir (POSTDIR) != 0)
	{
	  logerr ("putmsg: Couldn't change into POSTDIR", POSTDIR);
	  setreply (cmdline->userid, "500", H_FAIL);
	  flush_cmd ();
	  return (0);
	}
    }

  else
    {
      if (chdir (MAILDIR) != 0)
	{
	  logerr ("putmsg: Couldn't change into MAILDIR", MAILDIR);
	  setreply (cmdline->userid, "500", H_FAIL);
	  flush_cmd ();
	  return (0);
	}
    }

  if (chdir (header.area) != 0)
    {
      setreply (cmdline->userid, "510", H_AREA_NOT_EXIST);
      flush_cmd ();
      return (0);
    }

  header.author = cmdline->userid;

  if (!writemsg (&header, type))
    {
      setreply (cmdline->userid, "510", H_FAIL);
      flush_cmd ();
      return (0);
    }

  setreply (cmdline->userid, "211", H_OK);
  /* Don't flush_cmd(), writemsg has already used it all up */
  return (1);
}


/*-| getmsg |--------------------------------------------------------------*/

/* 
   Syntax:
   getmsg <type>, <userid/board area>, <messageid>
 */
int 
getmsg (int flag, Cmdline * cmdline)
{

  int type;
  char area[MAXAREALEN];
  char msgid[MSGIDLEN + 1];
  /* char format[20]; unused */
  int count;
  struct dirent *dent;
  DIR *dp;
  int msgidno;



  /* find type & record as int */
  type = gettype (cmdline->param);

  if (type == 0)
    {
      setreply (cmdline->userid, "500", H_BAD_PARAM);
      flush_cmd ();
      return (0);
    }

  count = sscanf (cmdline->param, "%*s %s %s", area, msgid);

  if (count < 2)
    {
      setreply (cmdline->userid, "500", H_BAD_PARAM);
      flush_cmd ();
      return (0);
    }

  if (!valid_msgid (msgid))
    {
      setreply (cmdline->userid, "500", H_BAD_MSGID);
      flush_cmd ();
      return (0);
    }

  /* Change into the right directory */

  if (type == POST)
    {
      if (chdir (POSTDIR) != 0)
	{
	  setreply (cmdline->userid, "510", H_FAIL);
	  flush_cmd ();
	  return (0);
	}
    }
  else
    {
      if (chdir (MAILDIR) != 0)
	{
	  setreply (cmdline->userid, "510", H_FAIL);
	  flush_cmd ();
	  return (0);
	}
    }

  if (chdir (area) != 0)
    {
      setreply (cmdline->userid, "510", H_AREA_NOT_EXIST);
      flush_cmd ();
      return (0);
    }

  /* Getting messy..  If they just want headers, also give them the headers
     that came after the one they specified.  Otherwise, if they want the whole
     message, give them nothing but that message. */

  if (flag == HEADERS)
    {

      if ((dp = opendir (".")) == NULL)
	return (0);

      msgidno = atoi (msgid);

      while ((dent = readdir (dp)) != NULL)
	if ((dent->d_name[0] != '.') && (atoi (dent->d_name) >= msgidno))
	  displaymsg (flag, dent->d_name);
    }
  else
    displaymsg (flag, msgid);

  setreply (cmdline->userid, "211", H_OK);
  flush_cmd ();
  return (1);
}


/*-| getareas |------------------------------------------------------------*/

/* Syntax:
   getars */
int 
getareas (Cmdline * cmdline)
{

  struct dirent *dent;
  DIR *dp;
  int msgid;

  msgid = 0;

  if ((dp = opendir (POSTDIR)) == NULL)
    return (0);

  while ((dent = readdir (dp)) != NULL)
    if (dent->d_name[0] != '.')
      bprintf ("%s\n", dent->d_name);

  setreply (cmdline->userid, "211", H_OK);
  flush_cmd ();
  return (1);
}


/*-| module_init |---------------------------------------------------------*/

int 
module_init (void)
{

  module_name = MODULE_NAME;
  return (1);
}

/*-------------------------------------------------------------------------*/
