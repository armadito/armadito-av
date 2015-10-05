// global definition at start
var tray;

// Load native UI library
var gui = require('nw.gui');

// Create systray icon
function initSystray(){

	// Create a tray icon
	tray = new gui.Tray({ title: 'Uhuru-AV', icon: 'img/uhuru_systray.png' });
	tray.menu = SystrayMenu();
	 
}

function SystrayMenu()
{
	// Give it a menu
	var menu = new gui.Menu();

	var item = new gui.MenuItem({
	  label: "Click me",
	  click: function() {
		console.log("I'm clicked");
	  },
	  key: "s",
	  modifiers: "ctrl-alt",
	});
	
	menu.append(item);
	
	return 	menu;
}

function closeSystray(){
	
	tray.remove();
    tray = null;
}
