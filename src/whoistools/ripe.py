#!/usr/bin/env python

"""
Perform a ripe query.
"""

__rcs_id__='$Id: ripe.py,v 1.3 2000/07/04 10:44:30 dom Exp $'
__version__='$Revision: 1.3 $'[11:-2]

import sys
import string
import whois

from types import ListType, TupleType

#WHOISHOST = 'whois.ripe.net'
WHOISHOST = 'localhost'

class RipeObj:
    """Representation of a RIPE object."""

    def __init__(self, data):
        """Create a new Ripe Object.

        The data must be a list of tuples, each of length two.

        """

        # Check that what we have is formatted correctly.
        assert type(data) == ListType
        for rec in data:
            assert len(rec) == 2

        self.data = data
        return

    def __getattr__(self, val):
        res = []
        for o, a in self.data:
            if o == val: res.append(val)
        return res

    def __str__(self):
        res = ''
        for o, a in self.data:
            res = res + '%-13s %s\n' % (o+':', a)
        return res[:-1]                 # Strip trailing NL.

    def id(self):
        return self.data[0][1]

def RipeQuery(query):
    """Perform a ripe query and return the result as a list of tuples."""

    res = []

    data = whois.Whois(query, WHOISHOST)
    data = string.split(data, '\n')
    currec = []
    for line in data:
        if not line:
            if len(currec) > 0:
                currec = RipeObj(currec)
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
        objs = RipeQuery(sys.argv[1])
        for o in objs:
            print o
            print
    except IndexError:
        print "usage: ripe query"
