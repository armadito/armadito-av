'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:DetailRapportModalController
 * @description
 * # DetailRapportModalController
 * Controller of the tatouApp
 */
angular.module('tatouApp')
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
