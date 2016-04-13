'use strict';
/*Create NWJS TRAY*/
// Create a tray icon
/*var tray = new nw.Tray({ title: 'Tray', icon: 'img/icon.png' });

// Give it a menu
var menu = new nw.Menu();
menu.append(new nw.MenuItem({ type: 'checkbox', label: 'box1' }));
tray.menu = menu;*/

// Remove the tray
/*tray.remove();
tray = null;*/

/*Create NWJS TRAY*/
/**
 * @ngdoc overview
 * @name tatouApp
 * @description
 * # tatouApp
 *
 * Main module of the application.
 */
angular
  .module('tatouApp', [
    'ngAnimate',
    'ngAria',
    'ngCookies',
    'ngMessages',
    'ngResource',
    'ngRoute',
    'ngSanitize',
    'ngTouch',
    'ui.router',
    'ui.bootstrap',
    'armadito.services',
	  'armadito.svc',
	  'armadito.ipc',
    'timer',
    'toastr',
    'ngTagsInput',
    'pascalprecht.translate'
  ])
  .config(function ($stateProvider, $urlRouterProvider, toastrConfig, $translateProvider) {
	  
	   angular.extend(toastrConfig, {
			progressBar: false,
			tapToDismiss: true,
			timeOut: 5000,
			maxOpened: 1
		  });

      $translateProvider.useStaticFilesLoader({
        prefix: 'scripts/filters/languages/',
        suffix: '.js'
      });
      // Tell the module what language to use by default
      $translateProvider.preferredLanguage('fr_FR');

   //
  // For any unmatched url, redirect to /main
  $urlRouterProvider.otherwise("/Main");
  //
  // Set up the states
  $stateProvider
    .state('Main', {
      url: '/Main',
      templateUrl: 'views/Main.html',
      controller: 'MainController'
    })
    .state('Main.Information', {
      url: '/Information',
      templateUrl: 'views/Information.html',
      controller: 'InformationController'
    })
    .state('Main.Scan', {
      url: '/Scan',
      templateUrl: 'views/Scan.html',
      controller: 'ScanController'
    })
    .state('Main.Journal', {
      url: '/Journal',
      templateUrl: 'views/Journal.html',
      controller: 'JournalController'
    })
    .state('Main.Parameters', {
      url: '/Parameters',
      templateUrl: 'views/Parameters.html',
      controller: 'ParametersController'
    });
});

