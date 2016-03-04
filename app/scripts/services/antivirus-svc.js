'use strict';
/**
 * @ngdoc service
 * @name tatooDesktopApp.MockAv
 * @description
 * # MockAv
 * Provider in the tatooDesktopApp.
 */

 
angular.module('armadito.svc', [])
.service('ArmaditoSVC', ['ArmaditoIPC', function (ArmaditoIPC) {

	var factory = {};
	var ipc_path;
	var clientId;
	var os = require('os');
	var client_sock;
	var av_response = 'none';
	
	factory.threatDataFromAv = function(data){
		
		console.log("[+] Debug :: threatDataFromAv :: Data received from AV :: " + data);
		av_response = 'OK';
		return av_response;
		
	};
	
	factory.setClientPath = function(){
				
		if(os.platform() == "win32"){
			ipc_path = '\\\\.\\pipe\\armadito_ondemand';
		}else{
			ipc_path = '\\\\.\\pipe\\armadito_ondemand';
		}
		return;		
	};

	factory.requestAVstatus = function(callback){
		
		console.log("[+] Debug :: requestAVstatus ::");
		//var request = '{ "av_request":"state", "id":123, "params": {}}';
		var request = { "av_request":"state", "id":123, "params": {}};
		var buffer = new Buffer( JSON.stringify(request), 'ascii' );
		
		//console.log("[+] Debug :: requestAVstatus :: request to send  = "+ request);
		
		// Set ipc path.
		this.setClientPath();	

		ArmaditoIPC.connect2_av(ipc_path,callback);
		
		ArmaditoIPC.write2_av(buffer);
					
		//
		ArmaditoIPC.disconnect2_av();		
		
		// send data
		//var response = ArmaditoIPC.sendAndReceive(clientPath, request);
		console.log("[+] Debug :: requestAVstatus :: antivirus response = " + av_response);
		
		//console.log("[+] Debug :: requestAVstatus :: antivirus response = " + ArmaditoIPC.av_response);		
		
		return ;	
	};
	
	factory.requestAVquarantine = function(callback){
		
		console.log("[+] Debug :: requestAVqurantine ::");		
		var request = { "av_request":"quarantine", "id":123, "params": {"action":"enum"}};
		var buffer = new Buffer( JSON.stringify(request), 'ascii' );
		
		// Set ipc path.
		this.setClientPath();	

		ArmaditoIPC.connect2_av(ipc_path,callback);
		
		ArmaditoIPC.write2_av(buffer);
		
		ArmaditoIPC.disconnect2_av();		
		
		// send data
		//var response = ArmaditoIPC.sendAndReceive(clientPath, request);
		//console.log("[+] Debug :: requestAVstatus :: antivirus response = " + av_response);
		
		//console.log("[+] Debug :: requestAVstatus :: antivirus response = " + ArmaditoIPC.av_response);		
		
		return ;	
	};
	
	factory.requestAVrestore = function(filename,callback){
		
		console.log("[+] Debug :: requestAVqurantine ::");		
		var request = { "av_request":"quarantine", "id":123, "params": {"action":"restore", "fname":filename}};
		var buffer = new Buffer( JSON.stringify(request), 'ascii' );
		
		// Set ipc path.
		this.setClientPath();	

		ArmaditoIPC.connect2_av(ipc_path,callback);
		
		ArmaditoIPC.write2_av(buffer);
		
		ArmaditoIPC.disconnect2_av();		
		
		// send data
		//var response = ArmaditoIPC.sendAndReceive(clientPath, request);
		//console.log("[+] Debug :: requestAVstatus :: antivirus response = " + av_response);
		
		//console.log("[+] Debug :: requestAVstatus :: antivirus response = " + ArmaditoIPC.av_response);		
		
		return ;	
	};
	

	return factory;
}]);
