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

	cd SOMEWHERE/armadito-av/gui
	bower install
	npm install


Installing node webkit
----------------------

After downloading latest stable SDK from http://nwjs.io/downloads/, untar the tarball:

::

	cd SOMEWHEREELSE
	tar xvzf nwjs-v0.14.0-linux-x64.tar.gz  ### adapt w.r.t. distribution, 32 or 64 bits

That's all

Running the interface
---------------------

First, the Armadito daemon must be launched. Refer to Armadito documentation for instructions.

The user interface can be launched with:

::

	cd SOMEWHERE/ng-armadito
	SOMEWHEREELSE/nwjs-sdk-v0.13.4-linux-x64/nw .


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

