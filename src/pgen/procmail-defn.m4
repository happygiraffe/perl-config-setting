########################################################################
# procmail-defn.m4 - definitions for building a procmailrc file.
#
# $Id: procmail-defn.m4,v 1.1 1997/06/28 14:58:08 dom Exp $
########################################################################

#----------------------------------------------------------------------
# First, the all important m4 definitions.
#----------------------------------------------------------------------

# def. "MH-USER", if you are using MH type folders.
define(MH-USER)dnl
ifdef(MH-USER, 
`# MH-USER was defined.', 
`# MH-USER not defined, defaulting to mbox.')

# LIST(listname, folder) ; foldername w/o the "+".
define(LIST,
`ifdef(`INABLOCK',`	'):0: ifdef(`MH-USER',$2/.lock)
ifdef(`INABLOCK',`	')* (^TO$1|^Return-Path:.*$1)
ifdef(`INABLOCK',`	')ifdef(`MH-USER', `| rcvstore +$2', `$2')'
)dnl

# SUBJ(subject, folder) ; foldername w/o the "+".
define(SUBJ,
`ifdef(`INABLOCK',`	'):0: ifdef(MH-USER,$2/.lock)
ifdef(`INABLOCK',`	')* ^Subject:.*$1
ifdef(`INABLOCK',`	')ifdef(MH-USER, `| rcvstore +$2', `$2')'
)dnl

# FROM(from, folder) ; foldername w/o the "+".
define(FROM,
`ifdef(`INABLOCK',`	'):0: ifdef(MH-USER,$2/.lock)
ifdef(`INABLOCK',`	')* ^(From|Sender):.*$1
ifdef(`INABLOCK',`	')ifdef(MH-USER, `| rcvstore +$2', `$2')'
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
