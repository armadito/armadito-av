


function ask_for_new_scan( new_scan, scan_id )
{

	new_scan.scan_path = escape_str(new_scan.scan_path);
	var message = '{ "new_scan_id" : '+ scan_id +', "scan_path": "'+ new_scan.scan_path +'", "param1": "'+new_scan.param1+'", "param2": "'+new_scan.param2+'" }';
	
	// Step 2
	console.log(" New scan_msg to be sent : ("+scan_id+")" + message);
	return write_to_AV(message);
}

function process_AV_response ( AV_response )
{
	// Step 3
	if( AV_response.error ){
		console.error("AV_response sent error msg : "+ AV_response.error );
		return -1;
	}
	
	// Step 3
	if( AV_response.new_scan && AV_response.new_scan == "ok" ){
		if( AV_response.scan_id && AV_response.scan_id > 0){
			console.log("AV_new_scan : OK - " + AV_response.scan_id);
			return 0;
		}	
	}
	
	return -1;
}





