'use strict';
/**
 * @ngdoc service-ipc
 * @name tatooDesktopApp.ipc
 * @description
 * # armadito-ipc
 * Provider in the tatooDesktopApp.
 */
 
angular.module('armadito.ipc', [])
.service('ArmaditoIPC', function () {
	
	var factory = {};
	var av_response;
	var client_socket;
	var client_sock;
	var net = require('net');
	
	//var factory.client_sock;
		
	factory.sendRequest = function(request){
						
		var buffer = new Buffer( JSON.stringify(request), 'ascii' );
		console.log("[+] Debug :: [IPC] requestRequest :: send request = " + buffer);	
				
		return;
		
	};
	
	factory.respHandler = function(data){
		
		var buffer = new Buffer(data, 'ascii');		
		//console.log("[+] Debug :: [IPC] respHandler :: response received = " + buffer);
		
		global.av_response = buffer;
						
		return buffer;
	};
	
	
	// ----------------------------------------------
	factory.connect2_av = function(ipc_path, callback){

		//console.log("[+] Debug :: [IPC] sendAndReceive :: Try to connect to Armadito service :: " + ipc_path);
		
		client_sock = net.connect({path:ipc_path});
				
		client_sock.on('error',function(err){
			console.log("[-] Error :: connect2_av :: connection failed :: " + err);
			return;
		});
	
		client_sock.on('connect',function(){
			console.log("[+] Debug :: connect2_av :: connected to ["+ ipc_path +"] successfully ! :: ");			
			return;
		});
		
		client_sock.on('data',function(data){
			//console.log("dans la fonction connect_AV : "+ data);			
			return callback(data);
		});
		
		client_sock.on('close',function(){
			client_sock.destroy();
			console.log("[+] Debug :: connect2_av :: closing connection to ["+ ipc_path +"] ...:: ");
			return;
		});
		
		return 1;	
	};
	
	// ----------------------------------------------
	factory.write2_av = function(data2send){
		
		client_sock.write(data2send, 'ascii',function(){
			console.log("[+] Debug :: write2_av :: data send to av ::" + client_sock);
		});
		
		
		return;		
	}
	
	factory.disconnect2_av = function(){
		client_sock.end();
		client_sock.close;
	};
		

	return factory;
});
	
