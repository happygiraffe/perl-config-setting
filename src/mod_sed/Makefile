#
# build mod_sed using apxs.
#
# @(#) $Id: Makefile,v 1.7 2001/06/22 21:46:04 dom Exp $
#

#---------------------------------------------------------------------
# User configurable parts here.

# Where does apxs live?  Change if it's not in your PATH.
APXS=		apxs

# Uncomment if you don't have fgetln(3) on your system (pretty likely
# unless you're running *BSD).
#FGETLN= 	fgetln.c

# End of user configurable parts.
#---------------------------------------------------------------------

DSO=		$(NAME).so
#APXSFLAGS= 	-S CFLAGS="-funsigned-char -g -pipe -march=pentiumpro"

SRCS=		mod_sed.c compile.c main.c misc.c process.c $(FGETLN)
OBJS=		$(SRCS:.c=.o)
HDRS=		extern.h defs.h

VERSION=	0.2
NAME=		mod_sed
DISTNAME=	$(NAME)-$(VERSION)

all: $(DSO)

install: all
	@$(APXS) -i $(DSO)

test:
	echo "break sed_handler" > .gdbinit
	LD_BIND_NOW=1 httpd -X -f $(PWD)/httpd.conf -DSED & echo $$!
#	gdb /usr/local/sbin/httpd $$!

SED_SRC=	/usr/src/usr.bin/sed
DIFF_FLAGS=	-u
# XXX In BSD make, if you take the "-" away from the beginning of the
# line, the damned thing stops after the first diff!  It's obviously
# picking up the exit code from the diff directly, somehow... 
diff:
	@-for f in $(SRCS) $(HDRS) ; do \
		if [ -f $(SED_SRC)/$$f ] ; then \
			diff $(DIFF_FLAGS) $(SED_SRC)/$$f $$f ; \
		fi ; \
	done ; true

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
