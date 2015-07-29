#!/usr/bin/env python
import os
import urllib
import dbus
from gi.repository import Nautilus, GObject
from gettext import ngettext, locale, bindtextdomain, textdomain

class UhuruExtension(Nautilus.MenuProvider, GObject.GObject):

    def __init__(self):
        self._dbus = dbus.SessionBus()

    def menu_activate_cb(self, menu, file):
        with open("/home/francois/testnautilus.txt", "a") as f:
            f.write(file.get_name())
            f.write("\n")
#            f.write(file.get_parent_location())
#            f.write("\n")
            f.write(file.get_uri()[7:] + "\n")
            f.write("\n")
        filename = urllib.unquote(file.get_uri()[7:])
        uhuru = self._dbus.get_object("org.uhuru.ScanService", "/")
        iface = dbus.Interface(uhuru, "org.uhuru.ScanApplication")
        iface.scan(filename)

    def get_file_items(self, window, files):
        if len(files) != 1:
            return
        
        file = files[0]
        
        item = Nautilus.MenuItem(name='NautilusPython::scanwithuhuru',
                                 label='Scan with Uhuru...',
                                 tip='Scan %s with Uhuru...' % file.get_name(),
                                 icon='uhuru')
        item.connect('activate', self.menu_activate_cb, file)
        return item,

    def get_background_items(self, window, file):
        item = Nautilus.MenuItem(name='NautilusPython::scanwithuhuru',
                                 label='Scan with Uhuru...',
                                 tip='Scan %s with Uhuru...' % file.get_name(),
                                 icon='uhuru')
        item.connect('activate', self.menu_activate_cb, file)
        return item,
