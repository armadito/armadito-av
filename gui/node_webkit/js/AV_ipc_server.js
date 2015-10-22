// Some variables are already defined in AV_ipc_client.js -- same JS context

var scan_id = 7744;
var server_path = "IHM_scan_" + scan_id;

if(os.platform() == "win32")
{
	server_path = '\\\\.\\pipe\\'+ server_path;
}
	
function create_IHM_scan_server(){
	
	var server = net.createServer(function(connection) { //'connection' listener
	  console.log('client connected');
	  
	  connection.on('end', function() {
		console.log('client disconnected');
	  });
	  
	  connection.write('Well received from u, AV !');
	  connection.pipe(connection);
	});

	server.listen( server_path , function() { //'listening' listener
	  console.log('Server bound on path : ' + server_path );
	  console.log('Waiting for connections from AV.'  );
	});
	
	return scan_id;
}	
	
