#
# Makefile for whoistools
#
# @(#) $Id: Makefile,v 1.2 2000/09/05 21:46:03 dom Exp $
#

NAME=		whoistools
VERSION=	0.2
DIST=		$(NAME)-$(VERSION)

FILES=						\
	Makefile				\
	README					\
	WhoisTag.py				\
	__init__.py				\
	asyncwhois.py				\
	listmgr.py				\
	ripe.py					\
	whois.py

all: whoislist.py compile

whoislist.py: listmgr.py
	@python listmgr.py

compile:
	@python -c 'from compileall import compile_dir;compile_dir(".")'

dist:
	@rm -rf $(DIST)
	@mkdir $(DIST)
	@cp $(FILES) $(DIST)
	tar cvzf $(DIST).tar.gz $(DIST)
	@rm -rf $(DIST)

clean:
	rm -f *.pyc