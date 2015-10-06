
function server_on_connect( socket )
{
	var message = create_message();
	
	ipc.server.emit(
		socket,
		message
	);		
}

function server_on_data_received( data , socket )
{
	ipc.log('got a message'.debug, data,data.toString());
	ipc.server.emit(
		socket,
		'goodbye mf'
	);
}

function client_on_connect ()
{
	ipc.log('## connected to world ##'.rainbow, ipc.config.delay);
	ipc.of.world.emit(
		'hello my dear'
	)
}

function client_on_disconnect ()
{
	ipc.log('## disconnected from world ##'.notice, ipc.config.delay);
}

function client_on_data_received (data) 
{
	ipc.log('got a message from world : '.debug, data,data.toString());
}
