# Copyright (C) 2001 Dominic Mitchell.  All rights reserved.  This
# program is free software; you can redistribute it and/or modify it
# under the same terms as Perl itself.

# @(#) $Id$

use strict;
use Test::More tests => 6;

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
isa_ok( $stg, 'TestSetting' );

#---------------------------------------------------------------------

my @expected_sections = ( 'settings', 'other stuff' );
my @sections          = $stg->sections();
is_deeply( \@expected_sections, \@sections, "Correct list of sections");

my @expected_keys = sort qw(foo baz Ivor combined);
my @keys          = sort $stg->keylist('settings');
is_deeply( \@expected_keys, \@keys, "Correct list of keys" );

my $v = $stg->get("settings", "baz");
is( $v, "quux", "'baz' key has correct value" );

$v = $stg->get("settings", "foo");
is( $v, "bar, The Engine", "'bar' key has correct value" );

$v = $stg->get("settings", "combined");
is( $v, "bar, The Engine quux ", "'combined' key has the right value");

# vim: set ai et sw=8 syntax=perl :
