# Copyright (C) 2001 Dominic Mitchell.  All rights reserved.  This
# program is free software; you can redistribute it and/or modify it
# under the same terms as Perl itself.

# Check that module loads ok.  This pulls in all the other modules so
# this is all that we need to test for right now.

BEGIN { $| = 1; print "1..1\n"; }
END {print "not ok 1\n" unless $loaded;}
use Config::Setting;
$loaded = 1;
print "ok 1\n";

# Local Variables:
# mode: cperl
# cperl-indent-level: 8
# indent-tabs-mode: nil
# cperl-continued-statement-offset: 8
# End:
#
# vim: ai et sw=8