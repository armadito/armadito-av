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
