Armadito gui
============

Installing prerequisites
------------------------

The prerequisites are:

* node.js
* git (needed by bower)
* bower

Installing node and bower
-------------------------

First, download node.js from https://nodejs.org/en/download/, using either the .msi or the .exe installer, at your choice.

During installation, default configuration choices are OK.

Once node is installed, launch a command line.

Checking installation:

::

	npm --version
	2.15.1

Then install bower using:

::

	npm install -g bower

Install git
-----------

To use bower, you must first install git.

git for windows is available here : https://git-for-windows.github.io/

Checking installation:

::

	git version 2.8.1.windows.1


Installing modules in source tree
---------------------------------


Run bower from **armadito-av/gui** directory to install the needed modules:

::
         
         cd SOMEWHERE/armadito-av/gui
	 bower install
	 npm install

This should output a lot of messages


Installing node webkit
----------------------

Download the archive from: http://nwjs.io/downloads/

Make sure to download the SDK.

Extract the archive using Windows file explorer.


Running the interface
---------------------

First, the Armadito service must be launched.

The user interface can be launched with:

::

	cd SOMEWHERE\ng-armadito
	SOMEWHEREELSE\nwjs-sdk-v0.14.0-win-x64\nw.exe .

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


