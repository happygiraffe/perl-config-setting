#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "command.h"
#include "config.h"
#include "talker.h"
#include "replies.h"

/* Externs */
extern Reply reply;

/* Globals */
static Channel chanlist[CHANNELS]; /* Chat channel structures */
char *module_name;	/* Holds MODULE_NAME for other files to see */

/*-| make_unode |-----------------------------------------------------------*/

Usernode *make_unode(void) 
{
  /* TODO: Replace printf with proper log */
  Usernode *unode_ptr;
  
  unode_ptr = (Usernode *) malloc(sizeof(Usernode));
  
  if (unode_ptr == NULL)
    logerr("make_unode: Could not allocate memory\n", "");
  else {
    /* initialise */
    unode_ptr->userid = NULL;
  	unode_ptr->prev = NULL;
  	unode_ptr->next = NULL;
  }
	return(unode_ptr);
}


/*-| make_ulist |-----------------------------------------------------------*/

Userlist *make_ulist(void) 
{
  /* TODO: Replace printf with proper log */
  Userlist *ulist_ptr;
  
  ulist_ptr = (Userlist *) malloc(sizeof(Userlist));
  if (ulist_ptr == NULL)
    logerr("make_ulist: Could not allocate memory\n", "");
  else {
    /* initialise */
  	ulist_ptr->first = NULL;
  	ulist_ptr->nodes = 0;
  }
	return(ulist_ptr);
}


/*-| finduser |-------------------------------------------------------------*/

Usernode *finduser(char *userid, Userlist *ulist, int flag) 
/* TODO: Make this function smaller (so it will fit in my brain for longer) */
{	
	Usernode *unode, *new_unode;
	int count;
	int cmpval;
	
	if (ulist == NULL)
		return(NULL);
	
	if (ulist->first == NULL) {
  	assert(ulist->nodes == 0);
  	new_unode = ulist->first = ulist->last = make_unode();
  	new_unode->userid = strdup(userid);
  	++ulist->nodes;
  	return(new_unode);
  }
	
	unode = ulist->first;
	
	for (count=0; count < ulist->nodes; ++count) {
	
	  cmpval = strcasecmp(userid, unode->userid);
	  
	  if (cmpval == 0) 
	    /* Found! */
	    if (flag == CREATE_ONLY)
	      /* Already there, would be bad to create another */
	      return(NULL);
	    else
	      /* Good match */
	      return(unode);
	  else {	  
	    if (cmpval < 0) {
	      /* Not found */
	      if (flag == FIND_ONLY)
	      	/* Unsuccessful search */
	      	return(NULL);
	     	else {
	     		/* Create - insert before current unode */
	     		new_unode = make_unode();
	     		if (unode->prev != NULL)
		     		unode->prev->next = new_unode;
	     		new_unode->prev = unode->prev;
	     		new_unode->next = unode;
	     		unode->prev = new_unode;
	     		
	     		/* Is unode now the first in the list? */
	     		if (new_unode->prev == NULL) {
	     		  assert(ulist->first = unode);
	     		  ulist->first = new_unode;
	     		}

		    	/* Copy string across - uses malloc, must remember to free() */
					new_unode->userid = strdup(userid);
					++ulist->nodes;

	     		return(new_unode);
	     	}
	    }
	  }
	  
	  if(unode->next == NULL) {
	    /* End of list */
	    assert(count+1 == ulist->nodes);
	    if (flag == FIND_ONLY)
	    	return(NULL);
	    else {
	    	/* Create - insert after current (last) unode */
	    	new_unode = make_unode();
	    	unode->next = new_unode;
	    	new_unode->prev = unode;
	    	new_unode->next = NULL;
	    	ulist->last = new_unode;
	    	
	    	/* Copy string across - uses malloc, must remember to free() */
				new_unode->userid = strdup(userid);
				++ulist->nodes;
				
	    	return(new_unode);
	    }
	  }
		unode = unode->next;
	}
	return(NULL);
}


/*-| adduser |--------------------------------------------------------------*/

int adduser(char *userid, int channel) 
{
  if (chanlist[channel].ulist == NULL)
  	chanlist[channel].ulist = make_ulist();
	return(finduser(userid, chanlist[channel].ulist, CREATE_ONLY) != NULL);
}


/*-| deluser |--------------------------------------------------------------*/

int deluser(char *userid, int channel)
{
	Usernode *unode;
	Userlist *ulist;
	
	/* Make statements a bit shorter */ 
	ulist = chanlist[channel].ulist;
	
	unode = finduser(userid, ulist, FIND_ONLY);
	
	if (unode == NULL)
		return(0);

	/* Found - try to sort out the links */
	
	if (unode->prev == NULL) {
		/* The expungee is first in the list */
		ulist->first = unode->next;
		if (unode->next != NULL)
			/* There are others in the list */
			unode->next->prev = NULL;
  	}
	else
		unode->prev->next = unode->next;
		
	if (unode->next == NULL) {
		/* The expungee is last in the list */
		ulist->last = unode->prev;
		if (unode->prev != NULL)
			/* There are others in the list */
			unode->prev->next = NULL;
	}
	else
		unode->next->prev = unode->prev;

	/* Free up the memory */
	if (unode->userid != NULL) /* It shouldn't be null, but just in case.. */
		free(unode->userid);
	free(unode);
	
	/* Decrement the node count */
	--ulist->nodes;
	
	return(1);
}


/*-| str2channel |----------------------------------------------------------*/

/* Returns channel number converted from the start of *sptr, or -1 on error */
int str2channel(char *sptr)
{
	int channel;
	char *message;
	char format[20];

	/* Using atoi - which means we can't trap errors.. */
	channel = atoi(sptr);
	
	if (channel == 0) {
		/* Hmm..  Either the channel really is 0, or atoi couldn't find any 
			number.  Find out which is true, we don't want channel 0 filling up
			with error messages.. */
		
		/* Cut off leading white space, as atoi does */
		while (isspace(*sptr)) ++sptr;
		
		if (*sptr != '0')
			/* Aha, not a real 0, return error */
			return(-1);
	}
	return(channel);
}


/*-| broadcast |------------------------------------------------------------*/

/* Returns number of users broadcast to */
int broadcast (int channel, char *message, Cmdline *cmdline) {
	/* 
	TODO: 
    o Receive list of unobtainable users from spider and delete them 
      from list (or something)
  */

	Usernode *unode;
	
	if (chanlist[channel].ulist == NULL)
		return(0);
	
	unode = chanlist[channel].ulist->first;
	
	while (unode != NULL) {
		/* printf rather than bprintf as it isn't a reply */
		printf("%s 280 tmesg Unsolicited talk message:\n%d\n%s\n%s\n.\n", 
			unode->userid, channel, cmdline->userid, message);
		unode = unode->next;
		sleep(1);
	}
	
	/* Return number of users broadcast to (so 0 is returned if the channel 
	   was empty) */
	/* return(chanlist[channel].ulist->nodes); */
	/* Decided against the above - although it's a grey area.  Goes back to
	the tree-falling-in-the-middle-of-the-desert thing.  Just because 
	no-one was there to witness it, doesn't mean it didn't happen.. */
	/*return(1);*/
	/* But Ahh.. */
	/* return(0); */
	/* No, not ahh! Shut up with your ahh! */
	return(1);
}


/*-| bchan |----------------------------------------------------------------*/

/* Command syntax..  
  bchan channel message */
int bchan(Cmdline *cmdline)
{
	int channel;
	char *message;
	char *sptr;
	char format[20];

	/* Using atoi - which means we can't trap errors.. */
	channel = str2channel(cmdline->param);
	
	if (channel == -1) {
		setreply(cmdline->userid, "500", H_BAD_CHANNEL);
		flush_cmd();
		return(0);
	}

	if (!finduser(cmdline->userid, chanlist[channel].ulist, FIND_ONLY))
		setreply(cmdline->userid, "511", H_NOTJOIN);

	/* Find start of message 'manually', as using sscanf proved to be too 
	  inefficient.  If only I could get strtol to work..  That does this
	  for you.  */
	message = cmdline->param; 		
  
	/* Skip the leading white space */
	while (isspace(*message)) ++message;
	/* Skip the digits */
	while (isdigit(*message)) ++message;
	/* Skip the obligitary space after the digit. */
	if (*message != ' ') {
		setreply(cmdline->userid, "500", H_BAD_PARAM); 
		flush_cmd();
		return(0);
	}
	++message;

	if (!broadcast(channel, message, cmdline)) {
	  setreply(cmdline->userid, "510", H_FAIL);
	  flush_cmd();
	  return(0);
	}

	setreply(cmdline->userid, "211", H_OK);
	flush_cmd();
	return(1);
}


/*-| synoff |---------------------------------------------------------------*/

int synoff(Cmdline *cmdline) {

	int channel;
	
	channel = str2channel(cmdline->param);
	
	if (channel < 0) {
	  setreply(cmdline->userid, "500", H_BAD_CHANNEL);
	  flush_cmd();
		return(0);
	}

	if (!deluser(cmdline->userid, channel)) {
		setreply(cmdline->userid, "520", H_BADUSR);
		flush_cmd();
		return(0);
	}
	
	setreply(cmdline->userid, "211", H_OK);
	flush_cmd();
	return(1);
}


/*-| join |-----------------------------------------------------------------*/

int join(Cmdline *cmdline) {

	int channel;
	
	channel = str2channel(cmdline->param);
	
	if ((channel < 0) || (channel > (CHANNELS -1))) {
		setreply(cmdline->userid, "500", H_BAD_CHANNEL);
		flush_cmd();
		return(0);
	}
	
	if (!adduser(cmdline->userid, channel)) {
		setreply(cmdline->userid, "510", H_FAIL);
		flush_cmd();
		return(0);
	}

	setreply(cmdline->userid, "211", H_OK);
	flush_cmd();
	return(1);
}


/*-| clist |----------------------------------------------------------------*/

int clist(Cmdline *cmdline) {

  int channel;
  Usernode *unode;
  
  channel = str2channel(cmdline->param);
  
  if (channel < 0) {
    setreply(cmdline->userid, "500", H_BAD_CHANNEL);
    flush_cmd();
    return(0);
  }

	if (chanlist[channel].ulist == NULL) {
	  /* The channel doesn't exist - it's empty.  Do nothing and report OK */
	  setreply(cmdline->userid, "211", H_OK);
	  flush_cmd();
	  return(1);
	}
  
  unode = chanlist[channel].ulist->first;
  
  while (unode != NULL) {
	  bprintf("%s\n", unode->userid);
	  unode = unode->next;
  }

 	setreply(cmdline->userid, "211", H_OK);
 	flush_cmd();
  return(1);
}


/*-| startup_rooms |--------------------------------------------------------*/

/* Spot the odd one out - Banana ) Raisin . Cauliflower (m) Apple o */
/* Answer:  .stiurf era srehto eht ,rewolfiluaC */

void startup_rooms()
{

	FILE *fp;
	char buf[MAXTOPICLEN+1];
	char format[20];
	int channel;
	
	if ((fp = fopen(CONFIG_FILE, "rt")) == NULL) {
		logerr("startup_rooms: Can't read soft config file", CONFIG_FILE);
		return;
	}
	
	while (fgets(buf, MAXTOPICLEN, fp) != NULL)
		if(buf[0] != '#')
			/* It's not a comment.. */
			channel = str2channel(buf);
			if (channel >= 0) {
				/* We got the channel.. */
				sprintf(format, "%%*d %%%dc", MAXTOPICLEN);
				if (sscanf(buf, format, buf)) {
					/* We've got the topic.. */
					strcpy(chanlist[channel].topic, buf);
					chanlist[channel].ulist = make_ulist();
				}
			}
}



/*-| module_init |----------------------------------------------------------*/

int module_init(void) 
/* TODO: initialise startup rooms from soft config file */
{
	int count;
	
	/* Announce our name to the world */
	module_name = MODULE_NAME;
  
	for (count=0; count<CHANNELS; ++count) {
		chanlist[count].ulist = NULL;
		chanlist[count].topic[0] = '\0';
	}
	
	/* Read in 'startup' rooms from soft config file */
	startup_rooms();
	return(1);
}

/*--------------------------------------------------------------------------*/

