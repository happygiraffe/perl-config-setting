#
# build mod_sed using apxs.
#
# @(#) $Id: Makefile,v 1.1 2001/06/03 11:56:56 dom Exp $
#

APXS=	/usr/local/sbin/apxs
DSO=	mod_sed.so
APXSFLAGS= -S CFLAGS="-funsigned-char -g -pipe -march=pentiumpro"

SRCS=	mod_sed.c compile.c main.c misc.c process.c
OBJS=	$(SRCS:.c=.o)
HDRS=	extern.h defs.h

all: $(DSO)

install: all
	@$(APXS) -i $(DSO)

test:
	echo "break sed_handler" > .gdbinit
	LD_BIND_NOW=1 httpd -X -f $(PWD)/httpd.conf -DSED & echo $$!
#	gdb /usr/local/sbin/httpd $$!

$(DSO): $(SRCS) $(HDRS)
	@$(APXS) -c $(APXSFLAGS) $(SRCS)

depend:

clean:
	rm -f $(DSO) $(OBJS)

tags:
	etags $(SRCS) $(HDRS)
