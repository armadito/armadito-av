'use strict';

describe('Controller: ScancontrollerCtrl', function () {

  // load the controller's module
  beforeEach(module('tatouApp'));

  var ScancontrollerCtrl,
    scope;

  // Initialize the controller and a mock scope
  beforeEach(inject(function ($controller, $rootScope) {
    scope = $rootScope.$new();
    ScancontrollerCtrl = $controller('ScancontrollerCtrl', {
      $scope: scope
      // place here mocked dependencies
    });
  }));

  it('should attach a list of awesomeThings to the scope', function () {
    expect(ScancontrollerCtrl.awesomeThings.length).toBe(3);
  });
});
