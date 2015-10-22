
// Parse a node.js Buffer 
function parse_json_buffer ( buffer ){
	
	// Note - JSON.parse can tie up the current thread because it is a synchronous method. So if you are planning to parse big JSON objects use a streaming json parser.		
	var json_object = null;

	try {
		json_object = JSON.parse(buffer.toString('ascii'));
	}
	catch(e){
		console.error("Parsing error:", e); 
		return null;
	}
	
	return json_object;
}

// Parse a node.js String
function parse_json_str ( str ){
	
	// Note - JSON.parse can tie up the current thread because it is a synchronous method. So if you are planning to parse big JSON objects use a streaming json parser.		
	var json_object = null;

	try {
		json_object = JSON.parse(str);
	}
	catch(e){
		console.error("Parsing error:", e); 
		return null;
	}
	
	return json_object;
}
