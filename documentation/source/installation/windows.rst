Installation on Windows
=======================

Prerequisites
*************

Packages redistribuables Visual C++ for Visual Studio 2013:
	- vcredist_x64.exe
	- vcredist_x86.exe

**URL** : <https://www.microsoft.com/fr-fr/download/details.aspx?id=40784>


Installation with MSI installer
*******************************

1. Download and install prerequisites (see previous section)

2. Download armadito-av msi:

	- Armadito-db-setup-0.10.0.msi
	- Armadito-setup-0.10.0.msi

3. Install armadito-av modules' databases :

	Launch the installer Armadito-db-setup-0.10.0.msi

4. Install armadito-av (analysis service + grapĥical user interface)

	Launch the installer Armadito-setup-0.10.0.msi


Installation from Sources
*************************

1. Build the armadito-av sources from the Visual Studio solution project.

Follow instructions in **Compilation** > **Compilation on Windows** section of this documentation.

2. Copy module's databases files in repository: 

::

    SOMEWHERE\\armadito-av\\build\\windows\\VS\\Armadito-AV\\out\\[build_mode]\\modules\\DB

3. Install the driver by executing the following command in a prompt as administrator :  

::

    ArmaditoGuard-setup --install

4. Install the analysis service by executing the following command in a prompt as administrator : 

::

    ArmaditoSvc --installBoot (service started at system start)

or 

::

    ArmaditoSvc --install (service started on demand)

.. toctree::
   
