Armadito 5.2 module
================

Copyright (c) NOV'IT/Teclib, 2014, 2015

Project home: http://www.somewhere.org


What is it?
-----------

Armadito 5.2 module is a scan module dedicated to binaries analysis (PE and ELF).
It is based on the libarmadito library.


Licensing
---------

TBW


Prerequisites
-------------

In order to compile Armadito 5.2 module, you need the following tools:

- automake/autoconf
- GNU make
- C compiler
- 5.2 original sources


Configuration
-------------

The sources of the module 5.2 are divided into:

- the analysis code, maintained in a separate git repo
- this code, which acts as 'glue' between the analysis code and libarmadito

The analysis code repository must be cloned before running the configure 
script. Once this clone done, a symbolic link must be created in the src/
subdirectory of this directory, named `module5_2_sources` (this name is
mandatory).

    $ cd src/
    $ ln -s /home/joebar/module5.2 ./module5_2_sources

If `configure` script fails with a message like:

    configure: error: cannot find sources (src/module5_2_sources/UhuruStatic/resources/UhuruStatic.h) in /home/joebar/uhuru-av/modules/module5_2 or ..

it means that the symbolic link was not properly created.

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

    $ mkdir -p /home/joebar/build/modules/module5_2

Typical invocation of the configure script is:

    $ /home/joebar/armadito-av/modules/module5_2/configure --prefix=/home/joebar/install --enable-debug PKG_CONFIG_PATH=/home/joebar/install/lib/pkgconfig

Note that the path specified in the value of PKG_CONFIG_PATH must be coherent
with the PREFIX used in libarmadito installation (see `libarmadito` README).


Compiling
---------

Once configured, compilation is easy:

    $ make
    make  all-recursive
    make[1]: entrant dans le répertoire " /home/joebar/build/modules/module5_2 "
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
