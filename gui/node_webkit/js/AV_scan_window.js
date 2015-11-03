var scan;
var selected_dir = "";
var param1 = "";
var param2 = "";

var gui = require('nw.gui');


// Function who asks libuhuru for a scan
function new_ondemand_scan(){

	if( global.new_scan != null ){
		console.log("Already processing scan.");
		return;
	}

    if(selected_dir == ""){
		console.log("Nothing selected.");
		return;
	}
	
	console.log("New On-demand scan for directory :" + selected_dir);
	console.log("Scan params :\n param1 : "+param1+" \n param2 : "+param2 ); 
	
	// transfer parameters to new window by global variable

	
	var new_scan = '{ "scan_path" : "'+selected_dir+'", "param1" : "'+param1+'", "param2": "'+param2+'"}'; 
	global.new_scan = new_scan;
	
	var win = gui.Window.open('AV_scan_results_window.html');
	win.on( 'closed', function (){
		console.log("on-demand-scan results window onclosed!");
		global.new_scan = null;
    });

	//win.maximize();
	
}

// Examples of AV params
function update_param1() 
{
   var tmpElement = document.getElementById('param1');
   param1 = tmpElement.options[tmpElement.selectedIndex].value; 
   console.log("Updated param2 :" + param1);		
}

function update_param2() 
{
   var tmpElement = document.getElementById('param2');
   param2 = tmpElement.options[tmpElement.selectedIndex].value; 
   console.log("Updated param2 :" + param2);	
}

// We add a listener onChange for directory selection
function chooseFile(name) {
	var chooser = document.querySelector(name);
	chooser.addEventListener("change", function(evt) {
	  selected_dir = escape_str(this.value);
	  console.log(this.value);
	}, false);

	chooser.click();  
}

// We add the listener when loading the page
chooseFile('#fileDialog');