# Copyright (C) 2001 Dominic Mitchell.  All rights reserved.  This
# program is free software; you can redistribute it and/or modify it
# under the same terms as Perl itself.

=pod

=head1 NAME

Config::Setting::XMLParser - parse XML settings file.

=head1 SYNOPSIS

 use Config::Setting::XMLParser;

 my $ini = Config::Setting::XMLParser->new(Filename => $xmlfile);
 foreach my $s ($ini->sections()) {
     print "[$s]\n";
     foreach my $k ($ini->keylist($s)) {
         print $k, "=", $ini->get($s, $k), "\n";
     }
     print "\n";
 }


=head1 DESCRIPTION

Config::Setting::XMLParser provides access to settings stored in an
XML File.  The XML File is expected to have the following structure:

  <settings>
    <section name="SECTION">
      <item name="KEY">VALUE</item>
    </section>
  </settings>

Multiple E<lt>sectionE<gt>s and E<lt>itemE<gt>s may be present.  Any
leading and trailing whitespace within an E<lt>itemE<gt> tag will be
stripped.

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

=back

One of Filename or String is required.  Any remaining arguments will
be passed to the XML::Parser constructor.

=item sections ( )

Return a list of all sections that occurred in the data.  They are
returned in the order in which they originally occurred.

=item keylist ( SECTION )

Return a list of all keys in SECTION.

=item get ( SECTION, KEY )

Return the value of KEY in SECTION.

=back

=head1 SEE ALSO

perl(1),
Config::Setting::IniParser(3).

=head1 AUTHOR

Dominic Mitchell, E<lt>dom@happygiraffe.netE<gt>.

=cut

package Config::Setting::XMLParser;

use strict;
use vars qw($rcsid $VERSION);

use Carp;

use XML::Parser;

$rcsid = '@(#) $Id$ ';
$VERSION = substr q$Revision$, 10, -1;

# Pass in either a Filename parameter or a String parameter.
sub new {
        my $class = shift;
        my (%args) = @_;

        croak "XMLParser->new() requires Filename or String parameter."
                unless exists($args{Filename}) || exists($args{String});

        my $self = {
                Contents => {},
                Sections => [],
                Filename => delete $args{Filename},
                String => delete $args{String},
        };
        bless($self, $class);
        return $self->_init->_parse(%args);
}

# Read in the file that we have been asked to and parse it.
sub _init {
        my $self = shift;

        my $txt;
        if ($self->{Filename}) {
                open my $fh, $self->{Filename}
                        or croak "open($self->{Filename}): $!";
                $self->{String} = do { local $/ ; <$fh> };
                close F;
        }

        return $self;
}

#---------------------------------------------------------------------

{
        my $me;                 # Copy of $self during parse.

        # Parse the stuff we hold.
        sub _parse {
                my $self = shift;
                my %args = @_;
                my $p = XML::Parser->new(
                        Style => "Subs",
                        Pkg => ref($self),
                        %args
                       );
                $p->setHandlers(Char => \&Text);

                $me = $self;
                $me->{CurItem} = 0;
                eval {
                        $p->parse($self->{String});
                };
                croak "$@" if $@;

                delete $self->{CurSection};
                delete $self->{CurItem};
                delete $self->{CurText};

                return $self;
        }

        sub section {
                my ($expat, $tag, %attrs) = @_;
                my $section = $attrs{name};
                croak "no section name specified!"
                        unless $section;
                unless (exists $me->{Contents}{$section}) {
                        $me->{Contents}{$section} = {};
                        push @{ $me->{Sections} }, $section;
                }
                $me->{CurSection} = $section;
#                warn ">> <section name='$section'>\n";
        }

        sub item {
                my ($expat, $tag, %attrs) = @_;
                my $key = $attrs{name};
                croak "no item name specified!"
                        unless $key;
                $me->{CurItem} = $key;
                my $section = $me->{CurSection};
                $me->{Contents}{$section}{$key} = "";
#                warn ">> <item name='$key'>\n";
        }

        sub Text {
                my ($expat, $val) = @_;
                return unless $me->{CurItem};
                my $section = $me->{CurSection};
                my $key = $me->{CurItem};
                $me->{Contents}{$section}{$key} .= $val;
#                warn ">> Text($val)\n";
        }

        sub item_ {
                my ($expat, $tag) = @_;
                my $section = $me->{CurSection};
                my $key = $me->{CurItem};
                # Trim whitespace.
                $me->{Contents}{$section}{$key} =~ s/^\s*(.*)\s*$/$1/;
#                warn ">> </item>\n";
                $me->{CurItem} = 0;
        }
}

#---------------------------------------------------------------------

sub sections {
        my $self = shift;
        return @{ $self->{Sections} };
}

# I don't want to call this "keys"...  At the very least it messes up
# Emacs' syntax highlighting.
sub keylist {
        my $self = shift;
        my ($section) = @_;
        croak "usage: XMLParser->keylist(section)"
                unless $section;
        return keys %{ $self->{Contents}{$section} };
}

sub get {
        my $self = shift;
        my ($section, $key) = @_;
        croak "usage: XMLParser->get(section,key)"
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
# vim: ai et sw=8 :
