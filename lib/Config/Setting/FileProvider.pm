# Copyright (C) 2001 Dominic Mitchell.  All rights reserved.  This
# program is free software; you can redistribute it and/or modify it
# under the same terms as Perl itself.

=pod

=head1 NAME

Config::Setting::FileProvider - return the contents of files.

=head1 SYNOPSIS

  use Config::Setting::FileProvider;
  my $p = Config::Setting::FileProvider->new(Env => "MYRCFILES",
                                             Paths => ["/etc/myrc",
                                                       "~/.myrc"]);
  my @contents = $p->provide();

=head1 DESCRIPTION

The Config::Setting::FileProvider module presents an interface to file
contents.  It returns the contents of various files, in order to the
application that requests it.

It is not intended that this class be used standalone, rather that it
be used as part of the Config::Setting module.

=head1 METHODS

=over 4

=item new ( ARGS )

Create a new Config::Setting::FileProvider object.  ARGS is a set of
keyword / value pairs.  Recognised options are:

=over 4

=item o

Env

The name of an environment variable to look at.  If it exists, it will
contain a colon separated list of paths to settings files.

=item o

Paths

A list of file paths to be used, in order, for settings files.

=back

In both Env and Paths, you may use the tilde-notation ("~") to specify
home directories.

Any Env settings files will be looked at I<after> any Paths settings
files.

It is reccomended that you specify both parameters in the constructor.

=item provide ( )

Return a list containing file contents.

=back

=head1 AUTHOR

Dominic Mitchell, E<lt>dom@happygiraffe.netE<gt>

=head1 SEE ALSO

Config::Setting(3).

=cut

package Config::Setting::FileProvider;

use strict;
use vars qw($rcsid $VERSION $default);

use Carp;
use Sys::Hostname;

$rcsid = '@(#) $Id$ ';
$VERSION = substr q$Revision$, 10, -1;
$default = "~/.settingsrc";

sub new {
        my ($proto, %args) = @_;

        my $class = ref($proto) || $proto;
        my $self = {
		Env => "SETTINGS_FILES",
		Paths => [ $default ],
		%args,
		Files => [ ],	# Must not be overridden!
	};
        bless $self, $class;
        return $self->_init();
}

sub _init() {
	my $self = shift;
	my @files = @{ $self->{Paths} };

	# Allow listed files to be overridden by a hostname-specific
	# one.
	my $hn = hostname;
	@files = map { $_, "$_.$hn" } @files;

	# Always allow the environment to override previous choices.
	if ($ENV{$self->{Env}}) {
		push @files, split(":", $ENV{$self->{Env}});
	}
	push @{ $self->{Files} }, @files;
	return $self;
}

# Utility function.
sub _tildesubst {
	my $fn = shift || $_;
	if ($fn =~ m!^~([^/]*)!) {
		$fn =~ s!!$1 ? (getpwnam($1))[7] :
			($ENV{HOME} || $ENV{LOGDIR})!ex;
	}
	return $fn;
}

sub provide {
	my $self = shift;
	my @files = map(_tildesubst, @{ $self->{Files} });
	my @texts;
        my $first = 1;
	foreach my $f (@files) {
                # First file in the list is considered "important".
                unless (-f $f) {
                        next unless $first;
                        croak "can't stat $f";
                }
                $first = 0;
		next unless -f $f;
		open F, $f
			or croak "open($f): $!";
		my $txt = join "", <F>;
		close F;
		push @texts, $txt;
	}
	return @texts;
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
