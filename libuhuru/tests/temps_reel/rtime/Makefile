all:
	gcc -g -o rtime -Wall -Wextra -Iincludes \
	-I/usr/include/dbus-1.0 \
    -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include \
    -I/usr/include/glib-2.0 \
    -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ \
    -ldbus-1 \
    -I/usr/include/gio-unix-2.0 \
    -ldbus-glib-1 src/* main.c