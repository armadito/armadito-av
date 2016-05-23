'use strict';

/**
 * @ngdoc directive
 * @name armaditoApp.directive:scrollBottom
 * @description
 * # schrollBottom
 */
angular.module('armaditoApp')
  .directive('scrollBottom', function () {
     return {
	    scope: {
	      scrollBottom: "="
	    },
	    link: function (scope, element) {
	      scope.$watchCollection('scrollBottom', function (newValue) {
	        if (newValue)
	        {
	          $(element).scrollTop($(element)[0].scrollHeight);
	        }
	      });
	    }
	  }
  });
