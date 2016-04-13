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

  	$scope.ok = function () {
		$uibModalInstance.close($scope.quarantineLength);
	};

	$scope.cancel = function () {
	    $uibModalInstance.dismiss('cancel');
	};

  }]);
