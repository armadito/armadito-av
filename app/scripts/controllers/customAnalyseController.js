'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:customAnalyseController
 * @description
 * # customAnalyseController
 * Controller of the tatouApp
 */
angular.module('tatouApp')
  .controller('customAnalyseController', ['$scope', '$uibModalInstance', 'items', function ($scope, $uibModalInstance, items) {
    
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
