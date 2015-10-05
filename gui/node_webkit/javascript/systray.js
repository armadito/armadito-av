// Create systray icon

// Load native UI library
var gui = require('nw.gui');

// Create a tray icon
var tray = new gui.Tray({ title: 'Tray', icon: 'img/icon.png' });

// Give it a menu
var menu = new gui.Menu();
menu.append(new gui.MenuItem({ type: 'checkbox', label: 'box1' }));
tray.menu = menu;

// Remove the tray
tray.remove();
tray = null;