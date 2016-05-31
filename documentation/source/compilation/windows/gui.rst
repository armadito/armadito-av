Armadito gui
============

Installing prerequisites
************************

The prerequisites are:

* node.js
* git (needed by bower)
* bower

Installing node and bower
*************************

First, download node.js from https://nodejs.org/en/download/, using either the .msi or the .exe installer, at your choice.

During installation, default configuration choices are OK.

Once node is installed, launch a command line.

Checking installation:

	npm --version
	2.15.1

Then install bower using:

	npm install -g bower

Install git
***********

To use bower, you must first install git.

git for windows is available here: https://git-for-windows.github.io/

Checking installation:

	git version 2.8.1.windows.1


Installing modules in source tree
*********************************

After cloning the sources:

	git clone git@gitlab.teclib.infra:armadito/ng-armadito.git

run bower from ng-armadito directory to install the needed modules:

	bower install
	npm install

This should output a lot of messages


Installing node web kit
***********************

Download the archive from: http://nwjs.io/downloads/

Make sure to download the SDK.

Extract the archive using Windows file explorer.


