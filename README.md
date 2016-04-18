# Armadito user interface

## Prerequisites

To run Armadito graphical user interface, you need:

- node webkit SDK
- bower
- node.js

### Installing prerequisites on Linux

#### Installing node and bower

Installations are done as root.

Installing node.js:

	apt-get install nodejs-legacy

Checking installation:

	npm --version
	1.4.21

Installing bower (must be done as root too):

	npm install -g bower


#### Installing modules in source tree on Linux

After cloning the repository, the source tree of the user interface must be configured for node webkit:

	cd SOMEWHERE/ng-armadito
	bower install
	npm install


#### Installing node web kit on Linux

After downloading latest stable SDK (actually 0.14.0) from http://nwjs.io/downloads/, untar the tarball:

	cd SOMEWHEREELSE
	tar xvzf nwjs-v0.14.0-linux-x64.tar.gz  ### adapt w.r.t. distribution, 32 or 64 bits

That's all

### Installing prerequisites on Windows

The prerequisites are:

* node.js
* git (needed by bower)
* bower

#### Installing node and bower

First, download node.js from https://nodejs.org/en/download/, using either the .msi or the .exe installer, at your choice.

During installation, default configuration choices are OK.

Once node is installed, launch a command line.

Checking installation:

	npm --version
	2.15.1

Then install bower using:

	npm install -g bower

#### Install git on windows

To use bower, you must first install git.

git for windows is available here: https://git-for-windows.github.io/

Checking installation:

	git version 2.8.1.windows.1


#### Installing modules in source tree on Windows


After cloning the sources:

	git clone git@gitlab.teclib.infra:armadito/ng-armadito.git

run bower to install the needed modules:

	bower install
	npm install

This should output a lot of messages


#### Installing node web kit on Windows

Download the archive from: http://nwjs.io/downloads/

Make sure to download the SDK.

Extract the archive using Windows file explorer.


## Running the interface

### Running the interface on Linux

First, the Armadito daemon must be launched. Refer to Armadito documentation for instructions.

The user interface can be launched with:

	cd SOMEWHERE/ng-armadito
	SOMEWHEREELSE/nwjs-sdk-v0.13.4-linux-x64/nw .


### Running the interface on Windows

First, the Armadito service must be launched. Refer to Armadito documentation for instructions.

The user interface can be launched with:

	cd SOMEWHERE\ng-armadito
	SOMEWHEREELSE\nwjs-sdk-v0.14.0-win-x64\nw.exe .


## Debugging the interface

Once the interface is launched:

- right-click in the window to display debug menu and select "Inspect"
- in the inspector window, select the "console" tab


## Build & development

This section is may be obsolete?

Run `grunt` for building and `grunt serve` for preview.
p
## Testing

This section is may be obsolete?

Running `grunt test` will run the unit tests with karma.

## Note

This project is generated with [yo angular generator](https://github.com/yeoman/generator-angular) version 0.15.1.

