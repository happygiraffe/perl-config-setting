#!/usr/bin/env python

"""
Perform a ripe query.
"""

__rcs_id__='$Id: ripe.py,v 1.5 2000/07/04 11:37:17 dom Exp $'
__version__='$Revision: 1.5 $'[11:-2]

import sys
import string
import whois

from types import ListType, TupleType, StringType

#WHOISHOST = 'whois.ripe.net'
WHOISHOST = 'localhost'

class RipeObj:
    """Representation of a RIPE object."""

    def __init__(self, data):
        """Create a new Ripe Object.

        The data must be a list of tuples, each of length two.

        """

        # Check that what we have is formatted correctly.
        self.__checkdata(data)
        self.data = data
        return

    def __checkdata(self, data):
        assert type(data) == ListType
        for rec in data:
            assert len(rec) == 2

    def __getattr__(self, val):
        res = []
        for o, a in self.data:
            if o == val: res.append(a)
        return res

    def __setattr__(self, key, val):
        if key == "data":               # Special case.
            self.__checkdata(val)
            self.__dict__[key] = val
            return

        # We walk through our list of data and replace all the old
        # keys with the new one(s).  That means that multiple keys
        # will be replaced by a single one if that's what is given to
        # us.

        # For consistency
        if type(val) == StringType: val = [val]
        
        done=0
        newdata = []
        for o, a in self.data:
            if o != key:
                newdata.append( (o, a) )
                continue
            if val:
                done=1
                newdata.append( (o, val[0]) )
                del val[0]
        if not done:
            newdata.append( (key, val[0]) )
        self.data = newdata
        return

    def __str__(self):
        res = ''
        for o, a in self.data:
            res = res + '%-13s %s\n' % (o+':', a)
        return res[:-1]                 # Strip trailing NL.

    def name(self):
        return self.data[0][1]

class inetnum(RipeObj):
    def name(self):
        return self.netname

class person(RipeObj):
    def name(self):
        return getattr(self, 'nic-hdl')[0]

class role(RipeObj):
    def name(self):
        return getattr(self, 'nic-hdl')[0]

class mntner(RipeObj):
    def id(self):
        return self.mntner

def RipeObjFactory(data):
    if   data[0][0] == 'inetnum': return inetnum(data)
    elif data[0][0] == 'person':  return person(data)
    elif data[0][0] == 'role':    return role(data)
    elif data[0][0] == 'mntner':  return mntner(data)
    else:                         return RipeObj(data)

def RipeQuery(query):
    """Perform a ripe query and return the result as a list of tuples."""

    res = []

    data = whois.Whois(query, WHOISHOST)
    data = string.split(data, '\n')
    currec = []
    for line in data:
        if not line:
            if len(currec) > 0:
                currec = RipeObjFactory(currec)
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
            o.remarks = 'This is a comment.'
            setattr(o, 'admin-c', 'SOMEBODY-ELSE')
            setattr(o, 'tech-c', ['FOO', 'BAR', 'BAZ'])
            print o
            print
    except IndexError:
        print "usage: ripe query"
