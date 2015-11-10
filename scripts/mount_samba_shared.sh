#!/bin/bash

# https://help.ubuntu.com/community/Samba/SambaClientGuide

mkdir ~/mnt
sudo mount -t cifs //n5samba/samba_shared ~/mnt -o username=uhuru,noexec

