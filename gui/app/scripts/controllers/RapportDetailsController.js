'use strict';

/**
 * @ngdoc function
 * @name armaditoApp.controller:RapportDetailsController
 * @description
 * # RapportDetailsController
 * Controller of the armaditoApp
 */
angular.module('armaditoApp')
  .controller('RapportDetailsController', ['$scope', '$uibModalInstance', 'items' ,
  		function ($scope, $uibModalInstance, items) {
	   	  $scope.items = items;
		  $scope.selected = {
		    item: $scope.items[0]
		  };

		  $scope.ok = function () {
		    $uibModalInstance.close($scope.selected.item);
		  };

		  $scope.cancel = function () {
		    $uibModalInstance.dismiss('cancel');
		  };
  }]);
