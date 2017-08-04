/*!---------------------------------------------------------
* Copyright (C) Genius Corporation. All rights reserved.
*--------------------------------------------------------*/

var env = require('../../backend/generic/env');

'use strict';

var HIDInterface = (function () {
    var _this;
    function HIDInterface() {
    	env.log(env.level.INFO,'HID','HIDInterface','New HIDInterface INSTANCE.');
    	_this = this;
    	if (env.isWindows){
    		_this.hid = require(`./win32/Release/${env.arch}_hidWin.node`);
    	}else if (env.isMac){
    		_this.hid = require('./darwin/Release/hidDarwin.node');
    	}else{
    		env.log(env.level.INFO,'HID','HIDInterface','This platform not support.');
    	}
        if (_this.hid === undefined)
        	throw new Error('HIDInterface can not init.'); 
        //_this.mDeviceList = new Map();
    }
    
    HIDInterface.prototype.GetDeviceList = function () {
    	return this.hid.getDeviceList();
    };

    HIDInterface.prototype.GetDevice = function (dev){
        return new _this.hid.Device(dev);
    };

    HIDInterface.prototype.Open = function (dev, iFaceType, reportNumber){
    	return dev.open(iFaceType, reportNumber);
    };

    HIDInterface.prototype.Close = function (dev){
    	dev.close();
    };

    HIDInterface.prototype.OpenTransfer = function (dev, iFaceType, reportNumber, SN, callback, reportNumArray){
        if (reportNumArray === undefined || reportNumArray === null)
    	   return dev.startTransfer(iFaceType, reportNumber, SN, callback);
        else
           return dev.startTransfer(iFaceType, reportNumber, SN, callback, reportNumArray);
    };

    HIDInterface.prototype.SetTransferSN = function (dev, iFaceType, reportNumber, SN){
        return dev.setTransferSn(iFaceType, reportNumber, SN);
    };

    HIDInterface.prototype.GetFeatureReport = function (dev, reportId, wLength){
    	return dev.hidGetReport(reportId, wLength);
    };

    HIDInterface.prototype.SendFeatureReport = function (dev, reportId, wLength, bBuffer){
    	return dev.hidSetReport(reportId, wLength, bBuffer);
    };
    
    HIDInterface.prototype.HIDRead = function (dev, reportId, wLength){
    	return dev.hidRead(reportId, wLength);
    };

    HIDInterface.prototype.HIDWrite = function (dev, reportId, wLength, bBuffer){
    	return dev.hidWrite(reportId, wLength, bBuffer);
    };

    HIDInterface.prototype.dispose = function () {
    	/*var _this = this;
    	_this.mDeviceList.forEach(function(value, key) {
		 	_this.Close(value);
		}, _this.mDeviceList);
		_this.mDeviceList.clear();*/
    }

    HIDInterface.prototype.hid = undefined;
    
    return HIDInterface;
})();

exports.HID = HIDInterface;
exports.HIDInstance = new HIDInterface();