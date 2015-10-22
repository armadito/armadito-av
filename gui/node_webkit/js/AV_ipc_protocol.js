
// test function clickable on IHM
function TestWrite()
{	
	var str = "{\"sitename\" : \"joys of programming\",\"categories\" :[\"c\", [\"c++\", \"c\"], \"java\", \"PHP\"],\"author-details\" : { \"admin\": false, \"name\" : \"Joys of Programming\", \"Number of Posts\" : 10 }}";
	write_to_AV(str);
}


function ask_for_new_scan( new_scan )
{
	// Step 1
	var scan_id = create_IHM_scan_server();
    var scan = parse_json_str(new_scan);
	if( scan == null){
		console.error("Error when parsing : " + new_scan);
		return -1;
	}
	
	scan.path = escape_str(scan.path);
	var message = '{ "scan_id" : '+ scan_id +', "path": "'+ scan.path +'", "param1": "'+scan.param1+'", "param2": "'+scan.param2+'" }';
	
	
	// Step 2
	console.log(" New scan_msg to send : ("+scan_id+")" + message);
	write_to_AV(message);
	
}