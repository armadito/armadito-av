#!/bin/sh
set -x
awk -F = '{printf("m4_define([%s],[%s])\n",$1,$2);}' VERSION > version.m4
aclocal --force
libtoolize --force --automake --copy 
autoheader --force
automake --foreign --add-missing --force-missing --copy
autoconf --force

