#!/bin/bash

# https://help.ubuntu.com/community/Samba/SambaClientGuide

mkdir -p ~/mnt/trusty
mkdir ~/mnt/precise
umount -l ~/mnt/trusty
umount -l ~/mnt/precise

sudo mount -t cifs //n5samba/samba_shared ~/mnt/trusty -o username=uhuru,noexec
sudo mount -t cifs //n5precise32a/share ~/mnt/precise -o username=uhuru,noexec
