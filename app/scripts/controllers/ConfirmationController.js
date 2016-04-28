'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:ConfirmationController
 * @description
 * # ConfirmationController
 * Controller of the tatouApp
 */
angular.module('tatouApp')
  .controller('ConfirmationController',['$scope', '$uibModalInstance', 'data', function ($scope, $uibModalInstance, data) {

  	$scope.sentence = data.sentence;
  	$scope.title = data.title;
  
	if(global.scan_in_progress){    
	    $scope.sentence = "main_view.Scan_in_progress";
	}

  	$scope.ok = function () {
		$uibModalInstance.close($scope.quarantineLength);
		// TODO: send cancel_scan to AV
	};

	$scope.cancel = function () {
	    $uibModalInstance.dismiss('cancel');
	};

  }]);
