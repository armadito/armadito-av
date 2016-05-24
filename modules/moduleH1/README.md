Armadito H1 module
================

Copyright (c) NOV'IT/Teclib, 2014, 2015

Project home: http://www.somewhere.org


What is it?
-----------

Armadito H1 module is a scan module dedicated to binaries analysis (PE and ELF).
It is based on the libarmadito library.


Licensing
---------

TBW


Prerequisites
-------------

In order to compile Armadito H1 module, you need the following tools:

- automake/autoconf
- GNU make
- C compiler
- H1 original sources


Configuration
-------------

To initialize the build using automake, autoconf and tools, a shell script 
`autogen.sh` is provided to ease this step:

    $ ./autogen.sh
    + aclocal --force
    + libtoolize --force --automake --copy
    + automake --foreign --add-missing --force-missing --copy
    + autoconf --force

This will generate the `Makefile.in` files and the `configure` script.

`configure` script takes the following useful options:

    --prefix=PREFIX         install architecture-independent files in PREFIX
                            [/usr/local]
    --enable-debug          enable debugging [default is yes]

The `PREFIX` directory will be used by `make install`. Its use is mandatory, unless 
building a package and installing in system directories.

libarmadito use the `pkg-config` utility to specify compiling options relative to 
libarmadito. Since the `libarmadito.pc` specification file for `pkg-config` is not located
in standard directory (usual `/usr/lib/pkgconfig`), invoking the configure script 
must use the PKG_CONFIG_PATH environment variable.

Building in a separate directory is highly recommended, unless you really want
to clobber the source tree with objects, libraries, binaries and other stuff.

    $ mkdir -p /home/joebar/build/modules/moduleH1

Typical invocation of the configure script is:

    $ /home/joebar/armadito-av/modules/moduleH1/configure --prefix=/home/joebar/install --enable-debug PKG_CONFIG_PATH=/home/joebar/install/lib/pkgconfig

Note that the path specified in the value of PKG_CONFIG_PATH must be coherent
with the PREFIX used in libarmadito installation (see `libarmadito` README).


Compiling
---------

Once configured, compilation is easy:

    $ make
    make  all-recursive
    make[1]: entrant dans le répertoire " /home/joebar/build/modules/moduleH1 "
    ...


Installing
----------

After compiling, installation is done by:

    $ make install
    ...

This will install libraries, configuration files... in the subdirectories of the PREFIX
directory defined at configure time.


Documentation
-------------

TBW
