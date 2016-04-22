'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:InformationController
 * @description
 * # InformationController
 * Controller of the tatouApp
 */
 
 var os = require('os');
 
angular.module('tatouApp')
  .controller('InformationController', ['$scope','ArmaditoSVC','ArmaditoIPC', function ($scope,ArmaditoSVC,ArmaditoIPC) {

		
		$scope.rowCollection = [];
		$scope.test =  false;

		$scope.update = function (){
			console.log("update !")
		};

		//copy the references (you could clone ie angular.copy but then have to go through a dirty checking for the matches)
		$scope.displayedCollection = [].concat($scope.rowCollection);
		
		// Initialize state.
		if(!$scope.state){
			$scope.state = {
				status : 0,
				service : false,
				realtime : true,
				update : "critical",
				last_update : "Not determined",
				version : "Not determined"
			};	
		}
		
				
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
			
			console.log('------- current date = ',date);
			
			try {
				json_object = JSON.parse(data);
				
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
								
				$scope.state.update = json_object.info.update;
				
				
				$scope.state.version = json_object.info.antivirus.version;

				$scope.state.last_update = json_object.info.update['last-update'];
				console.log('[+] Debug :: threatDataFromAv :: av last-update :: ',json_object.info.update['last-update']);
				
				// modules infos.
				console.log('------- module tab obj = ',json_object.info.modules);
				console.log('[+] Debug :: threatDataFromAv :: Number of modules :: ',json_object.info.modules.length);
				//console.log('[+] Debug :: threatDataFromAv :: module name :: ',json_object.info.modules[i].name);
				$scope.state.modules = json_object.info.modules;				
				/*for (var i = 0; i< $scope.state.modules.length ; i++){
					
					console.log('[+] Debug :: threatDataFromAv :: module name :: ',$scope.state.modules[i].name);
					console.log('[+] Debug :: threatDataFromAv :: module date :: ',$scope.state.modules[i].update['last-update']);
					
					//var ret = date - $scope.state.modules[i].update['last-update'];
					//console.log('[+] Debug :: threatDataFromAv :: rsult:: ',ret);
					
					//$scope.modules[];					
				}*/
				
				
				//$scope.state.version = json_object.info.antivirus.version;
				//$scope.state.service = json_object.info.antivirus.version;
			}
			catch(e){
				console.error("Parsing error:", e); 
				return null;
			}
			
			//console.log('[+] Debug :: In callback refresh object with data :: ' + data);
						
			//$scope.state.service = 3;
			console.log("[+]");
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

  
 