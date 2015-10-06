var scan;
var selected_dir = "";

// Function who asks libuhuru for a scan
function new_ondemand_scan(){

    if(selected_dir == ""){
		console.log("Nothing selected.");
		return;
	}
	
	console.log("New On-demand scan for directory :" + selected_dir);
    // TODO: Communicate with libuhuru
}

// We add a listener onChange for directory selection
function chooseFile(name) {
	var chooser = document.querySelector(name);
	chooser.addEventListener("change", function(evt) {
	  selected_dir = this.value;
	  console.log(this.value);
	}, false);

	chooser.click();  
}

// We add the listener when loading the page
chooseFile('#fileDialog');