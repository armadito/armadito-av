'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:ConfirmationController
 * @description
 * # ConfirmationController
 * Controller of the tatouApp
 */
angular.module('tatouApp')
  .controller('ConfirmationController',['$scope', '$uibModalInstance', 'data', 'ArmaditoSVC', function ($scope, $uibModalInstance, data, ArmaditoSVC) {

         $scope.threatDataFromAv = function(data){

	      var json_object;
	      var jobj_modules;
	      var date = new Date().toISOString();

	      try {

		json_object = JSON.parse(data);

		console.log("[+] Debug :: Data received from av :: ", json_object);

		// Handle the cancel scan response of the av.
		if(json_object.av_response == "scan_cancel" && json_object.status == 0 ){
		  console.log("[+] Debug :: Scan successfully cancelled !\n");      
		  global.scan_in_progress = 0;    
		  return;
		}

	      }
	      catch(e){
		console.error("Parsing error:", e); 
		return null;
	      }

	}

  	$scope.sentence = data.sentence;
  	$scope.title = data.title;
  
	if(global.scan_in_progress){    
	    $scope.sentence = "main_view.Scan_in_progress";
	}

  	$scope.ok = function () {
		$uibModalInstance.close($scope.quarantineLength);

		// Send cancel_scan to AV if needed
		if(global.scan_in_progress){ 
		    ArmaditoSVC.cancelScan($scope.threatDataFromAv);
                }
	};

	$scope.cancel = function () {
	    $uibModalInstance.dismiss('cancel');
	};

  }]);
