#!/bin/sh
#
# Provide a trace of all the system calls that spider makes.
#
# $Id: trace_spider.sh,v 1.1 1999/03/11 15:39:49 dom Exp $
#

config=!CONFIG!

pidfile=`awk '$1 == "PID_File" {print $2}' $config`

strace -o spider_trace -p `cat $pidfile` &
