'use strict';

/**
 * @ngdoc function
 * @name armaditoApp.controller:DetailRapportModalController
 * @description
 * # DetailRapportModalController
 * Controller of the armaditoApp
 */
angular.module('armaditoApp')
  .controller('DetailRapportModalController', ['$scope', '$uibModalInstance', 'items', function ($scope, $uibModalInstance, items) {
    
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
