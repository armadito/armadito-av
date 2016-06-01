Installation on Linux
====================

Installation from sources
-------------------------

When installing Armadito from sources (either tarball or git clone), you must first build it. Refer to section **Compilation** > **Compilation on Linux** of this documentation for detailed instructions.

After proper configuration and build of the different parts (core, modules, gui), each part is simply installed by the following command:

::

    $ make install

This will install libraries, tools, header files... in the subdirectories of the **PREFIX**
directory defined at configure time.

This command must be repeated for each directory (core/, modules/*/, gui/).

Care must be take to configure each part with the same **prefix** so that the different components are installed at their respective locations.


Installation from packages
--------------------------

Ubuntu distributions
~~~~~~~~~~~~~~~~~~~~

Packages for Ubuntu distributions are available at:

**URL**: <https://launchpad.net/~armadito/+archive/ubuntu/armadito-av>

::

      sudo add-apt-repository ppa:armadito/armadito-av


Note that this PPA is experimental and that graphical user interface is not yet packaged in a clean way.


.. toctree::
