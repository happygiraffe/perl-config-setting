#!/bin/sh
#
# A script to shutdown Spider, by sending it a SIGTERM.
#
# $Id: kill_spider.sh,v 1.1 1999/03/11 15:39:49 dom Exp $
#

config=!CONFIG!

spool_dir=`awk '$1=="Spool_dir" {print $2}' $config`
pid_file=`awk '$1=="PID_File" {print $2}' $config`

cd $spool_dir
kill `cat $pid_file`
