'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:ScanController
 * @description
 * # ScanController
 * Controller of the Armadito-av
 */
 
 var os = require('os');
 
angular.module('tatouApp')
  .controller('ScanController', ['$scope','ArmaditoSVC','ArmaditoIPC','$uibModal', function ($scope,ArmaditoSVC,ArmaditoIPC, $uibModal) {

    $scope.HideInputFile = true;
    $scope.type = "Choisissez un type d'analyse...";
    $scope.rowCollection = [];
    $scope.scan_server;

    //copy the references (you could clone ie angular.copy but then have to go through a dirty checking for the matches)
    $scope.displayedCollection = [].concat($scope.rowCollection);


    // Scan data :: Path to scan and progress
    $scope.scan_data = {
      path_to_scan : "",
      progress : 0
    };
    
    // Initialize state.
    $scope.state = {
      status : 0,
      service : false,
      realtime : false,
      update : "critical",
      last_update : "Il y a 3 jour",
      version : 0.1       
    };

    $scope.scan_data.files = [];
    
    $scope.state.modules = [];
    
    /*$scope.state.modules = 
    [{
      name : "clamav",
      version: "0.0.0",
      update: {"status": "not loaded", "last-update": "1970-01-01T06:13:00Z" }
     },
     {
      name : "module5.2",
      version: "0.0.0",
      update: {"status": "not loaded", "last-update": "1970-01-01T06:13:00Z" }
     },
     {
      name : "modulePDF",
      version: "1.0.1",
      update: {"status": "not loaded", "last-update": "1970-01-01T06:13:00Z" }
     }
    ];
    */


    // This function refresh structure values from data receive from AV. 
    $scope.threatDataFromAv = function(data){

      var json_object;
      var jobj_modules;
      var date = new Date().toISOString();

      try {

        json_object = JSON.parse(data);

        console.log("[+] Debug :: Data received from av :: ", json_object);


        // Handle the first response of the av.
        if(json_object.av_response == "scan" && json_object.status == 0 ){
          console.log("[+] Debug :: Scan order send successfully to av!\n");
          return;
        }

        console.log("[+] Debug :: Data received from av :: ihm_request =  " + json_object.ihm_request);

        // Handle progress information from av
        if(json_object.ihm_request == "scan"){

          $scope.scan_data.progress = json_object.params.progress ;
          console.log("[+] Debug :: progress = ", $scope.scan_data.progress);

          // terminate scan.
          if($scope.scan_data.progress == 100){
               $scope.scan_server.close();
          }

          $scope.$apply();
        }
        
        
      }
      catch(e){
        console.error("Parsing error:", e); 
        return null;
      }

    }


    $scope.StartScan = function(){

      console.log("[+] Debug :: type d'analyse ::", $scope.type);
      // reset progress bar
      $scope.scan_data.progress = 0;

      if($scope.type == "COMPLÈTE"){



        // only for test
        console.log("[+] Debug :: ANALYSE COMPLÈTE ::\n");
        $scope.scan_data.path_to_scan = "C:\\Users\\thibaut\\Desktop\\new";

      }else if($scope.type == "RAPIDE"){

        // only for test
        console.log("[+] Debug :: ANALYSE RAPIDE ::\n");
        $scope.scan_data.path_to_scan = "C:\\Users\\thibaut\\Desktop\\to_scan";

      }else{
        // display a notif.
        console.log("[+] Debug :: Please enter an analysis type ::\n");
        return;
      }

      ArmaditoSVC.launchScan($scope.scan_data.path_to_scan, $scope.threatDataFromAv);

      if($scope.scan_server){
        $scope.scan_server.close();
      }

      $scope.scan_server = ArmaditoSVC.receiveScanInfo($scope.threatDataFromAv);

    };


    $scope.analyseComplete = function () {
       $scope.type = "COMPLÈTE";
    };

    $scope.analyseRapide = function () {
      $scope.type = "RAPIDE";
    };

    $scope.analysePersonnalisee = function () {
      $scope.type = "PERSONALISÉE";
      var size = 'sm';
      var modalInstance = $uibModal.open({
        animation: $scope.animationsEnabled,
        templateUrl: 'views/customAnalyse.html',
        controller: 'customAnalyseController',
        size: size,
        resolve: {
          items: function () {
          return $scope.items;
        }
      }
      });

      modalInstance.result.then(function (scanOptions) {
        $scope.scanOptions = scanOptions;
        console.log("Scan options : ", $scope.scanOptions);
      }, function () {
        $log.info('Modal dismissed at: ' + new Date());
      });

    };



  }]);

  
 