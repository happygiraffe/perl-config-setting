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

__rcs_id__='$Id: listmgr.py,v 1.3 2000/09/05 20:41:47 dom Exp $'
__version__='$Revision: 1.3 $'[11:-2]

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

# vim: ai et sw=4
