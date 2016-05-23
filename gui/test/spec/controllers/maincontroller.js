'use strict';

describe('Controller: MaincontrollerCtrl', function () {

  // load the controller's module
  beforeEach(module('armaditoApp'));

  var MaincontrollerCtrl,
    scope;

  // Initialize the controller and a mock scope
  beforeEach(inject(function ($controller, $rootScope) {
    scope = $rootScope.$new();
    MaincontrollerCtrl = $controller('MaincontrollerCtrl', {
      $scope: scope
      // place here mocked dependencies
    });
  }));

  it('should attach a list of awesomeThings to the scope', function () {
    expect(MaincontrollerCtrl.awesomeThings.length).toBe(3);
  });
});
