Compilation on Windows
====================

On Windows, you can compile Armadito AV sources with Visual Studio.
This has been tested with Visual Studio 2013. You might have to apply some modifications regarding to which Visual Studio's version you use. 

Open the armadito-av VS project at location : 
 
   *SOMEWHERE\\armadito-av\\build\\windows\\VS\\Armadito-AV\\Armadito-AV.sln*


Driver compilation and signing
******************************

1. Add your certificate to local store.

- Open the Certificate Manager Tool (certmgr.msc)
- Go to "Certificats - utilisateur actuel" >> Personnel >> Certificats
- Right clic on the folder and choose "Toutes les tâches" >> importer.
- Follow the assistant to import your certificate. 


2. Sign sources with your certificate.

- Open the driver visual studio project (ArmaditoGuard)
- Open the property page.
- Go to "Propriété de configuration" > Driver Signing.
- In the area "Sign mode" choose "Product Sign".
- In the area "Production Certificate" choose "Select from store" and select your certificate previously added.


Compilation Results
*******************

By default, compilation results are located in :
  
   *SOMEWHERE\\armadito-av\\build\\windows\\VS\\Armadito-AV\\out\\[config]*

.. toctree::
