# -*- makefile -*-
#
# Makefile to install TkClient 1.0
#
# $Header: /home/ncvs/src/tkclient/Makefile,v 1.2 1997/02/13 17:08:43 dom Exp $
#

VERSION = 1.0

LIBDIR  = /home/dom/work/tkclient-$(VERSION)
DESTDIR = $(HOME)/bin

all: $(DESTDIR)/tkclient
	wish -f $(DESTDIR)/tkclient

install:
	sed -e "s#@LIBDIR@#$(LIBDIR)#" < tkclient.in > $(DESTDIR)/tkclient
	chmod 755 $(DESTDIR)/tkclient

$(DESTDIR)/tkclient: tkclient.in

dist:
	@echo Making TkClient distribution
	cd .. ; tar -cf - tkclient-$(VERSION) | \
	gzip -9 > tkclient-$(VERSION).tar.gz

.PHONY: all install
