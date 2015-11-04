
// "On-demand scan"
function TrayMenu1_onclick()
{
	console.log("TrayMenu1 Clicked");
	var win = gui.Window.open('pages/AV_scan_window.html');
	
}

// "AV Updates state"
function TrayMenu2_onclick()
{
	// TO CHANGE
	console.log("TrayMenu2 Clicked");
	var win = gui.Window.open('pages/update_status_window.html');
}

// "About"
function TrayMenu3_onclick()
{
	// TO CHANGE
	console.log("TrayMenu3 Clicked");
	var win = gui.Window.open('pages/about_window.html');
}

// "Stop AV"
function TrayMenu4_onclick()
{
	// TO CHANGE
	console.log("TrayMenu4 Clicked. Closing AV.");
	closeWindow();
}
