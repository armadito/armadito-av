#!/bin/bash

set -x
set -e

HERE=$(dirname $0)

MAX_COUNT=10

function run_one()
{
    OUT=$HERE/perf/"$1"
    shift
    [ -f $OUT ] && rm $OUT

    COUNT=0
    while test "$COUNT" -lt $MAX_COUNT ; do
	PERF=/var/tmp/$$.perf
	rm -f $PERF

	/usr/bin/time --format='%e' --quiet --output=$PERF $* > perf/command.out 2>&1

	cat $PERF >> $OUT
	rm $PERF

	COUNT=$(($COUNT + 1))
    done
}

function start_fanotify()
{
    ./fanotify-test $* > fanotify.log 2>&1 &
    FANOTIFY_PID=$!
}

function stop_fanotify()
{
    kill -1 $FANOTIFY_PID
}

run_one without-fanotify make -C /home/francois/projects/uhuru/build/ubuntu-14.04-64/uhuru-linux-packaging ubuntu-local

start_fanotify -m / /home
run_one with-fanotify-enable0 make -C /home/francois/projects/uhuru/build/ubuntu-14.04-64/uhuru-linux-packaging ubuntu-local
stop_fanotify

start_fanotify -m -a / /home
run_one with-fanotify-enable1-log0-type0 make -C /home/francois/projects/uhuru/build/ubuntu-14.04-64/uhuru-linux-packaging ubuntu-local ; 
stop_fanotify

start_fanotify -m -a -t / /home
run_one with-fanotify-enable1-log0-type1 make -C /home/francois/projects/uhuru/build/ubuntu-14.04-64/uhuru-linux-packaging ubuntu-local ; 
stop_fanotify

start_fanotify -m -a -l / /home
run_one with-fanotify-enable1-log1-type0 make -C /home/francois/projects/uhuru/build/ubuntu-14.04-64/uhuru-linux-packaging ubuntu-local 
stop_fanotify

start_fanotify -m -a -l -t / /home
run_one with-fanotify-enable1-log1-type1 make -C /home/francois/projects/uhuru/build/ubuntu-14.04-64/uhuru-linux-packaging ubuntu-local 
stop_fanotify
