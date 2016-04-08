'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:ConfirmationController
 * @description
 * # ConfirmationController
 * Controller of the tatouApp
 */
angular.module('tatouApp')
  .controller('ConfirmationController',['$scope', '$uibModalInstance', 'items', function ($scope, $uibModalInstance, items) {

  	$scope.quarantineLength = items;
  	console.log("hoho", $scope.quarantineLength);

  	$scope.ok = function () {
		$uibModalInstance.close($scope.quarantineLength);
	};

	$scope.cancel = function () {
	    $uibModalInstance.dismiss('cancel');
	};

  }]);
