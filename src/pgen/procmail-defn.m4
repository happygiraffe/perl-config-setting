########################################################################
# procmail-defn.m4 - definitions for building a procmailrc file.
#
# $Id: procmail-defn.m4,v 1.4 2001/05/26 17:43:38 dom Exp $
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

divert(-1)

# LIST(listname, folder [, type]) ; foldername w/o the "+".
define(LIST,
`ifdef(`INABLOCK',`	'):0: ifdef(`MH-USER',$2/.lock)
ifdef(`INABLOCK',`	')* (^TO$1|^Return-Path:.*$1)
ifdef(`INABLOCK',`	')ifelse($3,`JUNK',`/dev/null',
				$3, `RCVSTORE', `| rcvstore +$2',
				$3, `MH', `$2/.',
				$3, `MBOX', `$2',
				ifdef(`MH-USER', `| rcvstore +')$2)'
)dnl

# SUBJ(subject, folder [, type]) ; foldername w/o the "+".
define(SUBJ,
`ifdef(`INABLOCK',`	'):0: ifdef(MH-USER,$2/.lock)
ifdef(`INABLOCK',`	')* ^Subject:.*$1
ifdef(`INABLOCK',`	')ifelse($3,`JUNK',`/dev/null',
				$3, `RCVSTORE', `| rcvstore +$2',
				$3, `MH', `$2/.',
				$3, `MBOX', `$2',
				ifdef(`MH-USER', `| rcvstore +')$2)'
)dnl

# FROM(from, folder [, type]) ; foldername w/o the "+".
define(FROM,
`ifdef(`INABLOCK',`	'):0: ifdef(MH-USER,$2/.lock)
ifdef(`INABLOCK',`	')* ^(From|Sender):.*$1
ifdef(`INABLOCK',`	')ifelse($3,`JUNK',`/dev/null',
				$3, `RCVSTORE', `| rcvstore +$2',
				$3, `MH', `$2/.',
				$3, `MBOX', `$2',
				ifdef(`MH-USER', `| rcvstore +')$2)'
)dnl

# ARG(arg, folder [, type]) ; foldername w/o the "+".
define(ARG,
`ifdef(`INABLOCK',`	'):0: ifdef(MH-USER,$2/.lock)
ifdef(`INABLOCK',`	')* `$ARG' ?? $1
ifdef(`INABLOCK',`	')ifelse($3,`JUNK',`/dev/null',
				$3, `RCVSTORE', `| rcvstore +$2',
				$3, `MH', `$2/.',
				$3, `MBOX', `$2',
				ifdef(`MH-USER', `| rcvstore +')$2)'
)dnl

# START a blocked section.
define(`START',
`define(`INABLOCK')dnl
:0
* (^TO$1|^Return-Path:.*$1)
{'
)dnl

# END a blocked section
define(`END',
`undefine(`INABLOCK')dnl
}'
)dnl

########################################################################
# End of procmail-defn.m4
########################################################################
divert(0)dnl
