
// test function clickable on IHM
function TestWrite()
{	
	var str = "{\"sitename\" : \"joys of programming\",\"categories\" :[\"c\", [\"c++\", \"c\"], \"java\", \"PHP\"],\"author-details\" : { \"admin\": false, \"name\" : \"Joys of Programming\", \"Number of Posts\" : 10 }}";
	write_to_AV(str);
}


function ask_for_new_scan( new_scan )
{
	
	//var scan = JSON.parse(new_scan);
	
	// Step 1
	var scan_id = create_IHM_scan_server();
	
	
	// Step 2
	console.log(" New scan incoming ("+scan_id+"):" + new_scan);
}