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
 * @name armaditoApp.controller:ScanController
 * @description
 * # ScanController
 * Controller of the Armadito-av
 */
 
 var os = require('os');
 var homedir = require('homedir');

angular.module('armaditoApp')
  .controller('ScanController', ['$rootScope','$scope','ArmaditoSVC','ArmaditoIPC','$uibModal', 'ScanDataFactory', function ($rootScope,$scope,ArmaditoSVC,ArmaditoIPC, $uibModal, ScanDataFactory) {

    $scope.updateScope = function(){
   
	    // Get from factory
	    $scope.type = ScanDataFactory.data.type;
	    $scope.scan_progress = ScanDataFactory.data.progress;
	    $scope.scan_files = ScanDataFactory.data.files;
	    $scope.scanned_count = ScanDataFactory.data.scanned_count;
	    $scope.suspicious_count = ScanDataFactory.data.suspicious_count;
	    $scope.malware_count = ScanDataFactory.data.malware_count;
	    $scope.displayed_file = ScanDataFactory.data.displayed_file;
	    $scope.path_to_scan = ScanDataFactory.data.path_to_scan;
            $scope.canceled = ScanDataFactory.data.canceled;
    }

    // This function refresh structure values from data receive from AV. 
    // callback function
    $scope.threatDataFromAv = function(json_object){

      try {
        //console.log("[+] Debug :: Data received from av :: "); //, json_object);

        // Handle the first response of the av.
        if(json_object.av_response == "scan" && json_object.status == 0 ){
          console.log("[+] Debug :: Scan order send successfully to av!\n");
	  global.scan_in_progress = 1;    
	  $scope.updateScope();
	  $scope.$apply();
          return;
        }

        // Handle progress  information from av
        if(json_object.ihm_request == "scan"){

	  // Update model
	  if(json_object.params.path){
	      ScanDataFactory.set_displayed_file(json_object.params.path);
	      $scope.displayed_file = ScanDataFactory.data.displayed_file;
	  }

          if(json_object.params.scan_status === 'malware' || json_object.params.scan_status === 'suspicious'){
	      ScanDataFactory.add_scanned_file(json_object.params.path,
					      json_object.params.scan_status,
					      json_object.params.scan_action,
					      json_object.params.mod_name,
					      json_object.params.mod_report);
          }

          ScanDataFactory.update_counters(json_object.params.scanned_count,
			              json_object.params.suspicious_count,
				      json_object.params.malware_count,
			              json_object.params.progress);

	  // updateScope
	  $scope.updateScope();

          // Terminate scan.
          if($scope.scan_progress == 100){
               console.log("[+] Debug :: Scan finished!");
	       global.scan_in_progress = 0;

	       // Scan is finished, we remove listeners
	       $rootScope.myEmitter.removeAllListeners('scan_info');
          }

          // Apply modifications
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

    $scope.StartScan = function(){
    
      // Reset factory
      ScanDataFactory.reset();

      // Set Scan configuration
      $scope.configureScan();

      // Save current scan conf in factory
      ScanDataFactory.set_scan_conf($scope.path_to_scan, $scope.type);

      console.log("[+] Debug :: Start scan ::\n");

      // updateScope
      $scope.updateScope();

      // we remove last listener
      $rootScope.myEmitter.removeAllListeners('scan_info');

      // register emitter
      $rootScope.myEmitter.addListener('scan_info', $scope.threatDataFromAv);

      // Launch scan
      ArmaditoSVC.launchScan($scope.path_to_scan, $scope.threatDataFromAv);
    };

    $scope.configureScan = function(){

      if($scope.type == "analyse_view.Full_scan"){
        console.log("[+] Debug :: Full scan ::\n");

        if(os.platform() == "win32"){
           $scope.path_to_scan = "C:\\";
	}
	else {
	   $scope.path_to_scan = "/";
	}

      }else if($scope.type == "analyse_view.Quick_scan"){
        console.log("[+] Debug :: Quick scan ::\n");

        if(os.platform() == "win32"){
	   var userpath = process.env[(process.platform == 'win32') ? 'USERPROFILE' : 'HOME'];
	   userpath = userpath.replace(/\\/g, "\\\\");
	   $scope.path_to_scan =  userpath;
	}
	else {
	   $scope.path_to_scan = homedir(); // "/home";
	}

      }else if($scope.type == "analyse_view.Custom_scan"){
        console.log("[+] Debug :: Custom scan ::\n");
      }

    };

    $scope.CancelScan = function(){
    
      //console.log("[+] Debug :: cancel scan ");
      if($scope.canceled == 0){
           ArmaditoSVC.cancelScan($scope.threatDataFromAv);
	   ScanDataFactory.setCanceled();
      }

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
        $scope.path_to_scan = $scope.scanOptions.pathToScan;
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

    // updateScope
    $scope.updateScope();

  }]);

  
 
