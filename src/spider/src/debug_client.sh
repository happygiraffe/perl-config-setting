#!/bin/sh
#
# You might prefer to do this as an alias from your shell.
#
# $Id: debug_client.sh,v 1.1 1999/03/11 15:39:49 dom Exp $
#

config=!CONFIG!
spider_host=${SPIDER_HOST:-localhost}

port=`awk '$1=="Port" {print $2}' $config`

exec telnet $spider_host $port
