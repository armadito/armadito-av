#!/bin/bash

set -e

HERE=$(dirname $0)

rm -f OUT
$HERE/fanotify-test -a -m $* $HERE > $HERE/fanotify.out 2>&1
