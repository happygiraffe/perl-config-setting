# Copyright (C) 2001 Dominic Mitchell.  All rights reserved.  This
# program is free software; you can redistribute it and/or modify it
# under the same terms as Perl itself.

=pod

=head1 NAME

Config::Setting - Perl extension for configuration files.

=head1 SYNOPSIS

  use Config::Setting;
  my $stg = Config::Setting->new;
  $stg->get("section", "key");

=head1 DESCRIPTION

This module provides an OO interface to a file full of settings.
Settings are assumed to be contained in collections (known as
"sections").  Each setting has a key and a value. The value of a
setting may refer to other settings using a similiar syntax to
variables in perl.

Whilst this module can be used directly it is anticipated that it will
be subclassed.  This way policy regarding the location and layout of
the settings can be determined for your project.

=head1 METHODS

=over 4

=item new ( )

The constructor.  Takes no arguments.

=item provider ( )

Returns an object which can be used to collect the contents of files.
The default returns a Setting::FileProvider object.  You probably want
to override this method when you set up your subclass, in order to set
the policy for file locations.

=item parser ( STRING )

Returns an object which can parse the contents of STRING.  The default
is the Setting::IniParser object.  You may want to override this in a
subclass if you wish to use an alternative layout.

=item sections ( )

Return a list of which sections are available from this object.

=item keylist ( SECTION )

Return a list of keys that SECTION contains.

=item has ( SECTION, KEY )

Returns true if SECTION contains KEY.

=item expand ( )

Internal use only.

=item get ( SECTION, KEY )

Return the value of KEY in SECTION.  If the value contains any
variables of the form ${word}, they will be fully expanded in the
return value.

When trying to replace a variable "word", first, "word" will be looked
up as a key in the current section.  If not found, it will then be
looked up sequentially in all the other sections.  If still not found,
it will be replaced with an empty string.

Expansion is recursive, so an expanded variable can contain other
variables.

=back

=head1 AUTHOR

Dominic Mitchell, E<lt>dom@happygiraffe.netE<gt>

=head1 SEE ALSO

Config::Setting::FileProvider(3),
Config::Setting::IniParser(3),
Config::Setting::XMLParser(3).

=cut

package Config::Setting;

use strict;
use vars qw($VERSION $rcsid);

use Carp;
use Config::Setting::IniParser;
use Config::Setting::FileProvider;

$VERSION = '0.03';
$rcsid = '@(#) $Id: Setting.pm,v 1.4 2002/02/04 08:41:09 dom Exp $ ';

sub new {
        my ($proto) = @_;

        my $class = ref($proto) || $proto;
        my $self = {
                Config => { },
        };

        bless $self, $class;
        return $self->_init;
}

#---------------------------------------------------------------------
# These two functions are defaults and may be overridden

sub provider {
        my $self = shift;
        return Config::Setting::FileProvider->new(@_);
}

sub parser {
        my $self = shift;
        return Config::Setting::IniParser->new(@_);
}

#---------------------------------------------------------------------

sub _init {
        my $self = shift;
        my $provider = $self->provider;

        my @txts = $provider->provide();
        my @configs;
        foreach my $s (@txts) {
                my $p = $self->parser(String => $s);
                push @configs, $p;
        }

        return $self->_merge(@configs);
}

# Make up a combined configuration from all the ones provided.
# NB: Must maintain order of sections!
sub _merge {
        my $self = shift;
        my @configs = @_;
        my $cf = { };           # Combined config.
        my $sections = [ ];

        foreach my $c (@configs) {
                foreach my $s ($c->sections) {
                        unless (exists $cf->{$s}) {
                                $cf->{$s} = {};
                                push @$sections, $s;
                        }
                        foreach my $k ($c->keylist($s)) {
                                $cf->{$s}{$k} = $c->get($s, $k);
                        }
                }
        }
        $self->{Sections} = $sections;
        $self->{Config} = $cf;
        return $self;
}

#---------------------------------------------------------------------
# Data access...

sub sections {
        my $self = shift;
        return @{ $self->{Sections} };
}

sub keylist {
        my $self = shift;
        my ($section) = @_;
        croak "usage: Config::Setting->keylist(section)"
                unless $section;
        return keys %{ $self->{Config}{$section} };
}

sub has {
        my $self = shift;
        my ($section, $key) = @_;
        croak "usage: Config::Setting->get(section,key)"
                unless $section && $key;

        return exists $self->{Config}{$section}{$key};
}

# Get the value of a setting, searching all sections, but starting in
# the section specified.  May also specify a key that cannot be expanded.
# Internal.
sub expand {
        my $self = shift;
        my ($section, $key, $origkey) = @_;
        croak "usage: expand(section,key,origkey)"
                unless $section && $key && $origkey;

        # Move our section to the top of the list.
        my @sections = ($section, grep { $_ ne $section} $self->sections);

        return undef
                if $key eq $origkey;

        foreach my $s (@sections) {
                return $self->get($s, $key)
                        if $self->has($s, $key);
        }

        return undef;
}

# Return the value of a setting, fully expanded.
sub get {
        my $self = shift;
        my ($section, $key) = @_;
        croak "usage: Config::Setting->get(section,key)"
                unless $section && $key;

        my $val = $self->{Config}{$section}{$key};
        while ($val && $val =~ m/\$/) {
                $val =~ s{ \$ \{ (\w+) \} }{
                        $self->expand($section, $1, $key) || "";
                }exg;
        }
        return $val;
}

1;
__END__

# Local Variables:
# mode: cperl
# cperl-indent-level: 8
# indent-tabs-mode: nil
# cperl-continued-statement-offset: 8
# End:
#
# vim: ai et sw=8
