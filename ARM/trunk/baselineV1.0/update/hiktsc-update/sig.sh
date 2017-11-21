#!/bin/sh
pid=`ps | grep hikTSC | awk 'NR==1{print $1}'`
kill -10 $pid
