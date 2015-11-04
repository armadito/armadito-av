// global definition at start
var tray;

// Load native UI library
var gui = require('nw.gui');

// Create systray icon
function initSystray(){
 
	console.log("Init systray !\n");

	var ffi = require("ffi");
	/*var libm = new ffi.Library("libm", { "ceil": [ "double", [ "double" ] ] });
	libm.ceil(1.5); // 2

	console.log(" libm ceil(1.5) = "+ libm);*/

	// Create a tray icon
	tray = new gui.Tray({ title: 'Uhuru-AV', icon: 'img/uhuru_systray.png' });
	tray.menu = SystrayMenu();
}

// Create Menu and return it
function SystrayMenu(){
	
	var menu = new gui.Menu();

	var TrayMenu1 = new gui.MenuItem({
	  label: "On-demand scan",
	  click: TrayMenu1_onclick,
	  key: "s",
	  modifiers: "ctrl-alt",
	});
	
	var TrayMenu2 = new gui.MenuItem({
      label: "AV Updates state",
	  click: TrayMenu2_onclick,
	  key: "s",
	  modifiers: "ctrl-alt",
	});
	
	var TrayMenu3 = new gui.MenuItem({
      label: "About",
	  click: TrayMenu3_onclick,
	  key: "s",
	  modifiers: "ctrl-alt",
	});
	
	var TrayMenu4 = new gui.MenuItem({
      label: "Close",
	  click: TrayMenu4_onclick,
	  key: "s",
	  modifiers: "ctrl-alt",
	});
	
	menu.append(TrayMenu1);
	menu.append(TrayMenu2);
	menu.append(TrayMenu3);
	menu.append(TrayMenu4);
	
	return 	menu;
}

function closeSystray(){
	
	tray.remove();
        tray = null;
}

// automatically init systray icon at start
initSystray();


