#!/bin/sh
#
# You might prefer to do this as an alias from your shell.
#
# $Id: debug_client.sh,v 1.2 2000/01/06 07:23:54 dom Exp $
#

config=!CONFIG!
awk=!AWK!
spider_host=${SPIDER_HOST:-localhost}

port=`$awk '$1=="Port" {print $2}' $config`

exec telnet $spider_host $port
