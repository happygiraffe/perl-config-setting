#!/usr/bin/env python

"""
Python Whois tools.

Server(domain)	- Return an appropriate whois server for a domain.
Whois(domain, server=None)	- Return whois output for domain.
"""

__rcs_id__='$Id: whois.py,v 1.3 2000/07/04 10:12:35 dom Exp $'
__version__='$Revision: 1.3 $'[11:-2]

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

    if os.path.exists(whoislist):
        for line in open(whoislist).readlines():
            line = string.rstrip(line)
            fields = string.split(line, '|')
            if fields[1] == 'NONE': fields[1] = None
            if fields[1] == 'WEB': fields[1] = None
            tld = "." + fields[0]
            servers[tld] = fields[1]
    else:
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
