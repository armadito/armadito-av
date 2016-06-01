/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito gui.

Armadito gui is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito gui is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Armadito gui.  If not, see <http://www.gnu.org/licenses/>.

***/

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
	var os = require('os');
	var net = require('net');
	var fs = require('fs');
	
	//var factory.client_sock;
	
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
			callback(data);
			//close socket
			client_sock.end();
			return 1;
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
			console.log("[+] Debug :: write2_av :: data sent to av");
		});
		return;
	};
	
	factory.disconnect2_av = function(){
		client_sock.end();
		client_sock.close;
	};

	// ----------------------------------------------
	factory.createUIServer = function(ipc_path,callback){

		if(os.platform() != "win32"){ // linux only
		   this.cleanSocket(ipc_path);
		}

		console.log("[+] Debug :: Create server :::::: ", ipc_path);

		var server = net.createServer(function(server_sock){

			console.log("[+] Debug :: client connected!");

			server_sock.on('end', function(){
				console.log("[+] Debug :: client disconnected from server.");
			});

			server_sock.on('error', function(err){
				console.log("[-] Error :: server error :: ",err);
				if (err.code == 'EADDRINUSE') {
					console.log('Address in use, retrying...');
					setTimeout(function () {
						server_sock.close();
						server_sock.listen(ipc_path, HOST);
					}, 1000);
				}
			});

			server_sock.on( 'close', function (){
          		   console.log("[+] Debug :: closing ui socket server");
        	});

			server_sock.on('data', function(data){
				//console.log("[+] Debug :: data received ::"+ data);
				var response = { "ihm_response":"state", "id":0, "status":0 };
				try {
				  var buffer = new Buffer( JSON.stringify(response), 'ascii' );
				  server_sock.write(buffer);
				  callback(data);
				  buffer = null;
				}
				catch(e){
				   console.log("Exception ondata JSON stringify :" + e);
				}
			});

		});

		// console.log(":::::::::::: SERVER >>>>>",server);

		// console.log("[+] Debug :: CLeaning socket server path...\n");

		fs.access(ipc_path, fs.F_OK, function(err){

			console.log(err ? 'no socket file---------!' : 'file exists already!!------------');
			if(!err) {
				fs.unlinkSync(ipc_path);

			    console.log("---------------------------------------------------------\n");
			}

		});

		server.listen(ipc_path, function(){
			console.log("[+] Debug :: server listening on ::", ipc_path);
		});

		// console.log(":::::::::::: SERVER >>>>>",server);

		return server;

	};


	factory.closeServer = function(server){

		// Try to close the server.
		console.log("[+] Debug :: Trying to stop the socket server...\n");
		server.close();

	}

	factory.cleanSocket = function(path) {
	    try  {
		if (fs.statSync(path).isSocket())
		    fs.unlinkSync(path);
	    }
	    catch (e) {
		if (e.code == 'ENOENT') { // no such file or directory. File really does not exist
		    console.log("socket ", path, " does not exist.");
		    return;
		}

		console.log("Exception fs.statSync (" + path + "): " + e);
		throw e; // something else went wrong, we don't have rights, ...
	    }
	}


	return factory;
	
});
	
