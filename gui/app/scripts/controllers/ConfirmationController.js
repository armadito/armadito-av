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
 * @ngdoc function
 * @name armaditoApp.controller:ConfirmationController
 * @description
 * # ConfirmationController
 * Controller of the armaditoApp
 */
angular.module('armaditoApp')
  .controller('ConfirmationController',['$scope', '$uibModalInstance', 'data', 'ArmaditoSVC', function ($scope, $uibModalInstance, data, ArmaditoSVC) {

         $scope.threatDataFromAv = function(data){

	      var json_object;
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

		// Send cancel_scan to AV if needed
		if(global.scan_in_progress){ 
		    ArmaditoSVC.cancelScan($scope.threatDataFromAv);
                }
	
		$uibModalInstance.close($scope.quarantineLength);
	};

	$scope.cancel = function () {
	    $uibModalInstance.dismiss('cancel');
	};

  }]);
