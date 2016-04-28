'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:MainController
 * @description
 * # MainController
 * Controller of the tatouApp
 */

// Load native UI library
var osLocale = require('os-locale');
var gui = require('nw.gui');
// Create a tray icon
var tray = new gui.Tray({ title: 'Tray', icon: 'app/images/kimioslogoMini.png' });

// Reference to window and tray
var win = gui.Window.get();

// show window on tray click
tray.on('click', function() {
  win.show();
});

//var scan_in_progress = 0;
global.scan_in_progress = 0;

angular.module('tatouApp')
  .controller('MainController', [ '$rootScope', '$scope', '$state','$uibModal', '$translate', 'toastr','ArmaditoSVC', function ($rootScope, $scope,  $state, $uibModal, $translate, toastr, ArmaditoSVC) {

	osLocale(function (err, locale) {
	    console.log(locale);
	     $translate.use(locale);
	    //=> 'en_US'
	});

  	$scope.closeApp = function (){  

      	var size = 'sm';
      	var item = {
      		title : 'main_view.Leave',
      		sentence : "main_view.Are_you_sur_you_want_to_leave_Armadito"
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
      	 	window.close();
	    }, function () {
        	console.log('Modal dismissed at: ' + new Date());
      	});
  	};


  	$scope.reduceApp = function (){
  		win.minimize();
  		console.log(navigator.language);
  	};

  	//Buttons
  	$scope.buttons = [
  			{ 
  				button : {
		  			isActive : true,
		  			tittle : 'information_view.General',
		  			icon : "fa fa-tachometer fa-2x",
		  			view : 'Main.Information',
		  			backgroundColor: 'generalActive'
		  		}
	  		},
	  		{
	  			button : {
		  			isActive : false,
		  			tittle : 'analyse_view.Scan',
		  			icon : 'fa fa-search fa-2x',
		  			view : 'Main.Scan',
		  			backgroundColor: 'analyseActive'
		  		}
	  		},
	  		{
	  			button : {
		  			isActive : false,
		  			tittle : 'history_view.History',
		  			icon : 'fa fa-newspaper-o fa-2x',
		  			view : 'Main.Journal',
		  			backgroundColor:  'journalActive'
		  		}
	  		}/*,
	  		{
	  			button : {
		  			isActive : false,
		  			tittle : "Parameters",
		  			icon : "fa fa-cogs fa-2x",
		  			view : 'Main.Parameters'
		  		}	
	  		}*/
	];

	$rootScope.$on('$stateChangeStart', 
		function(event, toState, toParams, fromState, fromParams){
		    if(toState.url === '/Parameters'){
		    	for (var i = 0; i < $scope.buttons.length; i++) {
		    		$scope.buttons[i].button.isActive = false;
		    		$scope.buttonParameters = true;
		    	}
		    }else{
		    	$scope.buttonParameters = false;
		    }
		})

	$scope.activeButton = function (button){
		for (var i = 0; i < $scope.buttons.length; i++) {
			$scope.buttons[i].button.isActive = false;
		};
		button.isActive = true;
	};

  	$scope.refresh = function () {
  		$state.go('Main.Information');
  	};


  	// Event emitter
  	const EventEmitter = require('events');
  	const util = require('util');

  	function MyEmitter(){
  		EventEmitter.call(this);
  	};

  	$rootScope.myEmitter = new MyEmitter();

  	util.inherits(MyEmitter,EventEmitter);


  	$rootScope.myEmitter.on('notification', function(data){

  		console.log("[!] Event :: A [notification] event occured!\n");

  		$scope.displayNotification(data);

  	});  	

  	/* 
  		This function process data coming from av service :: 
  		- Notifications
  		- scan information
  	*/
  	$scope.processDataFromAV = function(data){

  		var json_object;  		

  		console.log("[+] Debug :: processDataFromAV :: "+data);

  		try{

  			json_object = JSON.parse(data);
  			
  			if(json_object.ihm_request == "notification"){

  				// Notifications.  				
  				$rootScope.myEmitter.emit('notification',data);

  			}else if(json_object.ihm_request == "scan"){

  				// scan information.
  				$rootScope.myEmitter.emit('scan_info',data);

  			}else{
  				console.log("[-] Error :: invalid request from av!\n");
  				return -1;
  			}

  		}  	
  		catch(e){
  			console.error("[-] Error :: Parsing error:", e); 
			return -1;
  		}

  		return 0;

  	}

  	$scope.displayNotification = function(data){

  		console.log("[i] Info :: displayNotification :: "+data);

  		var type;
  		var message;
  		var json_object;	
		
		try {

			json_object = JSON.parse(data);

			type = json_object.params.type;
			message = json_object.params.message;

			if(type == "INFO"){

				toastr.info(message, 'Info');

			}else if(type == "WARNING"){

				toastr.warning(message, 'Warning');

			}else if(type == "ERROR"){

				toastr.error(message, 'Error');

			}

				
		}
		catch(e){
			console.error("Parsing error:", e); 
			return null;
		}  		

  	};

  	ArmaditoSVC.startNotificationServer($scope.processDataFromAV);

  	$scope.refresh();

  }]);
