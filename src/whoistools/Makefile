#
# Makefile for whoistools
#
# @(#) $Id: Makefile,v 1.1 2000/09/05 11:25:03 dom Exp $
#

NAME=		whoistools
VERSION=	0.1
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

all: whoislist.py

whoislist.py: listmgr.py
	@python listmgr.py

dist:
	@rm -rf $(DIST)
	@mkdir $(DIST)
	@cp $(FILES) $(DIST)
	tar cvzf $(DIST).tar.gz $(DIST)
	@rm -rf $(DIST)