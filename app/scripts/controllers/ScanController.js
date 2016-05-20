'use strict';

/**
 * @ngdoc function
 * @name armaditoApp.controller:ScanController
 * @description
 * # ScanController
 * Controller of the Armadito-av
 */
 
 var os = require('os');
 
angular.module('armaditoApp')
  .controller('ScanController', ['$rootScope','$scope','ArmaditoSVC','ArmaditoIPC','$uibModal', function ($rootScope,$scope,ArmaditoSVC,ArmaditoIPC, $uibModal) {

    $scope.HideInputFile = true;
    $scope.type = "analyse_view.Choose_analyse_type";
    $scope.scan_server;
    $scope.malware_count = 0;
    $scope.suspicious_count = 0;
    $scope.scanned_count = 0;

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
      last_update : "Il y a 3 jours",
      version : 0.1       
    };

    $scope.scan_data.files = [];
    $scope.state.modules = [];

    

    // This function refresh structure values from data receive from AV. 
    // callback function
    $scope.threatDataFromAv = function(data){

      var json_object;
      var jobj_modules;
      var date = new Date().toISOString();

      try {

        json_object = JSON.parse(data);

        console.log("[+] Debug :: Data received from av :: "); //, json_object);

        // Handle the first response of the av.
        if(json_object.av_response == "scan" && json_object.status == 0 ){
          console.log("[+] Debug :: Scan order send successfully to av!\n");      
	  global.scan_in_progress = 1;    
          return;
        }

        console.log("[+] Debug :: Data received from av :: ihm_request "); // + json_object.ihm_request);

        // Handle progress  information from av
        if(json_object.ihm_request == "scan"){

          $scope.scan_data.progress = json_object.params.progress ;
          //console.log("[+] Debug :: progress = ", $scope.scan_data.progress);

          if(json_object.params.scan_status != 'undecided'){
            $scope.scan_data.files.push(json_object.params);
          }

	  if(json_object.params.malware_count){
	     $scope.malware_count = json_object.params.malware_count;
	  }
	
          if(json_object.params.suspicious_count){
	     $scope.suspicious_count = json_object.params.suspicious_count;
          }	
	
          if(json_object.params.scanned_count){
	     $scope.scanned_count = json_object.params.scanned_count;
          }	

          //console.log("tableau", $scope.scan_data.files);

          // terminate scan.
          if($scope.scan_data.progress == 100){
               console.log("[+] Debug :: Scan finished!");
	       global.scan_in_progress = 0;

	       // Scan is finished, we remove listener
	       $rootScope.myEmitter.removeListener('scan_info', $scope.threatDataFromAv);
          }

          $scope.$apply();
        }
        
      }
      catch(e){
        console.error("Parsing error:", e); 
	json_object = null;
        return null;
      }

      json_object = null;
    }


    // we remove last listener
    $rootScope.myEmitter.removeListener('scan_info', $scope.threatDataFromAv);

    // register emitter 
    $rootScope.myEmitter.addListener('scan_info', $scope.threatDataFromAv);


    $scope.StartScan = function(){
    
     // console.log("[+] Debug :: type d'analyse ::", $scope.type);
 
     // reset progress bar
      $scope.scan_data.progress = 0;
      $scope.scan_data.files = [];

      if($scope.type == "analyse_view.Full_scan"){
        // only for test
        console.log("[+] Debug :: ANALYSE COMPLÈTE ::\n");
        $scope.scan_data.path_to_scan = "C:\\users\\david\\Desktop\\PDF_test";

      }else if($scope.type == "analyse_view.Quick_scan"){

        // only for test
        console.log("[+] Debug :: ANALYSE RAPIDE ::\n");
        $scope.scan_data.path_to_scan = "C:\\users\\david\\Desktop\\mof";

      }else if($scope.type == "analyse_view.Custom_scan"){

        // only for test
        console.log("[+] Debug :: ANALYSE PERSONALISÉE ::\n");
      }
      else{
        // display a notif.
        console.log("[+] Debug :: Please enter an analysis type ::\n");
        return;
      }

      // reset counters
      $scope.malware_count = 0;
      $scope.suspicious_count = 0;
      $scope.scanned_count = 0;

      // we remove last listener
      $rootScope.myEmitter.removeListener('scan_info', $scope.threatDataFromAv);

      // register emitter 
      $rootScope.myEmitter.addListener('scan_info', $scope.threatDataFromAv);

      ArmaditoSVC.launchScan($scope.scan_data.path_to_scan, $scope.threatDataFromAv);

    };

    $scope.CancelScan = function(){
    
      //console.log("[+] Debug :: cancel scan ::", $scope.type);
      ArmaditoSVC.cancelScan($scope.threatDataFromAv);
    };


    $scope.fullScan = function () {
       $scope.type = "analyse_view.Full_scan";
    };

    $scope.quickScan = function () {
      $scope.type = "analyse_view.Quick_scan";
    };

    $scope.customScan = function () {
      $scope.type = "analyse_view.Custom_scan";
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
        //console.log("Scan options : ", $scope.scanOptions);
        $scope.scan_data.path_to_scan = $scope.scanOptions.pathToScan;
        $scope.StartScan();
      }, function () {
        console.log('Modal dismissed at: ' + new Date());
      });

    };

    $scope.truncate = function (fullStr, strLen) {
        if (fullStr.length <= strLen){
          return fullStr;
        }
        var separator = '...';
        
        var sepLen = separator.length,
            charsToShow = strLen - sepLen,
            frontChars = Math.ceil(charsToShow/2),
            backChars = Math.floor(charsToShow/2);
        
        return fullStr.substr(0, frontChars) + 
               separator + 
               fullStr.substr(fullStr.length - backChars);
    };

  }]);

  
 
