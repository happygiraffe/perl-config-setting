# @(#) $Id$

use strict;
use Module::Build;

Module::Build->new(
    module_name => 'Config::Setting',
    license     => 'bsd',
    requires    => {
        'Test::Simple' => 0,
    },
    create_makefile_pl => 'passthrough',
)->create_build_script;
