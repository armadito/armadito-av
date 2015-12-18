'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:JournalController
 * @description
 * # JournalController
 * Controller of the tatouApp
 */
angular.module('tatouApp')
  .controller('JournalController', ['$scope', '$uibModal', '$log', function ($scope, $uibModal, $log) {
    var Noms = ['Malware1', 'Malware2', 'Malware3', 'Malware4'];
    var Emplacements = ['/home/userName/...', '/home/Desktop/folder/...', '/home/userName/...', '/home/userName/...'];
    var Dates = ['05/11/2015', '18/11/2015', '13/12/2015', '15/12/2015'];
    var id = 1;

    function generateRandomItem(id) {

        var Nom = Noms[Math.floor(Math.random() * 3)];
        var Emplacement = Emplacements[Math.floor(Math.random() * 3)];
        var Date = Dates[Math.floor(Math.random() * 3)];

        return {
            id: id,
            Nom: Nom,
            Emplacement: Emplacement,
            Date : Date
        }
    }

    $scope.rowCollection = [];

    for (id; id < 4; id++) {
        $scope.rowCollection.push(generateRandomItem(id));
    }

    //copy the references (you could clone ie angular.copy but then have to go through a dirty checking for the matches)
    $scope.displayedCollection = [].concat($scope.rowCollection);

    //add to the real data holder
    $scope.addRandomItem = function addRandomItem() {
        $scope.rowCollection.push(generateRandomItem(id));
        id++;
    };

    //remove to the real data holder
    $scope.removeItem = function removeItem(row) {
        var index = $scope.rowCollection.indexOf(row);
        $scope.rowCollection.splice(index, 1);
    };

    /*Open modal for rapport details*/
    $scope.items = ['Date : 01/02/2015', 'Chemin fichier : blablabla', 'Type : Blablablabla'];

      $scope.animationsEnabled = true;

      $scope.open = function (size) {

        var modalInstance = $uibModal.open({
          animation: $scope.animationsEnabled,
          templateUrl: 'views/RapportDetails.html',
          controller: 'RapportDetailsController',
          size: size,
          resolve: {
            items: function () {
              return $scope.items;
            }
          }
        });

        modalInstance.result.then(function (selectedItem) {
          $scope.selected = selectedItem;
        }, function () {
          $log.info('Modal dismissed at: ' + new Date());
        });
      };

      $scope.toggleAnimation = function () {
        $scope.animationsEnabled = !$scope.animationsEnabled;
      };


  }]);
