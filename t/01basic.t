# Copyright (C) 2001 Dominic Mitchell.  All rights reserved.  This
# program is free software; you can redistribute it and/or modify it
# under the same terms as Perl itself.

# @(#) $Id$

# Check that module loads ok.  This pulls in all the other modules so
# this is all that we need to test for right now.

use strict;
use Test::More tests => 1;

use_ok( 'Config::Setting' );

# vim: set ai et sw=8 syntax=perl :
