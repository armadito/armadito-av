Compilation on Windows
======================

Set developpement environnement
*******************************

On Windows, you can compile Armadito AV sources with:
	- Visual Studio
	- and Windows Driver Kit (for the driver).


Notes:
This version has been developped and tested with Visual Studio Community 2013 and Windows Driver Kit 8.1
You might have to apply some modifications regarding to which Visual Studio's version you use.



Projects Compilation
********************

Open the armadito-av VS project at location : 
 
   *SOMEWHERE\\armadito-av\\build\\windows\\VS12\\Armadito-AV\\Armadito-AV.sln*

The projects availables in this solution are separated in subfolders:

- Driver
	- ArmaditoGuard (driver sources)
	- ArmaditoGuard Package (driver package)
- Libarmadito
	- libarmadito (armadito core library)
- Modules
	- clamav_a6o (clamav module)
	- moduleH1	(heuristic module)
	- modulePDF (PDF module)
- Service
	- ArmaditoSvc (analysis service)
- Setup
	- ArmaditoGuard-setup (driver installation)
	- Armadito-db-setup (setup project for module databases)
	- Armadito-setup (setup project for armadito)


Prerequisites:

To build the projects libarmadito and clamav_a6o you will need dependencies libraries and DLLs.

	- Download the windows dependendcies archive "deps.zip" available "put_win_deps_url".
	- Extract the archive content in location : [SOMEWHERE\\armadito-av\\deps] folder.


Driver compilation and signing
******************************

1. Add your certificate to local store.

- Open the Certificate Manager Tool (certmgr.msc)
- Go to "Certificats - utilisateur actuel" >> Personnel >> Certificats
- Right clic on the folder and choose "Toutes les tâches" >> importer.
- Follow the assistant to import your certificate. 


2. Sign sources with your certificate.

- Open the VS project "ArmaditoGuard"
- Open the property page.
- Go to "Propriété de configuration" > Driver Signing.
- In the area "Sign mode" choose "Product Sign".
- In the area "Production Certificate" choose "Select from store" and select your certificate previously added.
- Repete the previous steps for the project "ArmaditoGuard Package"


3. Build the projects "ArmaditoGuard" and "ArmaditoGuard Package"


Compilation Results
*******************

By default, compilation results are located in :
  
   *SOMEWHERE\\armadito-av\\build\\windows\\VS\\Armadito-AV\\out\\[config]*


Sources:
********

- https://msdn.microsoft.com/windows/hardware/drivers/develop/visual-studio-driver-development-environment

.. toctree::



