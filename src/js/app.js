'use strict';

// Declare app level module which depends on filters, and services
var app = angular.module('mainapp', [
            'app.controllers',
            'app.services'
        ])
        .controller('FirstCtrl',function($scope){
            $scope.test = function(){
                env.log(env.level.INFO,'app','FirstCtrl','FirstCtrl')
                alert('aaa');
            }
        })

