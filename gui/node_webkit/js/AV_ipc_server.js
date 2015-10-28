// Some variables are already defined in AV_ipc_client.js -- same JS context

var scan_id = 77;
var server_path = 'IHM_scan_' + scan_id;
var server;
var path = require('path');

//  For the moment, only one server can be running.
//  For multiple servers, create an array of servers , n_id * server[id]

if(os.platform() == "win32")
{
	server_path= '\\\\.\\pipe\\'+server_path;
}
	
// Configure a keep-alive??
function create_IHM_scan_server(){
	
	server = net.createServer(function(server_socket) { //'connection' listener
	  console.log('client connected');
	  
	  // Normalement global.scan_in_progress = true;
	  // We receive here the scan_progress value from AV
	  // What if we receive nothing ??? ( if AV_Scan crashed ?)
	  
	  server_socket.on('end', function() {
		console.log('client disconnected');
	  });
	  
	  server_socket.on('data', function(data) {
		 console.log("data received on server " );
		 console.log("data received on server : " + data);
		 
		 // Parsing here
		//var buff = new Buffer( data, data_encoding );
	    var scan_report = parse_json_str(data);
		
		if(!scan_report)
		{	
			console.log('Error on json_object received from AV. Waiting for another scan_report msg from AV.');
			server_socket.write("{\"error\",\"JSON scan_report msg parsing error.\"}");
		}
		else
		{	
			if(process_scan_report(scan_report) < 0)
			{
				//server_socket.write("{\"scan_results\",\"error\"}");
				console.error(" process_scan_report error !");
				server_socket.write("{\"error\",\"process_scan_report error\"}");
				
			}
			else{
				server_socket.write("{\"scan_results\",\"ok\"}");
			}
		}
		 
		 
	  });
	  
	});
	
	server.on( 'close', function (){
		console.log(' Closing IHM Scan server ('+scan_id+').');
	});
	
	server.on( 'closed', function (){
		console.log(' IHM Scan server ('+scan_id+') closed.');
	});

	server.listen( server_path , function() { //'listening' listener
	  console.log('Server bound on path : ' + server_path );
	  console.log('Waiting for connections from AV.'  );
	});
	
	return scan_id;
}	
	
function shutdown_scan_server(){
	console.log("shutdown server");
	server.close();
}