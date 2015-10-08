// global definitions

var net = require('net');
var tcp_server;

function start_tcp_server() {
	
	var listening_port = 8125;
	
	tcp_server = net.createServer(function(connection) { //'connection' listener
	  console.log('client connected');
	  
	  connection.on('end', function() {
		console.log('client disconnected');
	  });
	  
	  connection.write('hello\r\n');
	  connection.pipe(connection);
	});
	
	tcp_server.listen(listening_port, function() { //'listening' listener
	  console.log('server listening on port ' + listening_port );
	});
}

function stop_tcp_server(){

	tcp_server.close();
}
