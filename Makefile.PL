# Copyright (C) 2001 Dominic Mitchell.  All rights reserved.  This
# program is free software; you can redistribute it and/or modify it
# under the same terms as Perl itself.

# @(#) $Id: Makefile.PL,v 1.1 2001/06/24 15:21:58 dom Exp $

use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
	'NAME'	=> 'Config::Setting',
	'VERSION_FROM' => 'lib/Config/Setting.pm', # finds $VERSION
	'MAN3PODS'  => {
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
