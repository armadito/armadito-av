var client_path = "uhuruAV_ondemand";
var client_id =7;
var client_connnect_retrying = false;
var client_isRetrying = false;
var client_maxRetries = 3;
var client_retry = 1000;
var data_encoding ='ascii';
var client_socket;
var client_connected = false;
var net = require('net');
var os = require('os');

// Named pipes on win32
console.log("OS type : " + os.type());
console.log("OS platform : " + os.platform());
console.log("Arch : " + os.arch());
console.log("OS Release number : "+ os.release());
if(os.platform() == "win32")
{
	client_path = '\\\\.\\pipe\\'+ client_path;
}

// Function called when data is received from AV
function read_from_AV ( data ) {
	
	var buff = new Buffer(data, data_encoding);
	console.log(' received string -'+buff.toString()+'-');
	
	var AV_response = parse_json_buffer(buff);
	if(!AV_response){	
		console.log('Error on json_object received from AV. Stopping action.');
		return -1;
	}else{
		
		if( process_AV_response(AV_response) < 0){ // if response not valid
			// Step 3
			client_socket.destroy();
			shutdown_scan_server();
			global.new_scan = null;
		}
		else{ 
		    // Step 3
			console.log("AV_response OK. Closing socket.");
			client_socket.destroy();
			
			// We know that a scan is in progress only when AV sent back "ok"
			// Now waiting for connections from AV on scan_server.
			// Scan_in_progress = named pipe server running
			global.scan_in_progress = true;
		}
	}
	
	return 0;
}

// Write a JSON formatted string on socket to AV
function write_to_AV( data ){
 	
	var buff_to_write = new Buffer( data, data_encoding );
	
	// We check json_object before writing on socket to AV
	if(!parse_json_buffer(buff_to_write))
	{
		console.log('Error on json_object what sould be sent to AV. Stopping action.');
		return -1;
	}
	
	client_socket.write(buff_to_write, data_encoding, function (){  console.log("data_written.");  } );
	return 0;
}

// Try to connect to AV named pipe or linux
function connect_to_AV(){

    console.log('Requested connection to ' + client_id+ ' ' + client_path);
	console.log('Connecting client on Socket :' + client_path);
	
	client_socket = net.connect({path:client_path});

    client_socket.on(
        'error',
        function(err){
            console.error('\n\n######\nerror: '.error, err);
			return -1;
        }
    );

    client_socket.on(
        'connect',
        function(){
            console.log('Successfully connected.');
			return 0;
        }
    );

    client_socket.on(
        'close',
        function(){
            console.log('connection closed' + client_id + ' ' + client_path);
			client_socket.destroy();
			return;
			
			// may retry here ?
        }
    );

    client_socket.on(
        'data',
        function(data) {
			return read_from_AV(data);
        }
    );
	
	return 0;
	
}




