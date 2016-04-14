#!/bin/bash

# https://help.ubuntu.com/community/Samba/SambaClientGuide

mkdir -p ~/mnt/trusty_cifs
mkdir -p ~/mnt/precise_cifs

sudo umount -l ~/mnt/trusty_cifs
sudo umount -l ~/mnt/precise_cifs

sudo mount -t cifs //n5samba/samba_shared ~/mnt/trusty_cifs -o username=uhuru,noexec
sudo mount -t cifs //n5precise32a/share ~/mnt/precise_cifs -o username=uhuru,noexec
