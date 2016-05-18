Prepare compilation
*******************

Firstly, you need to download sources from our public github.
The next thing you need to do is setting the OS_V variable inside main script "compile_all.sh". 
It corresponds to the out subdirectory name where will be stored build stuff.

For example, in **compile_all.sh** :
::

   OS_V=ubuntu-14.04-64


Compile everything
******************

If you want to compile everything :
::

   $ cd armadito-av/scripts/
   $ ./compile_all.sh


Compile module by module
************************

If you want to compile one module only, or core :
::

   $ cd armadito-av/scripts/
   $ ./compile_all.sh PACKAGE

**PACKAGE** could be one of the following : 

* core (libarmadito)
* clamav (uhuru-mod-clamav)
* module5_2 (uhuru-mod-module5-2)
* modulePDF (uhuru-mod-PDF)

Compilation Results
*******************

By default, compilation results are located in **armadito-av/out/build/$OS_V**.

The next step is installation, i.e. with "make install".
