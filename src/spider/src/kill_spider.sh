#!/bin/sh
#
# A script to shutdown Spider, by sending it a SIGTERM.
#
# $Id: kill_spider.sh,v 1.3 2000/01/15 11:34:33 dom Exp $
#

config=!CONFIG!
awk=!AWK!

spool_dir=`$awk '$1=="Spool_dir" {print $2}' $config`
pid_file=`$awk '$1=="PID_File" {print $2}' $config`

cd $spool_dir
test -f $pid_file && kill `cat $pid_file`
