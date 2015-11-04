function hideWindow(){
	var win = gui.Window.get();
	win.hide();
}

function closeWindow(){
	
	console.log(" close window ");

	// Close systray 
	//tray.remove();
        //tray = null;
	
	// Close Window
	var win = gui.Window.get();
	win.close();	
}
