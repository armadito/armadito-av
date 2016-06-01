Configuration on Windows
========================

Armadito AV's configuration on Windows can be modified in the following file :
 
* <**install_dir**>\\Armadito-av\\conf\\armadito.conf

.. note:: By default, **install_dir** is *C:\\Program Files\\Teclib*.

On-demand scan 
~~~~~~~~~~~~~~

You are able to configure how on-demand scan works in **<install_dir>\\Armadito-av\\conf\\armadito.conf** :

::

   [on-demand]
   white-list-dir = "C:\\wl-dir1\\"; "C:\\wl-dir2\\"
   mime-types="*"
   modules="clamav"; "moduleH1"
   max-size = 10048576 

* **white-list-dir** : list of directories excluded from on-demand scan (not yet implemented).
* **mime-types** : MIME types of files scanned during on-demand scan.
* **modules** : Modules used by on-demand scan.
* **max-size** : Maximum size of scanned files during on-demand scan. 


On-access scan 
~~~~~~~~~~~~~~

You are able to configure how on-access scan works in **<install_dir>\\Armadito-av\\conf\\armadito.conf** :

::

   [on-access]
   enable = 0
   mime-types="*"
   modules="clamav"

* **enable** : enable (1) or disable (0) on-access scan.
* **mime-types** : MIME types of files scanned during on-access scan.
* **modules** : Modules used by on-access scan.


Virus Alerts
~~~~~~~~~~~~

Not yet implemented.

Quarantine
~~~~~~~~~~

To isolate infected files, Armadito AV can put detected files in quarantine.
**<install_dir>\\Armadito-av\\conf\\armadito.conf** contains configuration about quarantine :

:: 
   
   [quarantine]
   enable = 0
   quarantine-dir = "quarantine"

* **enable** : enable (1) or disable (0) quarantine.
* **quarantine-dir** : sub directory where will be moved files put in quarantine.


.. toctree::


