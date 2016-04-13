'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:JournalController
 * @description
 * # JournalController
 * Controller of the tatouApp
 */
angular.module('tatouApp')
  .controller('JournalController', ['$scope', '$uibModal', 'ArmaditoSVC','ArmaditoIPC','toastr', function ($scope, $uibModal, ArmaditoSVC, ArmaditoIPC, toastr) {
	  
    var Noms = ['Malware1', 'Malware2', 'Malware3', 'Malware4'];
    var Emplacements = ['/home/userName/...', '/home/Desktop/folder/...', '/home/userName/...', '/home/userName/...'];
    var Dates = ['05/11/2015', '18/11/2015', '13/12/2015', '15/12/2015'];
    var id = 1;
	
	$scope.quarantine = {
		count : 0,
		last_update : "1970"
		//files: []
	};
	
	$scope.quarantineObjects = [
		{
			name: 'malware',
			path: 'C:/Users/Bla/bla/bla',
			date: '04/12/2015',
			refresh : '<i class="fa fa-refresh"></i>',
			delete  : '<i class="text-danger fa fa-times"></i>'
		},
		{
			name: 'malware',
			path: 'C:/Users/Bla/bla/bla',
			date: '04/12/2015',
			refresh : '<i class="fa fa-refresh"></i>',
			delete  : '<i class="text-danger fa fa-times"></i>'
		},
		{
			name: 'malware',
			path: 'C:/Users/Bla/bla/bla',
			date: '04/12/2015',
			refresh : '<i class="fa fa-refresh"></i>',
			delete  : '<i class="text-danger fa fa-times"></i>'
		},
		{
			name: 'malware',
			path: 'C:/Users/Bla/bla/bla',
			date: '04/12/2015',
			refresh : '<i class="fa fa-refresh"></i>',
			delete  : '<i class="text-danger fa fa-times"></i>'
		},
		{
			name: 'malware',
			path: 'C:/Users/Bla/bla/bla',
			date: '04/12/2015',
			refresh : '<i class="fa fa-refresh"></i>',
			delete  : '<i class="text-danger fa fa-times"></i>'
		},
		{
			name: 'malware',
			path: 'C:/Users/Bla/bla/bla',
			date: '04/12/2015',
			refresh : '<i class="fa fa-refresh"></i>',
			delete  : '<i class="text-danger fa fa-times"></i>'
		},
		{
			name: 'malware',
			path: 'C:/Users/Bla/bla/bla',
			date: '04/12/2015',
			refresh : '<i class="fa fa-refresh"></i>',
			delete  : '<i class="text-danger fa fa-times"></i>'
		},
		{
			name: 'malware',
			path: 'C:/Users/Bla/bla/bla',
			date: '04/12/2015',
			refresh : '<i class="fa fa-refresh"></i>',
			delete  : '<i class="text-danger fa fa-times"></i>'
		}
	];

	$scope.refreshQuatantineObject = function (index){
		$scope.quarantineObjects.splice(index, 1);
		console.log("refreshed !");
	};

	$scope.deleteQuatantineObject = function (index){
		$scope.quarantineObjects.splice(index, 1);
		console.log("deleted !");
	};

	$scope.clearQuarantine = function (){
      	var size = 'sm';
      	var item = {
      		title : 'Confirmation de suppression',
      		sentence : "Etes vous sûr de vouloir suprimer "+ $scope.quarantineObjects.length + " objets de la quarantaines ?"
      	};
      	var modalInstance = $uibModal.open({
	        animation: $scope.animationsEnabled,
	        templateUrl: 'views/Confirmation.html',
	        controller: 'ConfirmationController',
	        size: size,
	        resolve: {
	          data: function () {
	            return  item;
	          }
	        }
      	});

      	modalInstance.result.then(function (scanOptions) {
	        $scope.quarantineObjects = [];
	        toastr.success('Suppression d\'objets en quarantaine effectuée avec succès.');
	    }, function () {
        	console.log('Modal dismissed at: ' + new Date());
      	});
	};

	//$scope.quarantine.count = 0;
	$scope.quarantine.files = [];
	
	$scope.status = {}; 
	$scope.status.openDay = true;
	$scope.status.openWeek = false;
	$scope.status.openMonth = false;
	$scope.status.openYear = false;
	$scope.threatDataFromAv = function(data){
		
		var json_obj;
		
		console.log('[+] Debug :: threatDataFromAv :: '+data);
		
		try{
			
			json_obj = JSON.parse(data);
			//console.log('[+] Debug :: threatDataFromAv :: jobj :: ',json_obj);
			//onsole.log('[+] Debug :: threatDataFromAv :: jobj :: ',json_obj.info.files[0].date);
			
			$scope.quarantine.files = json_obj.info.files;
			//console.log('[+] Debug :: threatDataFromAv :: date :: ',json_obj.info.files[0].date);
			
			/*for (var i = 0; i< $scope.quarantine.files.length ; i++){
				
				console.log('[+] Debug :: threatDataFromAv :: date :: ',$scope.quarantine.files[i].date);
				console.log('[+] Debug :: threatDataFromAv :: path :: ',$scope.quarantine.files[i].path);
				console.log('[+] Debug :: threatDataFromAv :: desc :: ',$scope.quarantine.files[i].desc);				
			}*/
			
		}
		catch(e){
			console.error("Parsing error:", e); 
			return null;
		}
		
		$scope.$apply();
			
		return;
	}
	
	$scope.query_quarantine = function(){
			
			console.log('[+] Debug :: Refreshing antivirus quarantine...');
			// send quarantine request to av.			
			ArmaditoSVC.requestAVquarantine($scope.threatDataFromAv);			
			console.log('[+] Debug :: Refreshing antivirus status ::' + ArmaditoIPC.client_socket);
			
			return;
	}
	
	$scope.restore_qurantine_file = function(filename){
		
		console.log('[+] Debug :: Restoring file : '+filename);
		ArmaditoSVC.requestAVrestore(filename,$scope.threatDataFromAv);
		
		return;
		
	}
	
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

       	$scope.openDetailRapport = function (size) {

		    var modalInstance = $uibModal.open({
		      animation: $scope.animationsEnabled,
		      templateUrl: 'views/DetailRapportModal.html',
		      controller: 'DetailRapportModalController',
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


  }]);
