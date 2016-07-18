Armadito gui
============

Armadito gui rely on recent web technologies to provide a multi-plateform graphical user interface for Armadito antivirus. 

Prerequisites
-------------

To run Armadito graphical user interface, you need:

- NW.js SDK
- bower
- node.js


Installing node and bower
-------------------------

Installations are done as root.

Installing node.js:

::

	apt-get install nodejs-legacy

Checking installation:

::

	npm --version
	1.4.21

Installing bower (must be done as root too):

::

	npm install -g bower


Installing modules in source tree
---------------------------------

After cloning the repository, the source tree of the user interface must be configured for node webkit:

::

	cd /home/joebar/armadito-av/gui
	bower install
	npm install


Installing node webkit
----------------------

Downloading the latest normal 32 and 64 bit from http://nwjs.io/downloads/, under the tarball:

::

        into the /home/joebar/install

That's all

Configuration
-------------

Once git repo cloned, you need to initialize the build using automake, autoconf and tools.
A shell script **autogen.sh** is provided to ease this step:

::

    $ ./autogen.sh 
    + aclocal --force
    + automake --foreign --add-missing --force-missing --copy
    + autoconf --force

This will generate the **Makefile.in** files and the **configure** script.

**configure** script takes the following useful options:

    --prefix=PREFIX         install architecture-independent files in PREFIX [default is /usr/local]
    
The **PREFIX** directory will be used by **make install**. Its use is mandatory, unless 
building a package and installing in system directories, since building the
scanning modules and the graphical user interface will need a libarmadito properly
installed.

Typical invocation of the configure script is:

::

    $ /home/joebar/armadito-av/gui/configure --prefix=/home/joebar/install --enable-debug 

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

Running the interface
---------------------

First, the Armadito daemon must be launched. Refer to Armadito documentation for instructions.

The user interface can be launched with:

::

	exec /home/joebar/install/lib/armadito/gui/nwjs/current/nw /home/joebar/install/lib/armadito/gui


Debugging the interface
-----------------------

Once the interface is launched:

- right-click in the window to display debug menu and select "Inspect" or tap F12
- in the inspector window, select the "console" tab

Build with grunt
----------------

Install grunt :

:: 

         npm install -g grunt-cli

Run `grunt` for building and `grunt serve` for preview.

You can use "--force" if you want to build with warnings.

.. note:: This project is generated with [yo angular generator] version 0.15.1.

