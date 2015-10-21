var path = "mynamedpipe";
var client_path = '\\\\.\\pipe\\'+ path;
var client_id =7;
var client_connnect_retrying = false;
var client_isRetrying = false;
var client_maxRetries = 3;
var client_retry = 1000;
var data_encoding ='ascii';
var client_socket;
var client_connected = false;

function write_to_AV(){
 	
	var buff_to_write = new Buffer( '{"type":"Buffer","data":[116,101,115,116]}' , data_encoding );
	client_socket.write(buff_to_write, data_encoding, function (){  console.log("data_written.");  } );
}

function connect_to_AV(){

    console.log('Requested connection to ' + client_id+ ' ' + client_path);
	console.log('Connecting client on Unix Socket :' + client_path);
	
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
			
            console.log('## recieved events ##');
			var buff = new Buffer(data, data_encoding);
			console.log(' received buffer ('+data_encoding+'): ' + buff);
			return;
        }
    );
}




