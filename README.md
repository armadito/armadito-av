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
  bower install angular-translate-loader-static-files --save    ## not sure this is still needed
 

#### Installing node modules dependencies

  cd SOMEWHERE/ng-armadito
  npm install node-lang-info

#### Installing node web kit on Linux

After downloading latest stable SDK (actually 0.14.0) from http://nwjs.io/downloads/, untar the tarball:

  cd SOMEWHEREELSE
  tar xvzf nwjs-v0.14.0-linux-x64.tar.gz  ### adapt w.r.t. distribution, 32 or 64 bits

That's all

## Running the interface

### Running the interface on Linux

First, the Armadito daemon must be launched. Refer to Armadito documentation for instructions.

The user interface can be launched with:

  cd SOMEWHERE/ng-armadito
  SOMEWHEREELSE/nwjs-sdk-v0.13.4-linux-x64/nw .


## Debugging the interface

Once the interface is launched:

- right-click in the window to display debug menu and select "Inspect"
- in the inspector window, select the "console" tab


## Build & development

This section is may be obsolete?

Run `grunt` for building and `grunt serve` for preview.

## Testing

This section is may be obsolete?

Running `grunt test` will run the unit tests with karma.

## Note

This project is generated with [yo angular generator](https://github.com/yeoman/generator-angular) version 0.15.1.

