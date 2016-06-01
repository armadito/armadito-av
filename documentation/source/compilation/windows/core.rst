Armadito core
=============

Armadito core corresponds to libarmadito library. Symbols exported from this library allows all modules libraries to use the same APIs.
On Windows, after build, a library called **libarmadito.dll** will be generated.

Prerequisites
-------------

* Microsoft Visual Studio 2013 (Community edition or more)
* Armadito windows dependencies archive (deps-x.zip) 

Uncompress **deps-x.zip** in armadito-av sources root directory. You should have then these exact dependencies paths :

::
    
   SOMEWHERE\armadito-av\deps\glib\...
   SOMEWHERE\armadito-av\deps\json-c\...

Build
-----

Open the armadito-av VS solution at location : 
 
::

   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\Armadito-AV.sln

Then, select **Lib-armadito\\libarmadito** project in Solution Explorer and build it. 


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

   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\conf\armadito.conf
   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\glib-2-vs12.dll
   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\gmodule-2.vs12.dll
   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\gthread-2.vs12.dll
   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\libarmadito.dll
