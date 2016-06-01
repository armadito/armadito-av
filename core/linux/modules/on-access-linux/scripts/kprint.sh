#!/bin/bash
set -x

cat /proc/sys/kernel/printk
echo 8 > /proc/sys/kernel/printk

echo -n "module fanotify_user +p" > /sys/kernel/debug/dynamic_debug/control
cat /sys/kernel/debug/dynamic_debug/control | grep fanotify
