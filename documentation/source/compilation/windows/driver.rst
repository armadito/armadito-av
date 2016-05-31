Armadito Windows Driver 
======================

Armadito Windows Driver is responsible of on-access protection of Armadito antivirus.

Prerequisites
-------------

* Microsoft Visual Studio 2013 (Community edition or more)
* Armadito windows dependencies archive (deps.zip)
* Windows Driver Kit 8.1

To get Windows Driver Kit 8.1 : <https://www.microsoft.com/en-us/download/details.aspx?id=42273>

.. warning:: Windows Driver Kit 8.1 goes **only** with MS Visual Studio 2013. You must get the WDK compatible to your Visual Studio version. 


Driver Signing
-------------

Add your certificate to local store
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Open the Certificate Manager Tool (certmgr.msc)
- Go to **Certificates - Actual User** > **Personal** > **Certificates**
- Right-click on the folder and choose **All tasks** > **Import**
- Then, follow the assistant to import your certificate. 


Sign with your certificate
^^^^^^^^^^^^^^^^^^^^^^^^^^

- Open Armadito-av solution in Visual Studio.
- Right-click on project **ArmaditoGuard** and select **Properties**.
- Go to **Configuration Properties** > **Driver Signing** > **General**.
- **Sign mode** > **Product Sign**.
- **Production Certificate** > **Select from store** and select your certificate previously added.
- Repeat the previous steps for the project **ArmaditoGuard Package**.

Build
-----

Open the armadito-av VS solution at location : 
 
::

   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\Armadito-AV.sln

Firstly, select **Driver\ArmaditoGuard** project in Solution Explorer and build it. 

Then, select **Driver\\ArmaditoGuard Package** project in Solution Explorer and build it. 

Finally, select **Setup\\ArmaditoGuard-setup** project in Solution Explorer and build it.

Out
---

Out folder could be one of these :

::

   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\Debug

or 

::

   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\Release

If build has been successful, you should have this file :

::

   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\driver\armaditoguard.cat
   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\driver\armaditoguard.inf
   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\driver\armaditoguard.sys
   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\driver\ArmaditoGuard-setup.exe

.. toctree::


