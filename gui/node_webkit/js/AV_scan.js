var scan;
var selected_dir = "";
var param1 = "";
var param2 = "";

// Function who asks libuhuru for a scan
function new_ondemand_scan(){

    if(selected_dir == ""){
		console.log("Nothing selected.");
		return;
	}
	
	console.log("New On-demand scan for directory :" + selected_dir);
	console.log("Scan params :\n param1 : "+param1+" \n param2 : "+param2 ); 
	
	
	window.open('on-demand_scan_results.html', '_blank', 'screenX=0,screenY=0,width=100,height=100');
	
    // TODO: Communicate with libuhuru
	
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
	  selected_dir = this.value;
	  console.log(this.value);
	}, false);

	chooser.click();  
}

// We add the listener when loading the page
chooseFile('#fileDialog');