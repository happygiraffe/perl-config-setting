#!/usr/bin/env python

"""
Implementation of <dtml-whois>.
"""

__rcs_id__='$Id: WhoisTag.py,v 1.1 2000/07/04 07:22:13 dom Exp $'
__version__='$Revision: 1.1 $'[11:-2]

from sys import stderr
from DocumentTemplate.DT_Util import *
from DocumentTemplate.DT_String import String
from whois import INTERNIC, Whois

class WhoisTag:
    """The whois tag, used like this:

    <dtml-whois name>

    You can optionally specify a server as 'server=whois.foo.net'.

    """

    name = 'whois'
    expr = None

    def __init__(self, args):
        # Parse the tags parameters.
        args = parse_params(args, name='', expr='', server=INTERNIC)

        if args.has_key('server'):
            self.server = args['server']

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
