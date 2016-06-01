Armadito module PDF
===================

Armadito module PDF is an heuristic module for PDF documents analysis.
On Windows, after build, a library called **modulePDF.dll** will be generated.


Prerequisites
-------------

* Microsoft Visual Studio 2013 (Community edition or more)
* Armadito windows dependencies archive (deps.zip)
* Armadito PDF sources from github repository

Uncompress **deps-x.zip** in armadito-av sources root directory. You should have then these exact dependencies paths :

::
    
   SOMEWHERE\armadito-av\deps\glib\...
   SOMEWHERE\armadito-av\deps\json-c\...

In order to get Armadito PDF sources, you have to clone the armadito-pdf public repository :

::
   
   cd SOMEWHERE\armadito-av\modules\modulePDF
   git clone https://github.com/armadito/armadito-pdf -b DEV


Build
-----

Open the armadito-av VS solution at location : 
 
::

   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\Armadito-AV.sln

Then, select **modules\\modulePDF** project in Solution Explorer and build it. 


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

   SOMEWHERE\armadito-av\build\windows\VS12\Armadito-AV\out\[build_mode]\modules\modulePDF.dll

