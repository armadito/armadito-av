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

function read_from_AV ( data ) {
	
	var buff = new Buffer(data, data_encoding);
	console.log(' received buffer ('+data_encoding+'): -' + buff +'-');
	console.log(' received string -'+buff.toString()+'-');
	
	if(!parse_json_buffer(buff))
	{
		console.log('Error on json_object received from AV. Stopping action.');
		return -1;
	}

	return 0;
}

function connect_to_AV(){

    console.log('Requested connection to ' + client_id+ ' ' + client_path);
	console.log('Connecting client on Socket :' + client_path);
	
	client_socket = net.connect(
		{
			path:client_path
		}
	);

    client_socket.on(
        'error',
        function(err){
            console.log('\n\n######\nerror: '.error, err);
        }
    );

    client_socket.on(
        'connect',
        function(){
            console.log('Successfully connected.')
        }
    );

	// Retry mode TO BE TESTED
    client_socket.on(
        'close',
        function(){
            console.log('connection closed' + client_id + ' ' + client_path);

            if( !client_connnect_retrying || client_retriesRemaining < 1 ){
                client_socket.destroy();
                return;
            }

			// Retrying 
		    client_isRetrying=true;
				
            setTimeout(
                    function(){
                        return function(){
                            client_retriesRemaining--;
                            client_isRetrying=false;
                            connect_to_AV();
                            setTimeout(
                                function(){
                                    if(!client_isRetrying)
                                        client_retriesRemaining = client_maxRetries;
                                },
                                100
                            )
                        }
                    },
             client_retry );
        }
    );

    client_socket.on(
        'data',
        function(data) {
			return read_from_AV(data);
        }
    );
}




