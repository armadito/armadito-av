'use strict';

describe('Controller: RapportdetailscontrollerCtrl', function () {

  // load the controller's module
  beforeEach(module('tatouApp'));

  var RapportdetailscontrollerCtrl,
    scope;

  // Initialize the controller and a mock scope
  beforeEach(inject(function ($controller, $rootScope) {
    scope = $rootScope.$new();
    RapportdetailscontrollerCtrl = $controller('RapportdetailscontrollerCtrl', {
      $scope: scope
      // place here mocked dependencies
    });
  }));

  it('should attach a list of awesomeThings to the scope', function () {
    expect(RapportdetailscontrollerCtrl.awesomeThings.length).toBe(3);
  });
});
