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

Armadito AV sources are compiled using automake and autoconf to generate Makefiles.
You can compile each part separately by ourself or use a single script to compile the whole project.

The first thing you need to do is setting OS_V variable inside "compile_all.sh". 
It corresponds to the out subdirectory name where will be stored build stuff.

For example, in **compile_all.sh** :
::

   OS_V=ubuntu-14.04-64

If you want to compile everything :
::
 
   cd scripts/
  ./compile_all.sh

If you want to compile one module only, or core :
::

  cd scripts/
  ./compile_all.sh PACKAGE

PACKAGE could be one of the following : 

* core (libarmadito)
* clamav (uhuru-mod-clamav)
* module5_2 (uhuru-mod-module5-2)
* modulePDF (uhuru-mod-PDF)


On WINDOWS :

Firstly, open VS/UhuruAV/UhuruAV.sln in Visual Studio (2013).
( # TO BE CONTINUED )

Licensing
---------

???

