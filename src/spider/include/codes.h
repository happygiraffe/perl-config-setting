/*********************************************************************
 * codes.h
 *
 * Standard definitions for reply codes and messages for use with
 * Spider.
 *
 * Copyright 1996 Dominic Mitchell (dom@myrddin.demon.co.uk)
 *
 * @(#) $Id: codes.h,v 1.1 2000/01/06 21:55:19 dom Exp $
 */

#ifndef _CODES_H_
#define _CODES_H_

/* needed for VERSION */
#include <config.h>             /* autoconf */

/* Startup line */
#define INITIAL_GREETING "Spider " VERSION

/* Standard reply format */
#define REPLY_FMT	"%03d %s"
/* Length of reply code & following space */
#define REPLY_CODE_LEN	4
/* Standard end-of-data mark */
#define END_OF_DATA	"."

#define INF_HELP	110
#define INF_HELP_MSG	"Known commands are:"

#define OK_CMD		211
#define OK_CMD_MSG	"Command accepted."
#define OK_HELLO	220
#define OK_HELLO_MSG	"Welcome to Spider."
#define OK_BYE		221
#define OK_BYE_MSG	"Goodbye.  Have a nice day."

#define OK_UNSOLIC	280
#define OK_UNSOLIC_MSG	"Unsolicited message:"

#define ERR_NOTNOW	420
#define ERR_NOTNOW_MSG	"Try again later - service not available now"

#define ERR_NOTCMD	500
#define ERR_NOTCMD_MSG	"Not a valid command"
#define ERR_SYNTAX	501
#define ERR_SYNTAX_MSG	"Bad syntax in command"
#define ERR_BADNAME	502
#define ERR_BADNAME_MSG "Invalid characters in name" 
#define ERR_PREVDEF	510
#define ERR_PREVDEF_MSG	"Command already defined"
#define ERR_NOTJOIN	511
#define ERR_NOTJOIN_MSG	"Not a member"
#define ERR_BADUSR	520
#define ERR_BADUSR_MSG	"Bad username"
#define ERR_BADPWD	521
#define ERR_BADPWD_MSG	"Bad password"
#define ERR_TOOMANY	522
#define ERR_TOOMANY_MSG	"Already logged on - only 1 logon allowed"
#define ERR_ILOGIN	523
#define ERR_ILOGIN_MSG	"LOGIN must be the initial command"
#define ERR_LOGIN	524
#define ERR_LOGIN_MSG	"LOGIN can only be used as the initial command"

#endif /* _CODES_H_ */

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * comment-column: 32
 * End:
 */
