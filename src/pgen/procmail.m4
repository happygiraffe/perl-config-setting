########################################################################
# ~/.procmailrc - mail filtering file.
#
# @(#) $Id: procmail.m4,v 1.2 2001/06/15 23:37:53 dom Exp $
########################################################################

#----------------------------------------------------------------------
# Initial procmail definitions.
#----------------------------------------------------------------------

PATH=		$HOME/bin:/usr/bin:/bin:/usr/local/bin:/usr/local/libexec/nmh
MAILDIR =	$HOME/mail	# You'd better make sure it exists
DEFAULT =	/var/mail/$LOGNAME
LOGFILE =	$MAILDIR/mail.log
VERBOSE =	off
COMSAT=		no
SENDMAIL=	/usr/sbin/sendmail
LOGABSTRACT=	all

#----------------------------------------------------------------------
# eliminate duplicate messages.
#----------------------------------------------------------------------
STOPDUPS

#----------------------------------------------------------------------
# The killfile...
#----------------------------------------------------------------------

# Need to specify a dummy folder for locking purposes.
FROM(a_spammer, nothing, JUNK) 

#----------------------------------------------------------------------
# File mailing lists away.
#----------------------------------------------------------------------

LIST(announce.*freebsd.org, lists/announce)
LIST(advocacy.*freebsd.org, lists/advocacy)
LIST(chat.*freebsd.org, lists/chat)
LIST(current.*freebsd.org, lists/current)
LIST(hackers.*freebsd.org, lists/hackers)
LIST(mobile.*freebsd.org, lists/mobile)

#----------------------------------------------------------------------
# File stuff sent specifically to one of my mailboxes correctly
#----------------------------------------------------------------------

ARG(pgen, pgen)

#----------------------------------------------------------------------
# Send the remaining mail to my inbox.
#----------------------------------------------------------------------
