Armadito ClamAV module 
======================

Armadito ClamAV module corresponds to the integration of libclamav library within Armadito-av. 
On Windows, after build, a library called **clamav_a6o.dll** will be generated. It aims to interact with both **libarmadito.dll** and **libclamav.dll**.


Prerequisites
-------------

* Microsoft Visual Studio 2013 (Community edition or more)
* Armadito windows dependencies archive (deps.zip)

Uncompress **deps-x.zip** in armadito-av sources root directory. You should have then these exact dependencies paths :

::
    
   SOMEWHERE\armadito-av\deps\glib\...
   SOMEWHERE\armadito-av\deps\json-c\...

Build
-----

Open the armadito-av VS solution at location : 
 
::

   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\Armadito-AV.sln

Then, select **modules\\clamav_a6o** project in Solution Explorer and build it. 


Out
---

Out folder could be one of these :

::

   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\Debug

or 

::

   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\Release

If build has been successful, you should have these files :

::

   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\modules\clamav_a6o.dll
   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\libclamav.dll
   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\libeay32.dll
   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\ssleay32.dll


