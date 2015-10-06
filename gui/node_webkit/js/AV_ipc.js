// global definitions
var ipc = require('node-ipc');
ipc.config.id   = 'world';
ipc.config.retry= 1500;
ipc.config.rawBuffer=true;
ipc.config.encoding='ascii';
ipc.config.networkPort='8077';

function start_world_server_ipc(){

	ipc.serveNet(
		function(){
			ipc.server.on(
				'connect',
				function(socket){
					server_on_connect(socket);
				}
			);

			ipc.server.on(
				'data',
				function(data,socket){
					server_on_data_received(data,socket);
				}
			);
		}
	);

	ipc.server.start();

}

function hello_world_ipc (){
	
	ipc.connectToNet(
		'world',
		function(){
			ipc.of.world.on(
				'connect',
				function(){
					client_on_connect();	
				}
			);

			ipc.of.world.on(
				'disconnect',
				function(){
					client_on_disconnect();
				}
			);
			
			ipc.of.world.on(
				'data',
				function(data){
					client_on_data_received(data);
				}
			);
		}
	);

}

