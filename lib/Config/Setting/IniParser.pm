# Copyright (C) 2001 Dominic Mitchell.  All rights reserved.  This
# program is free software; you can redistribute it and/or modify it
# under the same terms as Perl itself.

=pod

=head1 NAME

Config::Setting::IniParser - parse windows .ini style files.

=head1 SYNOPSIS

 use Config::Setting::IniParser;

 my $ini = Config::Setting::IniParser->new(Filename => $inifile);
 foreach my $s ($ini->sections()) {
     print "[$s]\n";
     foreach my $k ($ini->keylist($s)) {
         print $k, "=", $ini->get($s, $k), "\n";
     }
     print "\n";
 }


=head1 DESCRIPTION

Config::Setting::IniParser provides OO access to windows .ini style
files.  At present, it only provides read access, not writing.

=head1 METHODS

=over 4

=item new ( ARGS )

Instantiate a new object.  ARGS is a set of keyword / value pairs.
Recognised options are:

=over 4

=item Filename

Process the filename provided.

=item String

Process the string passed in directly, rather than accessing it from
the disk.

=item CommentChar

Pass in a character that is used as a comment inside the data.  This
defaults to "#", but is also commonly ";".

=back

One of Filename or String is required.

=item sections ( )

Return a list of all sections that occurred in the data.  They are
returned in the order in which they originally occurred.

=item keylist ( SECTION )

Return a list of all keys in SECTION.

=item get ( SECTION, KEY )

Return the value of KEY in SECTION.

=back

=head1 SEE ALSO

perl(1).

=head1 AUTHOR

Dominic Mitchell, E<lt>dom@happygiraffe.netE<gt>.

=head1 BUGS

Does not cater for quoted keys and values.

It is a bit eager about comment stripping.

=cut

package Config::Setting::IniParser;

use strict;
use vars qw($rcsid $VERSION);

use Carp;

$rcsid = '@(#) $Id$ ';
$VERSION = substr q$Revision$, 10, -1;

# Pass in either a Filename parameter or a String parameter.
sub new {
	my ($proto, %args) = @_;

        croak "IniParser->new() requires Filename or String parameter."
                unless exists($args{Filename}) || exists($args{String});

        my $class = ref($proto) || $proto;
        my $self = {
                Contents    => {},
                Sections    => [],
                Filename    => "",
                String      => "",
                CommentChar => "#",
                %args,
        };
        bless($self, $class);
        return $self->_init->_parse;
}

# Read in the file that we have been asked to and parse it.
sub _init {
        my $self = shift;

        my $txt;
        if ($self->{Filename}) {
                open F, $self->{Filename}
                        or croak "open($self->{Filename}): $!";
                $self->{String} = join('', <F>);
                close F;
        }

        return $self;
}

# Parse the stuff we hold.
sub _parse {
        my $self = shift;
        my $section = "";
        my $cc = $self->{CommentChar};
        my $lineno = 1;

        foreach my $line (split /\r?\n/, $self->{String}) {
                $line =~ s/$cc.*//;
                $line =~ s/^\s+//;
                next unless $line;

                if ($line =~ m/^\[(.*?)\]/) {
                        $section = $1;
                        unless (exists $self->{Contents}{$section}) {
                                $self->{Contents}{$section} = {};
                                push @{ $self->{Sections} }, $section;
                        }
                } elsif ($line =~ m/^(.+?)\s*=\s*(.*)/) {
                        croak "line $lineno occurs outside a section"
                                unless $section;
                        $self->{Contents}{$section}{$1} = $2;
                } else {
                        carp "line $lineno is invalid: '$line'";
                }
        }
        return $self;
}

sub sections {
        my $self = shift;
        return @{ $self->{Sections} };
}

# I don't want to call this "keys"...  At the very least it messes up
# Emacs' syntax highlighting.
sub keylist {
        my $self = shift;
        my ($section) = @_;
        croak "usage: IniParser->keylist(section)"
                unless $section;
        return keys %{ $self->{Contents}{$section} };
}

sub get {
        my $self = shift;
        my ($section, $key) = @_;
        croak "usage: IniParser->get(section,key)"
                unless $section && $key;

        return $self->{Contents}{$section}{$key};
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
