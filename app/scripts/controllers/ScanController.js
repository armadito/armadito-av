'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:ScanController
 * @description
 * # ScanController
 * Controller of the tatouApp
 */
 
 var os = require('os');
 
angular.module('tatouApp')
  .controller('ScanController', ['$scope', '$interval', 'MockAvService', 'AntivirusService', 'EventService', 'toastr',
        function ($scope,  $interval, MockAvService, AntivirusService, EventService, toastr) {

            $scope.rowCollection = [];

            //copy the references (you could clone ie angular.copy but then have to go through a dirty checking for the matches)
            $scope.displayedCollection = [].concat($scope.rowCollection);

            /* Antivirus scan*/

             //start Dummy AV
		//MockAvService.startMockAv();

              $scope.pdata = {
                scan_progress : 0
              };

             EventService.onMessageReceived('scan_event', function(evt, data){
                    //console.log('received data ' + data);

		     $scope.pdata.scan_progress = data.params.progress;
			 $scope.pdata.path = data.params.path;

                  $scope.$apply(function () {                  
                    var threats;
                    if(data){
                      $scope.tableScan = [
                        {
                          title : "Object courant",
                          data : $scope.pdata.path
                        },
                        {
                          title : "Fichiers traités",
                          data : $scope.pdata.path
                        },
                        {
                          title : "Temps écoulé",
                          data : "12 : 05 min"
                        }
                      ];

                      $scope.pdata =  data;
					 // console.log('[+] Debug :: path = ' + $scope.pdata.params.path);
					  //$scope.pdata.params.path = data.params.path;
					  $scope.pdata.scan_progress = data.params.progress;

                        threats = [
                          { 
                              id: data.id,
                              Etat: data.params.scan_status,
                              Detail: data.params.mod_report,
                              Chemin : data.params.path
                          }
                      ];
                    }
                    var selectedTreath = threats[Math.floor(Math.random() * threats.length)];
                    if(selectedTreath.id == 77){
                      $scope.displayedCollection.push(selectedTreath);  
                    }
                    $scope.showScanDetails = true;
                    //console.log($scope.displayedCollection);
                  });
                }, $scope)
              
              /* Timer */
              $scope.timerRunning = false;
 
              $scope.startTimer = function (){
                  $scope.$broadcast('timer-start');
                  $scope.timerRunning = true;
              };
              
              $scope.chooseFile = function (name) {
                var chooser = document.querySelector(name);
                chooser.addEventListener("change", function(evt) {
                  console.log("value", this.value);
                }, false);

                chooser.click();  
              }
                
              $scope.chooseFile = function (name) {
                var chooser = document.querySelector(name);
                chooser.addEventListener("change", function(evt) {
                  var path = this.value;
                  $scope.$apply(function(){
                    $scope.pathToScan = path;
                  })
                }, false);
              };

              $scope.chooseFile('#pathToScan');
				
              $scope.startMe = function(){

                if(($scope.pathToScan === "") || ($scope.pathToScan === undefined)){
                  toastr.warning('Veuillez choisir un dossier à analyser svp', '');
                }else{
                  $scope.startTimer();
                  // FD
                  //AntivirusService.startScan({
                  //scan_action: 'new_scan',
                  //scan_id: 77,
                  //scan_path: '/home/kimios'
                  //});
                  // FIXME: must get the path from platform (unix socket vs. named pipe)
				  
      				  if(os.platform() == "win32"){
      					  $scope.ui_ipc_path = '\\\\.\\pipe\\uhuru-IHM';
      				  }else{
      					  $scope.ui_ipc_path = '/tmp/.uhuru-ihm';
      				  }
				  
                  AntivirusService.startScan({
                    av_request: 'scan',
                    id: 77,
                    params : {
                      ui_ipc_path: $scope.ui_ipc_path,
                      path_to_scan: $scope.pathToScan
                    }
                  });
                }

              };

              $scope.stopMe = function(){
                AntivirusService.stopScan({
                  scan_action: 'cancel_scan',
                  scan_id: 77
                });
              };
              
            /* Antivirus scan*/

  }]);
