#!/bin/sh
#
# A script to shutdown Spider, by sending it a SIGTERM.
#
# $Id: kill_spider.sh,v 1.2 2000/01/06 07:23:54 dom Exp $
#

config=!CONFIG!
awk=!AWK!

spool_dir=`$awk '$1=="Spool_dir" {print $2}' $config`
pid_file=`$awk '$1=="PID_File" {print $2}' $config`

cd $spool_dir
kill `cat $pid_file`
