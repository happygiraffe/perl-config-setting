#!/usr/bin/env python
#
# Copyright 2000 Dominic Mitchell <dom@happygiraffe.net>
# All rights reserved.
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
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
# OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

"""
Python Whois tools.

Server(domain)	- Return an appropriate whois server for a domain.
Whois(domain, server=None)	- Return whois output for domain.
"""

__rcs_id__='$Id: whois.py,v 1.5 2000/09/05 11:05:33 dom Exp $'
__version__='$Revision: 1.5 $'[11:-2]

import os
import sys
import string
import socket

# Obtain full list from http://www.geektools.com/dist/whoislist.gz.

whoislist = "./whoislist"

INTERNIC = "whois.crsnic.net"

# In case whoislist isn't present.
defaultservers = {
    '.com':	INTERNIC,
    '.net':	INTERNIC,
    '.org':	INTERNIC,
    '.edu':	INTERNIC,
    '.uk':	'whois.nic.uk',
}
servers = {}

def Server(domain):
    """Return the whois server for domain."""

    server = INTERNIC			# Default.
    for tld in servers.keys():
	l = len(tld)
	if domain[-l:] == tld:
	    server = servers[tld]
	    break
    
    return server

def Whois(domain, server=None):
    """Return the whois output for a domain."""

    if server == None:
        server = Server(domain)

    server = socket.gethostbyname(server)
    port = socket.getservbyname('whois', 'tcp')
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(server, port)

    # The protocol itself.
    sock.send(domain + '\r\n')
    sock.shutdown(1)                    # No more sends.
    data = ''
    while 1:
        newdata = sock.recv(16384)
        if not newdata: break
        data = data + newdata
    sock.close()
    # Zap any CR's we see.
    if string.find(data, '\r') >= 0:
        data = string.join(string.split(data, '\r'), '')
    return data

def WhoisList(domain, server=None):
    """Return the whois output for a domain, as a list of lines."""
    data = Whois(domain, server)
    if string.find(data, '\r') >= 0:
        data = string.join(string.split(data, '\r'), '')
    data = string.split(data, '\n')
    # Tidying.
    if data[-1] == '': del data[-1]
    return data

def _init():
    """Initialise the servers dict from a file."""
    global servers

    try:
        # To create this module, run listmgr.py
        from whoislist import servers
    except ImportError:
        servers = defaultservers

# Call when module is first imported.
_init()

if __name__ == '__main__':
    try:
        from pprint import pprint
        name = sys.argv[1]
        dom = sys.argv[2]
        print "%% whois -h %s '%s'" % (dom, name)
        pprint(WhoisList(name, dom))
    except IndexError:
	print "usage: whoisserver.py name domain"
