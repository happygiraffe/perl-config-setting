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
Implementation of <dtml-whois>.
"""

__rcs_id__='$Id: WhoisTag.py,v 1.5 2000/09/05 21:02:05 dom Exp $'
__version__='$Revision: 1.5 $'[11:-2]

from sys import stderr
from DocumentTemplate.DT_Util import *
from DocumentTemplate.DT_String import String
from whois import Whois

class WhoisTag:
    """The whois tag, used like this:

    <dtml-whois name>

    You can optionally specify a server as 'server=whois.foo.net'.

    """

    name = 'whois'
    expr = None

    def __init__(self, args):
        # Parse the tags parameters.
        args = parse_params(args, name='', expr='', server=None)

        if args.has_key('server'):
            self.server = args['server']
        else:
            self.server = None

        # Try to sort out whether we have an expression to be
        # evaluated, or a simple name...
        self.__name__, self.expr = name_param(args, 'whois', 1)

    def render(self, md):
        # Boilerplate?
        name = self.__name__
        val = self.expr
        if val is None:
            val = md[name]
        else:
            val = val.eval(md)
        return "<pre>" + Whois(val, self.server) + "</pre>"

    __call__ = render

String.commands['whois'] = WhoisTag

# vim: ai et sw=4
