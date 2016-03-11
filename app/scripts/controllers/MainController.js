'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:MainController
 * @description
 * # MainController
 * Controller of the tatouApp
 */
angular.module('tatouApp')
  .controller('MainController', [ '$scope', '$state','toastr','ArmaditoSVC', function ($scope,  $state, toastr, ArmaditoSVC) {

  	//Buttons
  	$scope.buttons = [
  			{ 
  				button : {
		  			isActive : true,
		  			tittle : "Information",
		  			icon : "fa fa-desktop fa-2x",
		  			view : 'Main.Information'
		  		}
	  		},
	  		{
	  			button : {
		  			isActive : false,
		  			tittle : 'Scan',
		  			icon : 'fa fa-search fa-2x',
		  			view : 'Main.Scan'
		  		}
	  		},
	  		{
	  			button : {
		  			isActive : false,
		  			tittle : "Journal",
		  			icon : "fa fa-newspaper-o fa-2x",
		  			view : 'Main.Journal'
		  		}
	  		},
	  		{
	  			button : {
		  			isActive : false,
		  			tittle : "Parameters",
		  			icon : "fa fa-cogs fa-2x",
		  			view : 'Main.Parameters'
		  		}	
	  		}
	];

	$scope.activeButton = function (button){
		for (var i = 0; i < $scope.buttons.length; i++) {
			$scope.buttons[i].button.isActive = false;
		};
		button.isActive = true;
	};

  	$scope.refresh = function () {
  		$state.go('Main.Information');
  	};

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

  	ArmaditoSVC.startNotificationServer($scope.displayNotification);

  	$scope.refresh();

  }]);
