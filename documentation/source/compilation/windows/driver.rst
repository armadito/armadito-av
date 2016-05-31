Windows Driver 
=============

Driver Signing
**************

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

Compilation
***********


