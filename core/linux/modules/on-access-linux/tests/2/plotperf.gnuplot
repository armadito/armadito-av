set encoding utf8
set title "performances"
set xlabel '#run'
set ylabel 'time in seconds'
#set xtics 0,1,4
#set xrange [-0.5:4.5]
set yrange [100:200]

## set terminal png size 400,300 enhanced font "Helvetica,20"
## set terminal png enhanced font "Helvetica,20"
set terminal png enhanced
set output 'perf/out.png'

plot "perf/without-fanotify" title 'without fanotify' with linespoints, \
     "perf/with-fanotify-enable0" title 'with fanotify, not enabled' with linespoints, \
     "perf/with-fanotify-enable1-log0-type0" title 'with fanotify, enabled, no log, no type check' with linespoints, \
     "perf/with-fanotify-enable1-log0-type1" title 'with fanotify, enabled, no log, type check' with linespoints, \
     "perf/with-fanotify-enable1-log1-type0" title 'with fanotify, enabled, log, no type check' with linespoints;
