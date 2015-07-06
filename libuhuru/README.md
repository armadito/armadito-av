libuhuru
========

Copyright (c) NOV'IT/Teclib, 2014, 2015

Project home: http://www.somewhere.org


What is it?
-----------

libuhuru is the support library for the Uhuru antivirus. It provides the following services:

- scanning of files or directory by scanning modules
- dynamic loading of scanning modules
- alert reporting and quarantine
- information on databases
- configuration 


Licensing
---------

TBW


Prerequisites
-------------

In order to compile libuhuru, you need the following tools:

- automake/autoconf
- GNU make
- C compiler
- glib, libmagic, libxml2 library and headers

To install the needed headers:

- Ubuntu: `apt-get install libglib2.0-dev libmagic-dev libxml2-dev`
- CentOS: ...

If you want to build documentation, you will need the following additional tools:

- asciidoc


Configuration
-------------

Once git repo cloned, you need to initialize the build using automake, autoconf and tools.
A shell script `autogen.sh` is provided to ease this step:

    $ ./autogen.sh 
    + aclocal --force
    + libtoolize --force --automake --copy
    + autoheader --force
    + automake --foreign --add-missing --force-missing --copy
    + autoconf --force

This will generate the `Makefile.in` files and the `configure` script.

`configure` script takes the following useful options:

    --prefix=PREFIX         install architecture-independent files in PREFIX
                            [/usr/local]
    --enable-debug          enable debugging [default is yes]

The `PREFIX` directory will be used by `make install`. Its use is mandatory, unless 
building a package and installing in system directories, since compiling the
scannning modules and the graphical user interface will need a libuhuru properly
installed.

Building in a separate directory is highly recommended, unless you really want
to clobber the source tree with objects, libraries, binaries and other stuff.

    $ mkdir /home/joebar/build/libuhuru

Typical invocation of the configure script is:

    $ /home/joebar/uhuru-av/libuhuru/configure --prefix=/home/joebar/install --enable-debug 


Compiling
---------

Once configured, compilation is easy:

    $ make
    make  all-recursive
    make[1]: entrant dans le répertoire " /home/joebar/build/libuhuru "
    ...


Installing
----------

After compiling, installation is done by:

    $ make install
    ...

This will install libraries, tools, header files... in the subdirectories of the PREFIX
directory defined at configure time.


Documentation
-------------

TBW
