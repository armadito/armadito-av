Uhuru GUI
=========

Copyright (c) NOV'IT/Teclib, 2014, 2015

Project home: http://www.somewhere.org


What is it?
-----------

Uhuru GUI is the graphical user interface for the Uhuru antivirus. It provides the following services:

- scanning of files or directory on demand
- notification of automatic scans
- past analysis and events
- notification of databases updates
- antivirus state (running services, alerts...)


Licensing
---------

TBW


Prerequisites
-------------

In order to compile Uhuru GUI, you need the following tools:

- automake/autoconf
- GNU make
- C++ compiler
- libuhuru library and headers
- Qt dev tools

To install the needed packages:

- Ubuntu: `apt-get install g++ libqt4-dev libqt4-svg qt4-designer qt4-dev-tools`
- CentOS: ...


Configuration
-------------

Once git repo cloned, you need to initialize the build using automake, autoconf and tools.
A shell script `autogen.sh` is provided to ease this step:

    $ ./autogen.sh 
    + aclocal --force
    + autoheader --force
    + automake --foreign --add-missing --force-missing --copy
    + autoconf --force

This will generate the `Makefile.in` files and the `configure` script.

`configure` script takes the following useful options:

    --prefix=PREFIX         install architecture-independent files in PREFIX
                            [/usr/local]
    --enable-debug          enable debugging [default is yes]

The `PREFIX` directory will be used by `make install`. Its use is mandatory,
since running the GUI from the build tree is not yet supported.

Building in a separate directory is highly recommended, unless you really want
to clobber the source tree with objects, libraries, binaries and other stuff.

    $ mkdir /home/joebar/build/gui

Typical invocation of the configure script is:

    $ /home/joebar/uhuru-av/gui/configure --prefix=/home/joebar/install --enable-debug 


Compiling
---------

Once configured, compilation is easy:

    $ make
    make  all-recursive
    make[1]: entrant dans le répertoire " /home/joebar/build/gui "
    ...


Installing
----------

After compiling, installation is done by:

    $ make install
    ...

This will install libraries, tools, icons... in the subdirectories of the PREFIX
directory defined at configure time.


Running
-------

If Uhuru is not installed in standard directories (and this is highly *not* recommended),
running the Uhuru tools requires first to use the LD_LIBRARY_PATH environment variable
to specify install location of libuhuru. This can be done either:

- for the shell session:
    $ export LD_LIBRARY_PATH=/home/joebar/install/lib

- or for each command:
    $ LD_LIBRARY_PATH=/home/joebar/install/lib /home/joebar/install/bin/uhuru-qt

Running the GUI requires first to launch the Uhuru scan daemon, which is compiled
and installed with libuhuru.

To run the Uhuru scan daemon:

    $ LD_LIBRARY_PATH=/home/joebar/install/lib /home/joebar/install/bin/uhuru-scan-daemon -n

Then GUI can be launched with:

    $ LD_LIBRARY_PATH=/home/joebar/install/lib /home/joebar/install/bin/uhuru-qt


Documentation
-------------

TBW


