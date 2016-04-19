Compilation on Linux
====================

On linux, Armadito AV sources are compiled using automake and autoconf to generate Makefiles.

You can compile each part separately by ourself or use a single script to compile the whole project.

If you want to compile everything :

* cd scripts/
* ./compile_all.sh

If you want to compile one module only, or core :

* cd scripts/
* ./compile_all.sh MODULE

MODULE could be one of the following : 

* core (libarmadito)
* clamav (uhuru-mod-clamav)
* module5_2 (uhuru-mod-module5-2)
* modulePDF (uhuru-mod-PDF)

.. toctree::
