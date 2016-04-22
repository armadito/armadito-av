'use strict';

/**
 * @ngdoc directive
 * @name tatouApp.directive:schrollBottom
 * @description
 * # schrollBottom
 */
angular.module('tatouApp')
  .directive('schrollBottom', function () {
     return {
	    scope: {
	      schrollBottom: "="
	    },
	    link: function (scope, element) {
	      scope.$watchCollection('schrollBottom', function (newValue) {
	        if (newValue)
	        {
	          $(element).scrollTop($(element)[0].scrollHeight);
	        }
	      });
	    }
	  }
  });
