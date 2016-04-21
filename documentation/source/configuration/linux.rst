Configuration on Linux
======================

Armadito AV's configuration on Linux is stored in two files :
 
* /etc/armadito/armadito.conf
* /etc/armadito/conf.d/on-access-linux.conf

.. note:: If you have compiled from sources, these configuration files are in your PREFIX directory.

.. warning:: Configuration presented in this document is used for illustration purposes only. 

On-demand scan 
~~~~~~~~~~~~~~

You are able to configure how on-demand scan works in **/etc/armadito/armadito.conf** :

::

   [on-demand]
   white-list-dir = "/boot"; "/dev"; "/etc"; "/proc"; "/run"; "/sys"; "/var"
   mime-types="*"
   modules="clamav"; "module5_2"
   max-size = 10048576 

* **white-list-dir** : list of directories excluded from on-demand scan.
* **mime-types** : MIME types of files scanned during on-demand scan.
* **modules** : Modules used by on-demand scan.
* **max-size** : Maximum size of scanned files during on-demand scan. 

On-access scan
~~~~~~~~~~~~~~

Linux Armadito AV's on-access scan mainly relies on fanotify API. 
You can find further information on how it works by reading official man page : `fanotify man7 <http://man7.org/linux/man-pages/man7/fanotify.7.html>`_.

Configuring on-access scan can be done by modifying **/etc/armadito/conf.d/on-access-linux.conf** :

::
 
   [on-access]
   enable=1
   enable-permission=1
   enable-removable-media=1
   mount="/home"
   directory="/var/tmp"; "/tmp"
   white-list-dir = "/bin"; "/boot"; "/dev"; "/etc"; "/lib"; "/lib32"; "/lib64"
   mime-types = "application/x-executable"; "application/pdf"; "application/zip"
   modules = "clamav"
   max-size = 10048576 


* **enable** : enable (1) or disable (0) on-access scan. 
* **enable-permission** : enable (1) or disable (0) permission check. 

  * If **enabled**, files detected as malicious will be blocked by Armadito AV. 
  * If **disabled**, files detected as malicious will only be notified.
* **enable-removable-media** : enable (1) or disable (0) removable media monitoring. 

  * If **enabled**, removable media mount points will be added on the fly to the monitoring list.
* **mount** : list of directories that will be monitored by mount points. I.e. using FAN_MARK_MOUNT.
* **directory** : list of directories that will be monitored by recursively marking all subdirectories. 
* **white-list-dir** : list of directories excluded from on-demand scan.
* **mime-types** : MIME types of files scanned during on-demand scan.
* **modules** : Modules used by on-demand scan.
* **max-size** : Maximum size of scanned files during on-demand scan. 


Virus Alerts
~~~~~~~~~~~~

When a virus is detected by Armadito AV, an alert report is generated and stored in a defined location.
This can be configured by modifying **/etc/armadito/armadito.conf** :

:: 

   [alert]
   alert-dir = "/var/spool/armadito"

* **alert-dir** : directory where scan alerts will be stored.

Quarantine
~~~~~~~~~~

To isolate infected files, Armadito AV can put detected files in quarantine.
**/etc/armadito/armadito.conf** contains configuration about quarantine :

:: 
   
   [quarantine]
   enable = 0
   quarantine-dir = "/var/lib/armadito/quarantine"

* **enable** : enable (1) or disable (0) quarantine.
* **quarantine-dir** : directory where will be moved files putted in quarantine.


.. toctree::


