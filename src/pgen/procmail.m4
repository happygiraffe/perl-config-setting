########################################################################
# ~/.procmailrc - mail filtering file.
#
# @(#) $Id: procmail.m4,v 1.1 2001/05/26 17:23:58 dom Exp $
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
ARG=		$1
SENDMAIL=	/usr/sbin/sendmail
LOGABSTRACT=	all

#----------------------------------------------------------------------
# eliminate duplicate messages.
#----------------------------------------------------------------------
:0 Wh: msgid.lock
| formail -D 32768 msgid.cache

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
# Send the remaining mail to my inbox.
#----------------------------------------------------------------------
