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
 * @ngdoc overview
 * @name armaditoApp
 * @description
 * # armaditoApp
 *
 * Main module of the application.
 */
angular
  .module('armaditoApp', [
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
    'armadito.svc',
    'armadito.ipc',
    'timer',
    'toastr',
    'ngTagsInput',
    'pascalprecht.translate',
    'truncate'
  ])
  .config(function ($stateProvider, $urlRouterProvider, toastrConfig, $translateProvider) {
	  
	   angular.extend(toastrConfig, {
			progressBar: false,
			tapToDismiss: true,
			timeOut: 5000,
			maxOpened: 1
		  });
     
      //Enable a more secure variant escape 
      $translateProvider.useSanitizeValueStrategy('escape');

      $translateProvider.useStaticFilesLoader({
        prefix: 'scripts/filters/languages/',
        suffix: '.js'
      });
      // Tell the module what language to use by default
      $translateProvider.preferredLanguage('en_US');

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

