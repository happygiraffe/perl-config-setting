#!/usr/bin/env python

"""Manage the whoislist database.

This program will download, decompress and store the list of whois
servers for TLDs.  It will then create a python module from it.
"""

import os,sys
import urllib
from gzip import GzipFile
import string
from cStringIO import StringIO

#-----------------------------------------------------------------------

__rcs_id__='$Id: listmgr.py,v 1.1 2000/09/05 11:00:04 dom Exp $'
__version__='$Revision: 1.1 $'[11:-2]

WHOISLISTURL = 'http://www.geektools.com/dist/whoislist.gz'

WHOISLISTMOD = 'whoislist.py'

#-----------------------------------------------------------------------

def main():
    # Prepare the python module we create.
    whlmod = open(WHOISLISTMOD, 'w')
    whlmod.write("""# Automatically Generated --- DO NOT EDIT

servers = {
""")

    # Download the file and read it line by line.
    print "Downloading %s ..." % WHOISLISTURL
    whlgz = urllib.urlopen(WHOISLISTURL)
    # Hack to get a seekable file.
    whlgz = StringIO(whlgz.read())
    print "Extracting %s ..." % os.path.basename(WHOISLISTURL)
    whl = GzipFile(None, None, None, whlgz)
    print "Creating %s ..." % WHOISLISTMOD
    while 1:
        line = whl.readline()
        if not line: break              # EOF
        line = string.rstrip(line)
        fields = string.split(line, '|')
        if fields[1] == 'NONE' or fields[1] == 'WEB':
            host = None
        else:
            host = "'%s'" % fields[1]
        tld = "." + fields[0]
        whlmod.write("    '%s': %s,\n" % (tld, host))
    whlmod.write("}\n")
    whlmod.close()

#-----------------------------------------------------------------------

if __name__ == '__main__':
    main()
