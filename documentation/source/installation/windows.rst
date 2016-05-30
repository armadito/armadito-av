Installation on Windows
======================

How to install Armadito on Windows


Prerequisites
*************

Packages redistribuables Visual C++ for Visual Studio 2013:
	- vcredist_x64.exe
	- vcredist_x86.exe

url: https://www.microsoft.com/fr-fr/download/details.aspx?id=40784


Installation with msi installer
*******************************

1. Download and install prerequisites (see previous section)

2. Download armadito-av msi:

	- Armadito-db-setup-0.10.0.msi
	- Armadito-setup-0.10.0.msi

3. Install armadito-av module's database.

	Launch the installer Armadito-db-setup-0.10.0.msi

4. Install armadito-av (analysis service + user interface)

	Launch the installer Armadito-setup-0.10.0.msi


Installation from Sources
*************************


1. Build the armadito-av sources from the Visual Studio solution project.

See Section "Compilation on Windows"

Compilation results are located in repository: *SOMEWHERE\\armadito-av\\build\\windows\\VS\\Armadito-AV\\out\\[config]*

2. Copy module's databases files in repository: *SOMEWHERE\\armadito-av\\build\\windows\\VS\\Armadito-AV\\out\\[config]\\modules\\DB*

3. Install the driver.

Execute the command in a prompt as administrator:  ArmaditoGuard-setup --install

4. Install the analysis service

Execute the command in a prompt as administrator: ArmaditoSvc --installBoot (service started at system start)
or ArmaditoSvc --install (service started on demand)


.. toctree::
   
