'use strict';

/**
 * @ngdoc function
 * @name armaditoApp.controller:customAnalyseController
 * @description
 * # customAnalyseController
 * Controller of the armaditoApp
 */
angular.module('armaditoApp')
  .controller('customAnalyseController', ['$scope', '$uibModalInstance', 'items', function ($scope, $uibModalInstance, items) {
    
    $scope.items = items;

    $scope.optionScan = {
        pathToScan : '',
        heuristicMode : false,
        scanArchive : false,
        excludeFolder : ''
    };

   $scope.ok = function () {
    	$uibModalInstance.close($scope.optionScan);
    };

    $scope.cancel = function () {
    	$uibModalInstance.dismiss('cancel');
    };

    $scope.chooseFile = function () {
      var name = '#pathToScan';
      var chooser = document.querySelector(name);
      chooser.addEventListener("change", function(evt) {
        var path = this.value;
        $scope.$apply(function(){
           $scope.optionScan.pathToScan = path;
           console.log("pathToScan", $scope.optionScan.pathToScan);
        })
      }, false);
    };

    
     $scope.excludedFolders = [];

    $scope.chooseFileToExclude = function () {
      var name = '#fileToExclude';
      var chooser = document.querySelector(name);
      chooser.addEventListener("change", function(evt) {
        console.log(evt);
        var path = this.value;
        $scope.$apply(function(){
          $scope.pathToExclude = path;
          var optionScan = {
            pathToScan : $scope.pathToExclude
          };
          $scope.excludedFolders.push(optionScan);
          console.log($scope.excludedFolders);
        }) 
      }, false);
    };

  }]);
