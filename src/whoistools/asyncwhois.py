#!/usr/bin/env python

"""
Asynchronous Python Whois client.

Originally by Fredrik Lundh <effbot@telia.com>

This probably needs fiddling with before it works...
"""

__rcs_id__='$Id: asyncwhois.py,v 1.1 2000/09/05 11:03:05 dom Exp $'
__version__='$Revision: 1.1 $'[11:-2] 

import os
import sys
import asyncore
import socket
import string
import whois

class WhoisRequest(asyncore.dispatcher_with_send):
    # simple whois requestor

    def __init__(self, consumer, query, host, port=43):
        asyncore.dispatcher_with_send.__init__(self)

        self.consumer = consumer
        self.query = query

        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connect((host, port))

    def handle_connect(self):
        self.send(self.query + "\r\n")

    def handle_expt(self):
        self.close() # connection failed, shutdown
        self.consumer.abort()

    def handle_read(self):
        # get data from server
        self.consumer.feed(self.recv(2048))

    def handle_close(self):
        self.close()
        self.consumer.close()

class WhoisConsumer:

    def __init__(self, host):
        self.text = ""
        self.host = host
	self.srv = whois.Server(host)
	if os.path.exists(host): os.rename(host, host + '.old')
	self.fd = open(host, "w")

    def feed(self, text):
        self.text = self.text + text

    def abort(self):
        print self.host, "=>", "failed"

    def close(self):
	if self.srv == whois.INTERNIC:
	    # Further parsing, and another request.  If it returns
	    # something, then we don't finish off here.  XXX Should
	    # probably be a subclass...
	    if self.digdeeper(): return
	self.fd.write(self.text)
	self.fd.close()
	nextHost()

    def digdeeper(self):
	srv = ''
	words = string.split(self.text)
	for i in range(len(words) - 2):
	    if words[i] == 'Whois' and words[i+1] == 'Server:':
		srv = words[i+2]
		break
	if srv:		# Reset & try again.
	    self.text = ''
	    self.srv = srv
	    WhoisRequest(self, self.host, srv)
	return srv

def nextHost():
    """Process the next request in the queue."""
    if hosts:
	host = hosts[0]
        del hosts[0]
	if os.path.exists(host): 
	    nextHost()
	else:
	    WhoisRequest(WhoisConsumer(host), host, whois.Server(host))

hosts = []
for line in sys.stdin.readlines():
    hosts.append(string.rstrip(line))

for i in range(32):		# Try to keep 30 requests on the go.
    nextHost()

# loop returns when all requests have been processed
asyncore.loop()
