'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:ScanController
 * @description
 * # ScanController
 * Controller of the tatouApp
 */
angular.module('tatouApp')
  .controller('ScanController', ['$scope', '$interval', 'MockAvService', 'AntivirusService', 'EventService', 
        function ($scope,  $interval, MockAvService, AntivirusService, EventService) {

            $scope.rowCollection = [];

            //copy the references (you could clone ie angular.copy but then have to go through a dirty checking for the matches)
            $scope.displayedCollection = [].concat($scope.rowCollection);

            /* Antivirus scan*/

             //start Dummy AV
              MockAvService.startMockAv();


              $scope.pdata = {
                scan_progress : 0
              };

             EventService.onMessageReceived('scan_event', function(evt, data){
                  $scope.$apply(function () {                  
                    var threats;
                    if(data){
                      $scope.tableScan = [
                        {
                          title : "Object courant",
                          data : $scope.pdata.scan_file_path
                        },
                        {
                          title : "Fichiers traités",
                          data : $scope.pdata.scan_progress + 37
                        },
                        {
                          title : "Temps écoulé",
                          data : "12 : 05 min"
                        }
                      ];

                      $scope.pdata =  data;
                        threats = [
                          { 
                              id: data.scan_id,
                              Etat: data.new_items[0].scan_status,
                              Detail: data.new_items[0].mod_report,
                              Chemin : data.scan_file_path
                          },
                          {
                              id: 1,
                              Etat: "test",
                              Detail: "spybot",
                              Chemin : "path/myPath/mySpecialpath..."
                          },
                          {
                              id: 3,
                              Etat: "test",
                              Detail: "trojan",
                              Chemin : "path/myPath/mySpecialpath..."
                          },
                          {
                              id: 4,
                              Etat: "test",
                              Detail: "vers",
                              Chemin : "path/myPath/mySpecialpath..."
                          }
                      ];
                    }
                    var selectedTreath = threats[Math.floor(Math.random() * threats.length)];
                    if(selectedTreath.id == 77){
                      $scope.displayedCollection.push(selectedTreath);  
                    }
                    $scope.showScanDetails = true;
                    console.log('received data ' + data.scan_progress);
                    console.log($scope.displayedCollection);
                  });
                }, $scope)
              
              /* Timer */
              $scope.timerRunning = false;
 
              $scope.startTimer = function (){
                  $scope.$broadcast('timer-start');
                  $scope.timerRunning = true;
              };

              $scope.startMe = function(){
                $scope.startTimer();
                AntivirusService.startScan({
                    scan_action: 'new_scan',
                    scan_id: 77,
                    scan_path: '/home/kimios'
                  });
              };

              $scope.stopMe = function(){
                AntivirusService.stopScan({
                  scan_action: 'cancel_scan',
                  scan_id: 77
                });
              };
              
            /* Antivirus scan*/

  }]);
