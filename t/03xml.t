# Copyright (C) 2001 Dominic Mitchell.  All rights reserved.  This
# program is free software; you can redistribute it and/or modify it
# under the same terms as Perl itself.

use strict;
use Test;
use Data::Dumper;

BEGIN {
	plan tests => 6;
}

# Override the default file layout, by sub classing.
package TestSetting;

use strict;
use vars qw(@ISA);

use Config::Setting;
use Config::Setting::FileProvider;
use Config::Setting::XMLParser;

@ISA = qw(Config::Setting);

sub provider {
        my $self = shift;
        return Config::Setting::FileProvider->new(
               Env => "TEST_SETTINGS",
               Paths => [ "t/test.xml" ],
              );
}

sub parser {
        my $self = shift;
        return Config::Setting::XMLParser->new(@_);
}

package main;

# Test 1: Can we subclass ok?
my $stg = eval {
        TestSetting->new;
};
warn $@ if $@;
$stg ? ok(1) : ok(0);

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

# Test 2: Do we have the correct list of sections?
my @expected_sections = ( 'settings', 'other stuff' );
my @sections = $stg->sections();
ok( compare_arrays(\@expected_sections, \@sections) );

# Test 3: Do we have the correct list of keys?
my @expected_keys = sort qw(foo baz Ivor combined);
my @keys = sort $stg->keylist('settings');
ok( compare_arrays(\@expected_keys, \@keys) );

# Test 4: Does the baz key have the right value?
my $v = $stg->get("settings", "baz");
ok($v eq "quux");

# Test 5: Does the foo key have the right value?
$v = $stg->get("settings", "foo");
ok($v eq "bar, The Engine");

# Test 6: Does the combined key have the right value?
$v = $stg->get("settings", "combined");
ok($v eq "bar, The Engine quux ");

# Local Variables:
# mode: cperl
# cperl-indent-level: 8
# indent-tabs-mode: nil
# cperl-continued-statement-offset: 8
# End:
#
# vim: ai et sw=8
