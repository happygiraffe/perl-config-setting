divert(-1)

# Copyright (c) 2001 Dominic Mitchell.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

divert(0)dnl
########################################################################
# procmail-defn.m4 - definitions for building a procmailrc file.
#
# $Id: procmail-defn.m4,v 1.6 2001/06/22 23:58:52 dom Exp $
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
