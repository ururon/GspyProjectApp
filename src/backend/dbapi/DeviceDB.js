/*!---------------------------------------------------------
* Copyright (C) Genius Corporation. All rights reserved.
*--------------------------------------------------------*/

var db = require('./DB'); 
var env = require('../Generic/env');

//var remote = require('remote');

var DeviceDB = (function (){
    var _this; 
    function DeviceDB() {
      	_this = this;
        _this.DB =new db.DB(); 
    }

    DeviceDB.prototype.getDevice = function(callback){
        _this.DB.queryCmd('Device',{},function(mds){
            callback(mds);
        });
    };

    DeviceDB.prototype.getDevice2 = function(callback){
        return new Promise(function (resolve, reject) {
            return  _this.DB.queryCmd('Device',{},function(docs){  
                resolve(docs);     
            });  
        });
    };

    DeviceDB.prototype.insertDevice = function(callback){
        var obj={
            "VID":"0458",
            "PID":"6001",
            "DeviceName":"Robert"
        }
        _this.DB.insertCmd('Device',obj,function(mds){
            callback(mds);
        });
    };


    DeviceDB.prototype.DB = undefined;  
  
    return DeviceDB;  

})()

exports.DeviceDB = DeviceDB;