# Copyright (C) 2001 Dominic Mitchell.  All rights reserved.  This
# program is free software; you can redistribute it and/or modify it
# under the same terms as Perl itself.

use strict;
use Test::Simple tests => 6;

# Override the default file layout, by sub classing.
package TestSetting;

use strict;
use vars qw(@ISA);

use Config::Setting;
use Config::Setting::FileProvider;

@ISA = qw( Config::Setting );

sub provider {
        my $self = shift;
        return Config::Setting::FileProvider->new(
               Env   => "TEST_SETTINGS_INI",
               Paths => [ "t/test.ini" ],
              );
}

package main;

# Test 1: Can we subclass ok?
my $stg = TestSetting->new;
ok( $stg, "Object creation" );

#---------------------------------------------------------------------
# From perlfaq4

sub compare_arrays {
        my ($first, $second) = @_;
        local $^W = 0;  # silence spurious -w undef complaints
        return 0 unless @$first == @$second;
        for (my $i = 0; $i < @$first; $i++) {
                return 0 if $first->[$i] ne $second->[$i];
        }
        return 1;
}

#---------------------------------------------------------------------

my @expected_sections = ( 'settings', 'other stuff' );
my @sections          = $stg->sections();
ok( compare_arrays(\@expected_sections, \@sections),
    "Correct list of sections");

my @expected_keys = sort qw(foo baz Ivor combined);
my @keys          = sort $stg->keylist('settings');
ok( compare_arrays(\@expected_keys, \@keys), "Correct list of keys" );

my $v = $stg->get("settings", "baz");
ok( $v eq "quux", "'baz' key has correct value" );

$v = $stg->get("settings", "foo");
ok( $v eq "bar, The Engine", "'bar' key has correct value" );

$v = $stg->get("settings", "combined");
ok($v eq "bar, The Engine quux ", "'combined' key has the right value");

# Local Variables:
# mode: cperl
# cperl-indent-level: 8
# indent-tabs-mode: nil
# cperl-continued-statement-offset: 8
# End:
#
# vim: ai et sw=8
