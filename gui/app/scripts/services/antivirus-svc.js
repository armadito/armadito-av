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
 * @ngdoc service
 * @name tatooDesktopApp.MockAv
 * @description
 * # MockAv
 * Provider in the tatooDesktopApp.
 */

var fs = require('fs');

angular.module('armadito.svc', [])
.service('ArmaditoSVC', ['ArmaditoIPC', function (ArmaditoIPC) {

	var factory = {};
	var ipc_path;
	var server_ipc_path;
	var scan_server_ipc_path;
	var clientId;
	var client_sock;
	var av_response = 'none';

	var os = require('os');

	factory.threatDataFromAv = function(data){
		
		//console.log("[+] Debug :: threatDataFromAv :: Data received from AV :: " + data);
		av_response = 'OK';
		data = null;
		return av_response;
	};
	
	factory.setClientPath = function(){
				
		if(os.platform() == "win32"){
			ipc_path = '\\\\.\\pipe\\armadito_ondemand';
		}else{
			ipc_path = '/tmp/.armadito-daemon';
		}
		return;		
	};

	factory.setServerPath = function(){
				
		if(os.platform() == "win32"){
			server_ipc_path = '\\\\.\\pipe\\armadito-UI';
		}else{
			server_ipc_path = '/tmp/.armadito-ui';
		}
		return;		
	};


	factory.setScanServerPath = function(){
				
		if(os.platform() == "win32"){
			scan_server_ipc_path = '\\\\.\\pipe\\armadito-ui-scan';
		}else{
			scan_server_ipc_path = '/tmp/.armadito-ui-scan';
		}
		return;	

	};

	factory.startNotificationServer = function(callback){

		var server;

		console.log("[+] Debug :: startNotificationServer ::");
		// set server ipc path according to OS version.
		this.setServerPath();

		server = ArmaditoIPC.createUIServer(server_ipc_path, callback);		

	};


	// Launch a scan.
	factory.launchScan = function(path_to_scan, callback){

		console.log("[+] Debug :: launchScan :: start scan on path", path_to_scan);

		// Set client ipc path.
		this.setClientPath();

		// Set server ipc path.
		this.setServerPath();

		var request = { "av_request":"scan", "id":123, "params": {ui_ipc_path: server_ipc_path , path_to_scan: path_to_scan}};
		var buffer = new Buffer( JSON.stringify(request), 'ascii' );

		ArmaditoIPC.connect2_av(ipc_path,callback);
		ArmaditoIPC.write2_av(buffer);
		ArmaditoIPC.disconnect2_av();

		// TODO close server ?
		//this.cleanALL();
	};

	// Cancel current scan (if any).
	factory.cancelScan = function(callback){

		console.log("[+] Debug :: cancelScan ");

		var request = { "av_request":"scan_cancel", "id":123, "params": {}};
		var buffer = new Buffer( JSON.stringify(request), 'ascii' );

		ArmaditoIPC.connect2_av(ipc_path,callback);
		ArmaditoIPC.write2_av(buffer);
		ArmaditoIPC.disconnect2_av();
	};
		

	// Request antivirus status.
	factory.requestAVstatus = function(callback){
		
		console.log("[+] Debug :: requestAVstatus ::");		
		var request = { "av_request":"state", "id":123, "params": {}};
		var buffer = new Buffer( JSON.stringify(request), 'ascii' );
		
		//console.log("[+] Debug :: requestAVstatus :: request to send  = "+ request);
		
		// Set ipc path.
		this.setClientPath();	

		ArmaditoIPC.connect2_av(ipc_path,callback);
		
		ArmaditoIPC.write2_av(buffer);
					
		//ArmaditoIPC.disconnect2_av();
		
		// send data
		//var response = ArmaditoIPC.sendAndReceive(clientPath, request);
		// console.log("[+] Debug :: requestAVstatus :: antivirus response = " + av_response);
		
		//console.log("[+] Debug :: requestAVstatus :: antivirus response = " + ArmaditoIPC.av_response);		
		
		return ;	
	};
	

	// Enum quarantine files.
	factory.requestAVquarantine = function(callback){
		
		console.log("[+] Debug :: requestAVqurantine ::");		
		var request = { "av_request":"quarantine", "id":123, "params": {"action":"enum"}};
		var buffer = new Buffer( JSON.stringify(request), 'ascii' );
		
		// Set ipc path.
		this.setClientPath();	

		ArmaditoIPC.connect2_av(ipc_path,callback);
		
		ArmaditoIPC.write2_av(buffer);
		
		//ArmaditoIPC.disconnect2_av();
		
		// send data
		//var response = ArmaditoIPC.sendAndReceive(clientPath, request);
		//console.log("[+] Debug :: requestAVstatus :: antivirus response = " + av_response);
		
		//console.log("[+] Debug :: requestAVstatus :: antivirus response = " + ArmaditoIPC.av_response);		
		
		return ;	
	};
	
	// Restore file from quarantine.
	factory.requestAVrestore = function(filename,callback){
		
		console.log("[+] Debug :: requestAVqurantine ::");		
		var request = { "av_request":"quarantine", "id":123, "params": {"action":"restore", "fname":filename}};
		var buffer = new Buffer( JSON.stringify(request), 'ascii' );
		
		// Set ipc path.
		this.setClientPath();	

		ArmaditoIPC.connect2_av(ipc_path,callback);
		
		ArmaditoIPC.write2_av(buffer);
		
		ArmaditoIPC.disconnect2_av();		
		
		
		return ;	
	};

	// Delete file from quarantine.
	factory.requestAVdelete = function(filename,callback){
		
		
		var request = { "av_request":"quarantine", "id":123, "params": {"action":"delete", "fname":filename}};
		var buffer = new Buffer( JSON.stringify(request), 'ascii' );
		
		// Set ipc path.
		this.setClientPath();	

		ArmaditoIPC.connect2_av(ipc_path,callback);
		
		ArmaditoIPC.write2_av(buffer);
		
		ArmaditoIPC.disconnect2_av();		
				
		
		return ;	
	};

	// Update modules database files and reload the av service.
	factory.updateDB = function(callback){

		var request = { "av_request":"updatedb", "id":123, "params": {}};
		var buffer = new Buffer( JSON.stringify(request), 'ascii' );

		// Set ipc path.
		this.setClientPath();

		ArmaditoIPC.connect2_av(ipc_path,callback);
		
		ArmaditoIPC.write2_av(buffer);
		
		ArmaditoIPC.disconnect2_av();

		return;

	};


	factory.cleanALL = function(){
		
		// Set ipc path.
		this.setScanServerPath();		

		console.log(' scan_server_ipc_path: ', scan_server_ipc_path);
		ArmaditoIPC.cleanSocket(scan_server_ipc_path);

		return;
	};
	

	return factory;
}]);
