
var ipc;

function start_world_server_ipc(){

	var ipc=require('node-ipc');

	/***************************************\
	 * 
	 * You should start both hello and world
	 * then you will see them communicating.
	 * 
	 * *************************************/

	ipc.config.id   = 'world';
	ipc.config.retry= 1500;

	ipc.serve(
		function(){
			ipc.server.on(
				'app.message',
				function(data,socket){
					//ipc.log('got a message from'.debug, (data.id).variable, (data.message).data);
					ipc.server.emit(
						socket,
						'app.message',
						{
							id      : ipc.config.id,
							message : data.message+' world!'
						}
					);
				}
			);
		}
	);

	ipc.server.define.listen['app.message']='This event type listens for message strings as value of data key.';

	ipc.server.start();
}

function hello_world_ipc (){
	
	// Hello-client
    console.log("Test IPC.");

	var ipc=require('node-ipc');

	/***************************************\
	 *
	 * You should start both hello and world
	 * then you will see them communicating.
	 *
	 * *************************************/

	ipc.config.id   = 'hello';
	ipc.config.retry = 1000;

	ipc.connectTo(
		'world',
		function(){
			ipc.of.world.on(
				'connect',
				function(){
					ipc.log('## connected to world ##'.rainbow, ipc.config.delay);
					ipc.of.world.emit(
						'app.message',
						{
							id      : ipc.config.id,
							message : 'hello'
						}
					)
				}
			);
			ipc.of.world.on(
				'disconnect',
				function(){
					ipc.log('disconnected from world'.notice);
				}
			);
			ipc.of.world.on(
				'app.message',
				function(data){
					ipc.log('got a message from world : '.debug, data);
				}
			);

			console.log(ipc.of.world.destroy);
		}
	);
	
}

