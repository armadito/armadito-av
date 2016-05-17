'use strict';

/**
 * @ngdoc function
 * @name armaditoApp.controller:InformationController
 * @description
 * # InformationController
 * Controller of the armaditoApp
 */
 
 var os = require('os');
 var sprintf = require("sprintf-js").sprintf;

angular.module('armaditoApp')
  .controller('InformationController', ['$scope','ArmaditoSVC','ArmaditoIPC', function ($scope,ArmaditoSVC,ArmaditoIPC) {

		
		$scope.rowCollection = [];
		$scope.test =  false;

		$scope.update = function (){
			console.log("update !")
		};

		//copy the references (you could clone ie angular.copy but then have to go through a dirty checking for the matches)
		$scope.displayedCollection = [].concat($scope.rowCollection);
		
		// Initialize state.
		$scope.state = {
			status : 0,
			service : false,
			realtime : false,
			upToDate : false,
			update : "critical",
			last_update : "Not determined",
			last_update_timestamp : 0,
			version : "Not determined"
		};
			

		$scope.timeConverter = function(timestamp){
		  var a = new Date(timestamp * 1000);
		  var months = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'];
		  var year = a.getFullYear();
		  var month = months[a.getMonth()];
		  var date = a.getDate();
		  var hour = a.getHours();
		  var min = a.getMinutes();
		  var sec = a.getSeconds();

		  return sprintf("%02d %s %d %02d:%02d:%02d", date, month, year, hour, min, sec);
		};
	
		/*$scope.state.modules = 
		[{
			name : "clamav",
			version: "0.0.0",
			update: {"status": "not loaded", "last-update": "1970-01-01T06:13:00Z" }
		 },
		 {
			name : "module5.2",
			version: "0.0.0",
			update: {"status": "not loaded", "last-update": "1970-01-01T06:13:00Z" }
		 },
		 {
			name : "modulePDF",
			version: "1.0.1",
			update: {"status": "not loaded", "last-update": "1970-01-01T06:13:00Z" }
		 }
		];*/


		$scope.state.modules = [];
		

		$scope.threatDataFromAv = function(data){
			
			var json_object;
			var jobj_modules;
			var date = new Date().toISOString();
			
			//console.log('------- current date = ',date);

			
			try {
				json_object = JSON.parse(data);

				console.log("[+] Debug :: data from av ::",json_object);
				
				if(json_object.info.antivirus.service == 'on'){
					$scope.state.service = true;					
				}else{
					$scope.state.service = false;					
				}
								
				if(json_object.info.antivirus['real-time-protection']  == 'on'){
					$scope.state.realtime = true;
				}else{
					$scope.state.realtime = false;
				}

				if(json_object.info.update.status  == 'up-to-date'){
					$scope.state.upToDate = true;
				}else{
					$scope.state.upToDate = false;
				}
				
								
				$scope.state.update = json_object.info.update;

				$scope.state.version = json_object.info.antivirus.version;

				// unused : json_object.info.update['last-update'];
				
				$scope.state.last_update_timestamp = json_object.info.update.timestamp;
				$scope.state.last_update = $scope.timeConverter(json_object.info.update.timestamp);
				

				//console.log('[+] Debug :: threatDataFromAv :: av last-update :: ',json_object.info.update['last-update']);
				
				// modules infos.
				//console.log('------- module tab obj = ',json_object.info.modules);
				//console.log('[+] Debug :: threatDataFromAv :: Number of modules :: ',json_object.info.modules.length);

				$scope.state.modules = json_object.info.modules;				
				for (var i = 0; i< $scope.state.modules.length ; i++){

					$scope.state.modules[i].update['date'] = $scope.timeConverter($scope.state.modules[i].update['timestamp']);
					
					//console.log('[+] Debug :: threatDataFromAv :: module name :: ',$scope.state.modules[i].name);
					console.log('[+] Debug :: threatDataFromAv :: module timestamp :: ',$scope.state.modules[i].update['timestamp']);	
					//console.log('[+] Debug :: threatDataFromAv :: module date :: ',$scope.state.modules[i].update['date']);				
				}

			}
			catch(e){
				console.error("Parsing error:", e); 
				return null;
			}
			
			//console.log('[+] Debug :: In callback refresh object with data :: ' + data);
						
			//$scope.state.service = 3;			
			$scope.$apply();
			return;
		}


		$scope.update_db = function(){

			console.log("[+] Debug :: update modules database\n");
			ArmaditoSVC.updateDB();

			// TODO :: refresh state.

		}
			
		// refresh antivirus status.
		$scope.refresh_status = function(){
			
			console.log('[+] Debug :: Refreshing antivirus status...');
			
			// send state request to av.			
			//$scope.state.service = "2";
			//$scope.state.service = ArmaditoIPC.av_response;
			//$scope.state.last_update = ArmaditoIPC.av_response;
			ArmaditoSVC.requestAVstatus($scope.threatDataFromAv);
			
			console.log('[+] Debug :: Refreshing antivirus status ::' + ArmaditoIPC.client_socket);
		}
		
		
		$scope.refresh_status();

		//Real time activated or not 
		/*$scope.$watch('state.realtime', function(newValue, oldValue) {
			if( newValue){
			    console.log("test : ", newValue);
			}
    	});*/

    	$scope.testfunc = function () {
    		console.log($scope.state.realtime);
    	};

  }]);

  
 
