/*!---------------------------------------------------------
* Copyright (C) Genius Corporation. All rights reserved.
*--------------------------------------------------------*/

var env = require('../Generic/env');
var DeviceObj = require('./DeviceDB'); 

var AppDB = (function (){
  var _this; 
  function AppDB() {
      	_this = this;
        _this.DeviceDB = new DeviceObj.DeviceDB();
   }   

    AppDB.prototype.getDevice = function(callback){
        _this.DeviceDB.getDevice(function(data){
            callback(data)
        })
    }; 

    AppDB.prototype.insertDevice = function(callback){
        _this.DeviceDB.insertDevice(function(data){
            callback(data)
        })
    }; 

    AppDB.prototype.getDevice2 = function(callback){
        return new Promise(function (resolve, reject) 
        {
            return _this.DeviceDB.getDevice2().then(function(data){
                callback(data);
            })
        });
    }; 

    AppDB.prototype.DeviceDB = undefined;

    return AppDB;  

})()

exports.AppDB = AppDB;