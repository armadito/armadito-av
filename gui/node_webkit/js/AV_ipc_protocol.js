


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

function process_scan_report( scan_report )
{
	// Step 4
	if( scan_report.error ){
		console.error("AV scan_report error msg : "+ scan_report.error );
		return -1;
	}

	if( scan_report.scan_progress && scan_report.scan_progress > 0){
		console.log (" scan_progress = " +  scan_report.scan_progress);
		scan_progress_on_change(scan_report.scan_progress);
	}
	
	if( scan_report.scan_file_path && scan_report.scan_file_path != "null" ){
	
		console.log(" scanned file : " + scan_report.scan_file_path);
		update_scan_file_path(scan_report.scan_file_path);
	}

	
	return 0;
}






