#!/bin/bash
set -x

HERE=$(dirname $0)
. $HERE/k.sh

rm -f /home/uhuru/projects/uhuru/install/ubuntu-14.04-64/etc/uhuru/conf.d/on-access-linux-test.conf
LD_LIBRARY_PATH=/home/uhuru/projects/uhuru/install/ubuntu-14.04-64/lib strace -e trace=open,close /home/uhuru/projects/uhuru/install/ubuntu-14.04-64/sbin/uhuru-scand --no-daemon --log-level=debug
