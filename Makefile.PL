# Copyright (C) 2001 Dominic Mitchell.  All rights reserved.  This
# program is free software; you can redistribute it and/or modify it
# under the same terms as Perl itself.

# @(#) $Id: Makefile.PL,v 1.2 2002/02/04 08:37:26 dom Exp $

use ExtUtils::MakeMaker;
WriteMakefile(
	'NAME'         => 'Config::Setting',
	'VERSION_FROM' => 'lib/Config/Setting.pm', # finds $VERSION
	'PREREQ_PM'    => {
	    'Test::Simple' => 0,
	},
	'MAN3PODS'     => {
		'lib/Config/Setting.pm' =>
			'$(INST_MAN3DIR)/Config::Setting.3',
		'lib/Config/Setting/FileProvider.pm' =>
			'$(INST_MAN3DIR)/Config::Setting::Fileprovider.3',
		'lib/Config/Setting/IniParser.pm' =>
			'$(INST_MAN3DIR)/Config::Setting::IniParser.3',
		'lib/Config/Setting/XMLParser.pm' =>
			'$(INST_MAN3DIR)/Config::Setting::XMLParser.3',
	},
);
