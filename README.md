Uhuru
=====

Copyright (c) NOV'IT/Teclib, 2014, 2015

Project home: http://www.somewhere.org

What is it?
-----------

Uhuru is an open-source antivirus.

Dépendances
-----------

apt-get install libglib2.0-dev libmagic-dev libxml2-dev automake libtool libclamav-dev g++ libqt4-dev libqt4-svg qt4-designer qt4-dev-tools sni-qt valgrind

Packaging
---------------------
On LINUX :

In order to compile and prepare all packages for UhuruAV : 
- cd scripts/
- ./compile_all.sh

This script does everything, even rerepro update on the apt repository.

If you want to prepare a single package :
- cd scripts/
- ./compile_all.sh PACKAGE_NAME

PACKAGE_NAME could be one of the following : 
- core (libarmadito)
- clamav (uhuru-mod-clamav)
- module5_2 (uhuru-mod-module5-2)

On WINDOWS :

Firstly, open VS/UhuruAV/UhuruAV.sln in Visual Studio (2013).
( # TO BE CONTINUED )

Licensing
---------

???

