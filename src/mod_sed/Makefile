#
# build mod_sed using apxs.
#
# @(#) $Id: Makefile,v 1.2 2001/06/14 07:06:12 dom Exp $
#

APXS=		/usr/local/sbin/apxs
DSO=		$(NAME).so
#APXSFLAGS= 	-S CFLAGS="-funsigned-char -g -pipe -march=pentiumpro"

SRCS=		mod_sed.c compile.c main.c misc.c process.c
OBJS=		$(SRCS:.c=.o)
HDRS=		extern.h defs.h

VERSION=	0.1
NAME=		mod_sed
DISTNAME=	$(NAME)-$(VERSION)

all: $(DSO)

install: all
	@$(APXS) -i $(DSO)

test:
	echo "break sed_handler" > .gdbinit
	LD_BIND_NOW=1 httpd -X -f $(PWD)/httpd.conf -DSED & echo $$!
#	gdb /usr/local/sbin/httpd $$!

dist:
	mkdir $(DISTNAME)
	tar cf - `cat MANIFEST` | (cd $(DISTNAME); tar xf -)
	tar cvzf $(DISTNAME).tar.gz $(DISTNAME)
	rm -rf $(DISTNAME)

$(DSO): $(SRCS) $(HDRS)
	@$(APXS) -c $(APXSFLAGS) $(SRCS)

depend:

clean:
	rm -f $(DSO) $(OBJS)

tags:
	etags $(SRCS) $(HDRS)
