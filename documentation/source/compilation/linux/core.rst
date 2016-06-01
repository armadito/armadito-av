Armadito core
=============

**Armadito core** corresponds to libarmadito library. Symbols exported from this library allows all modules libraries to use the same APIs.

Prerequisites
-------------

In order to build libarmadito, you need the following tools:

- automake/autoconf
- GNU make
- C compiler
- glib, libmagic, libxml2 library and headers

To install the needed headers:

- Ubuntu: 

::

     apt-get install automake autoconf libtool libglib2.0-dev libmagic-dev libxml2-dev libjson-c-dev


Configuration
-------------

Once git repo cloned, you need to initialize the build using automake, autoconf and tools.
A shell script **autogen.sh** is provided to ease this step:

::

    $ ./autogen.sh 
    + aclocal --force
    + libtoolize --force --automake --copy
    + autoheader --force
    + automake --foreign --add-missing --force-missing --copy
    + autoconf --force

This will generate the **Makefile.in** files and the **configure** script.

**configure** script takes the following useful options:

    --prefix=PREFIX         install architecture-independent files in PREFIX
                            [/usr/local]
    --enable-debug          enable debugging [default is yes]

The **PREFIX** directory will be used by **make install**. Its use is mandatory, unless 
building a package and installing in system directories, since building the
scanning modules and the graphical user interface will need a libarmadito properly
installed.

Building in a separate directory is highly recommended, unless you really want
to clobber the source tree with objects, libraries, binaries and other stuff.

::

    $ mkdir /home/joebar/build/libarmadito

Typical invocation of the configure script is:

::

    $ /home/joebar/armadito-av/libarmadito/configure --prefix=/home/joebar/install --enable-debug 


Building
--------

Once configured, build is easy:

::

    $ make

Installing
----------

After build, installation is done by:

::

    $ make install

This will install libraries, tools, header files... in the subdirectories of the **PREFIX**
directory defined at configure time.

.. toctree::
