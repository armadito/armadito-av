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
var a6oApp = angular
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

a6oApp.factory('ScanDataFactory', function () {
	return {
	    data: {
	      suspicious_count: 0,
	      scanned_count: 0,
	      malware_count: 0,
	      scan_progress: 0,
	      progress: 0,
	      canceled: 0,
	      path_to_scan: "",
	      displayed_file: "",
	      type: "analyse_view.Choose_analyse_type",
	      files: []
	    },
	    update_counters: function(_scanned_count, _suspicious_count, _malware_count, _progress ) {
	      // Improve this method as needed
	      this.data.suspicious_count = _suspicious_count;
	      this.data.scanned_count = _scanned_count;
              this.data.malware_count = _malware_count;
	      this.data.progress = _progress;
	    },
	    set_displayed_file: function (_displayed_file) {
	      this.data.displayed_file = _displayed_file;
	    },
	    add_scanned_file: function (_file_path, _file_scan_status, _file_scan_action , _file_mod_name, _file_mod_report) {
              var file = {
		path: _file_path,
		scan_status: _file_scan_status,
		scan_action: _file_scan_action,
		mod_name: _file_mod_name,
                mod_report: _file_mod_report
	      };

	      this.data.files.push(file);
              file = null;
	    },
	    set_scan_conf: function (_path_to_scan, _type) {
	      this.data.path_to_scan =  _path_to_scan;
	      this.data.type = _type;
	    },
            setCanceled: function () {
              this.data.canceled = 1;
	    },
	    reset: function() {
	           this.data = {
		      suspicious_count: 0,
		      scanned_count: 0,
		      malware_count: 0,
		      progress: 0,
		      canceled : 0,
		      path_to_scan: "",
		      displayed_file: "",
		      type: "analyse_view.Choose_analyse_type",
		      files: []
	           };
	    }
	};

});
