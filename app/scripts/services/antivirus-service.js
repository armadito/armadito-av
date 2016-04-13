'use strict';
/**
 * @ngdoc service
 * @name tatooDesktopApp.MockAv
 * @description
 * # MockAv
 * Provider in the tatooDesktopApp.
 */
angular.module('armadito.services', [])
  .factory('EventService', function($rootScope){
    return {
      sendMsg : function(msg, data){
        $rootScope.$emit(msg, data);
      },
      onMessageReceived: function(msg, func, scope){
        var unlink = $rootScope.$on(msg, func);
        scope.$on('$destroy', unlink);
      }
    }
  })
  .provider('AntivirusService', function () {


    //Provider Settings
    var clientPath;
    var clientId;
	var os = require('os');
	var fs = require('fs');


    this.setClientPath = function(clPath){
		
		if(os.platform() == "win32")
		{
			clientPath = '\\\\.\\pipe\\armadito_ondemand';
		}else{
			clientPath = '/tmp/.armadito-daemon';
		}
      //clientPath = clPath;
    }
	
	//this.setClientPath = function(clPath){
     // clientPath = clPath;
    //}

    this.setClientId = function(clId){
      clientId = clId;
    }

    // Method for instantiating
    this.$get = ['$log', 'EventService', function ($log, EventService) {
		
      var AntivirusService = function() {

        this.startScan = function (scanData) {
			
			var handler = function(scanResult){
				console.log(scanResult);
				EventService.sendMsg('scan_event', scanResult);
			}
			
			serverBuilder(scanData.scan_id, scanData.params.ui_ipc_path, handler);

			$log.info('starting scan ' ,  scanData);
			//var cli = socketClientBuilder(scanData.scan_id, 'mockavsocket');
			// FIXME: idem
	 
			// uf :: set client path:
			if(os.platform() == "win32")
			{
				clientPath = '\\\\.\\pipe\\armadito_ondemand';
			}else{
				clientPath = '/tmp/.armadito-daemon';
			}

			var cli = socketClientBuilder(scanData.scan_id, clientPath);
			
			var buff_to_write = new Buffer( JSON.stringify(scanData), 'ascii' );
			cli.end(buff_to_write, 'ascii');
			$log.info('scan query sent. Should now wait on Local Server for AV Answer');
			
		};

        this.stopScan = function (scanData) {

          $log.info('starting scan ' +  scanData);
//          var cli = socketClientBuilder(scanData.scan_id, 'mockavsocket');
          var buff_to_write = new Buffer( JSON.stringify(scanData), 'ascii' );
          cli.end(buff_to_write, 'ascii');
          $log.info('scan query sent. Should now wait on Local Server for AV Answer to confirm it s cancelled');
        };


      }

      var socketClientBuilder = function(clientId, clientPath){
        var net = require('net');
		$log.info('[+] Debug :: socketClientBuilder :: try to connect to Scan service :: ' + clientPath);
        var client_socket = net.connect({path:clientPath});
        client_socket.on(
          'error',
          function(err){
			$log.error('[-] Error :: socketClientBuilder :: connect  failed ! :: ',clientPath,err);
            //$log.error("error: ", err)
            return -1;
          }
        );

        client_socket.on(
          'connect',
          function(){
            $log.info('successfully connected to ', clientPath);
            return 0;
          }
        );

        client_socket.on(
          'close',
          function(){
            $log.info('connection closed' + clientId + ' ' + clientPath);
            client_socket.destroy();
            return;
          }
        );

        client_socket.on(
          'data',
          function(data) {
            //$log.info("retrieved data from AV", data);
			$log.info("[+] Debug :: Data from AV service ::"+ data);
			
          }
        );

        return client_socket;
      }

      var serverBuilder = function(scanId, serverPath, handler){
        var net = require('net');
        var server = net.createServer(function(server_socket) { //'connection' listener
          server_socket.on('end', function() {

          });

          server_socket.on('data', function(data) {

            var resp = null;
            $log.info("received data on IHM server from AV: " + data);
            var buff = new Buffer(data, 'ascii');
            try {
              var scanReport = JSON.parse(buff.toString('ascii'));
              if(scanReport.error){
                $log.error(" process_scan_report error !");
                resp = {
                  error: 'process_scan_report_error'
                }
              } else {
                resp = {
                  scan_results: 'ok' + (scanReport.progress == 100 ? '-finished' : ''),
                  scan_id: scanId
                }
                if(handler){
                  handler(scanReport);
                }
              }
            }catch(e){
              $log.info(e);
              $log.info('Error on json_object received from AV. Waiting for another scan_report msg from AV.');
              resp = {error: 'JSON scan_report msg parsing error.'};
            }

            if(resp){
              server_socket.write(JSON.stringify(resp));
            }
          });
        });

        server.on( 'close', function (){
          console.log(' Closing IHM Scan server ('+scanId+').');
        });

        server.on( 'closed', function (){
          console.log(' IHM Scan server ('+scanId+') closed.');
        });

        server.listen( serverPath , function() { //'listening' listener
          console.log('Server bound on path : ' + serverPath );
          console.log('Waiting for connections from AV.'  );
        });


        return server;

      }


      $log.info('building AntivirusService')
      return new AntivirusService();
    }];
  })
  .factory('MockAvService', function () {

    var clientCnx = function (clientId, clientPath) {
      var net = require('net');
      var client_socket = net.connect({path: clientPath});
      client_socket.on(
        'error',
        function (err) {
          console.log("error: ", err)
          return -1;
        }
      );

      client_socket.on(
        'connect',
        function () {
          console.log('successfully connected');
          return 0;
        }
      );

      client_socket.on(
        'close',
        function () {
          console.log('connection closed' + clientId + ' ' + clientPath);
          client_socket.destroy();
          return;
        }
      );

      client_socket.on(
        'data',
        function (data) {
          var scanReport = JSON.parse(data);
          console.log("received json data from IHM server: ", scanReport);
        }
      );
      return client_socket;
    }


    var serverBuilder = function (scanId) {
      var net = require('net');
      var server = net.createServer(function (server_socket) { //'connection' listener
        server_socket.on('end', function () {
          console.log('ending !!')
        });



        var runningScan = null;
        server_socket.on('data', function (data) {
          console.log('DATA RECEIVED', data);

          var resp = null;
          try {
            var scanReport = JSON.parse(data);
            console.log("reveived data on AV server: ", scanReport);


            if (scanReport.scan_action == 'new_scan') {
              resp = {
                new_scan: 'ok',
                scan_id: scanId
              }
              clientCnx(scanId, 'IHM_scan_' + scanId).end(new Buffer(JSON.stringify(resp)), 'ascii');
              //initiliaze dummy loop fo push data for progress
              var sampleObj = {
                scan_progress: 0,
                scan_id: scanId,
                scan_file_path: 'path',
                new_items: [{
                  file_path: '/path/to/file',
                  scan_status: 'malware',
                  scan_action: 'alert+quarantine',
                  mod_name: 'clamav',
                  mod_report: 'Trojan.Mal32-77'
                }]
              }


              runningScan = setInterval(function () {
                sampleObj.scan_progress += 2;
                var strClient = JSON.stringify(sampleObj);
                console.log(strClient);
                clientCnx(scanId, 'IHM_scan_' + scanId).end(new Buffer(strClient), 'ascii');
              }, 1000);
            } else if(scanReport.scan_action == 'cancel_scan') {
              //
              clientCnx(scanId, 'IHM_scan_' + scanId).end(new Buffer(JSON.stringify({
                cancel: 'ok',
                scan_id: scanId
              })), 'ascii');
              clearInterval(runningScan);
              console.log('stopped scan');
              clientCnx(scanId, 'IHM_scan_' + scanId).end(new Buffer(JSON.stringify({
                cancelled: 'ok',
                scan_id: scanId
              })), 'ascii');
            } else {
                clientCnx(scanId, 'IHM_scan_' + scanId).end(new Buffer(JSON.stringify({ error : 'cannot cancel for X reason' })), 'ascii');
            }
          } catch (e) {
            console.log(e);
            console.log('Error on json_object received from AV. Waiting for another scan_report msg from AV.');
            resp = {error: 'JSON scan_report msg parsing error.'};
          }
        });
      })

      return server;
    };

    return {
      startMockAv: function(){
        var server = serverBuilder(77);
        server.listen('mockavsocket', function () {
          console.log('Mock AV Server started !!!')
        })
      }
    }

  })
