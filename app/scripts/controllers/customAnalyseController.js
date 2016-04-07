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
      console.log("héhé");
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

    $scope.tags = [
      { name: 'Tag1' },
      { name: 'Tag2' },
      { name: 'Tag3' }
    ];

    var test = [
      { "name": "Tag1" },
      { "name": "Tag2" },
      { "name": "Tag3" },
      { "name": "Tag4" },
      { "name": "Tag5" },
      { "name": "Tag6" },
      { "name": "Tag7" },
      { "name": "Tag8" },
      { "name": "Tag9" },
      { "name": "Tag10" }
    ];

    $scope.loadTags = function(query) {
      return test;
    };
     $scope.excludedFolders = [];

    $scope.chooseFileToExclude = function () {
      var name = '#fileToExclude';
      var chooser = document.querySelector(name);
      chooser.addEventListener("change", function(evt) {
        console.log(evt);
        var path = this.value;
        $scope.$apply(function(){
          $scope.optionScan.pathToScan = path;
          var optionScan = {
            pathToScan : $scope.optionScan.pathToScan
          };
          $scope.excludedFolders.push(optionScan);
          console.log($scope.excludedFolders);
        })
      }, false);
    };

  }]);
