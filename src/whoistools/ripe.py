#!/usr/bin/env python

"""
Perform a ripe query.
"""

__rcs_id__='$Id: ripe.py,v 1.2 2000/07/04 10:11:55 dom Exp $'
__version__='$Revision: 1.2 $'[11:-2]

import sys
import string
import whois

#WHOISHOST = 'whois.ripe.net'
WHOISHOST = 'localhost'

def Ripe(query):
    """Perform a ripe query and return the result as a list of tuples."""

    res = []

    data = whois.Whois(query, WHOISHOST)
    data = string.split(data, '\n')
    currec = []
    for line in data:
        if not line:
            if len(currec) > 0:
                res.append(currec)
            currec = []
            continue
        if line[0] == '%': continue
        field, value = string.split(line, None, 1)
        field = field[:-1]
        currec.append( (field, value) ) # Make sure it's a tuple.

    return res

if __name__ == '__main__':
    try:
        from pprint import pprint
        objs = Ripe(sys.argv[1])
        #pprint(objs)
        for o in objs:
            print "%-13s %s" % (o[0][0] + ':', o[0][1])
    except IndexError:
        print "usage: ripe query"
