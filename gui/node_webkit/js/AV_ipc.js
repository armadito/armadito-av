// global definitions

var net = require('net');
var server;

function start_tcp_server() {
	
	// A port value of zero will assign a random port.
	var listening_port = 0;
	
	server = net.createServer(function(connection) { //'connection' listener
	  console.log('client connected');
	  
	  connection.on('end', function() {
		console.log('client disconnected');
	  });
	  
	  connection.write('hello\r\n');
	  connection.pipe(connection);
	});
	
	server.on('error', function (e) {
	  if (e.code == 'EADDRINUSE') {
		console.log('Address in use, retrying...');
		setTimeout(function () {
		  server.close();
		  server.listen(listening_port, on_server_listening);
		}, 1000);
	  }
	});
	
	server.listen(listening_port, on_server_listening );

}

function stop_tcp_server(){

	server.close();
}




