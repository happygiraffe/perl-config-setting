########################################################################
# procmail-defn.m4 - definitions for building a procmailrc file.
#
# $Id: procmail-defn.m4,v 1.5 2001/06/15 23:37:38 dom Exp $
########################################################################

`# This procmail.rc was built on' syscmd(date)dnl

divert(-1)

#----------------------------------------------------------------------
# First, the all important m4 definitions.
#----------------------------------------------------------------------

# def. "MH-USER", if you are using MH type folders.
divert(0)dnl
ifdef(MH-USER, 
`# MH-USER was defined.', 
`# MH-USER not defined, defaulting to mbox.')

# Save initial argument for later use.
SAVARG = $1

divert(-1)

# STOPDUPS
define(STOPDUPS,
`:0 Wh: msgid.lock
| formail -D 32768 msgid.cache')


# LIST(listname, folder [, type]) ; foldername w/o the "+".
define(LIST,
`ifdef(`INABLOCK',`	'):0: ifdef(`MH-USER',$2/.lock)
ifdef(`INABLOCK',`	')* (^TO$1|^Return-Path:.*$1)
ifdef(`INABLOCK',`	')ifelse($3,`JUNK',`/dev/null',
				$3, `RCVSTORE', `| rcvstore +$2',
				$3, `MH', `$2/.',
				$3, `MBOX', `$2',
				ifdef(`MH-USER', `| rcvstore +')$2)'
)

# SUBJ(subject, folder [, type]) ; foldername w/o the "+".
define(SUBJ,
`ifdef(`INABLOCK',`	'):0: ifdef(MH-USER,$2/.lock)
ifdef(`INABLOCK',`	')* ^Subject:.*$1
ifdef(`INABLOCK',`	')ifelse($3,`JUNK',`/dev/null',
				$3, `RCVSTORE', `| rcvstore +$2',
				$3, `MH', `$2/.',
				$3, `MBOX', `$2',
				ifdef(`MH-USER', `| rcvstore +')$2)'
)

# FROM(from, folder [, type]) ; foldername w/o the "+".
define(FROM,
`ifdef(`INABLOCK',`	'):0: ifdef(MH-USER,$2/.lock)
ifdef(`INABLOCK',`	')* ^(From|Sender):.*$1
ifdef(`INABLOCK',`	')ifelse($3,`JUNK',`/dev/null',
				$3, `RCVSTORE', `| rcvstore +$2',
				$3, `MH', `$2/.',
				$3, `MBOX', `$2',
				ifdef(`MH-USER', `| rcvstore +')$2)'
)

# ARG(arg, folder [, type]) ; foldername w/o the "+".
define(ARG,
`ifdef(`INABLOCK',`	'):0: ifdef(MH-USER,$2/.lock)
ifdef(`INABLOCK',`	')* `$SAVARG' ?? $1
ifdef(`INABLOCK',`	')ifelse($3,`JUNK',`/dev/null',
				$3, `RCVSTORE', `| rcvstore +$2',
				$3, `MH', `$2/.',
				$3, `MBOX', `$2',
				ifdef(`MH-USER', `| rcvstore +')$2)'
)

# START a blocked section.
define(`START',
`define(`INABLOCK')dnl
:0
* (^TO$1|^Return-Path:.*$1)
{'
)

# END a blocked section
define(`END',
`undefine(`INABLOCK')dnl
}'
)

########################################################################
# End of procmail-defn.m4
########################################################################
divert(0)dnl
