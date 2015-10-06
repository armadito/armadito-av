// global definitions
var ipc = require('node-ipc');
ipc.config.id   = 'world';
ipc.config.retry= 1500;
ipc.config.rawBuffer=true;
ipc.config.encoding='ascii';

function start_world_server_ipc(){

	ipc.serveNet(
		function(){
			ipc.server.on(
				'connect',
				function(socket){
					ipc.server.emit(
						socket,
						'hello'
					);
				}
			);

			ipc.server.on(
				'data',
				function(data,socket){
					ipc.log('got a message'.debug, data,data.toString());
					ipc.server.emit(
						socket,
						'goodbye'
					);
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
					ipc.log('## connected to world ##'.rainbow, ipc.config.delay);
					ipc.of.world.emit(
						'hello'
					)
				}
			);

			ipc.of.world.on(
				'data',
				function(data){
					ipc.log('got a message from world : '.debug, data,data.toString());
				}
			);
		}
	);

}

