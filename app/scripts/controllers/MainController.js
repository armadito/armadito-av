'use strict';

/**
 * @ngdoc function
 * @name tatouApp.controller:MainController
 * @description
 * # MainController
 * Controller of the tatouApp
 */
angular.module('tatouApp')
  .controller('MainController', [ '$scope', '$state', function ($scope,  $state) {

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

  	$scope.refresh();

  }]);
