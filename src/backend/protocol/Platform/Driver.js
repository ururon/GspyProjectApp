/*!---------------------------------------------------------
* Copyright (C) Genius Corporation. All rights reserved.
*--------------------------------------------------------*/

var env = require('../../Generic/env');

'use strict';

var DriverInterface = (function () {
    var _this;
    function DriverInterface() {
        env.log("[MSG]","[Driver]","[DriverInterface]","[New DriverInterface INSTANCE]");
    	_this = this;
    	if (env.isWindows){
    		_this.DriverApi = require(`./win32/Release/${env.arch}_driverWin.node`);
    	}else if (env.isMac){
    		_this.DriverApi = require('./darwin/Release/driverDarwin.node');
    	}else{
    		env.log(env.level.INFO,'Driver','DriverInterface','This platform not support.');
    	}
        if (_this.DriverApi === undefined)
        	throw new Error('DriverInterface can not init.');
        _this.DriverApi.RegisterLogMessage(function(msg){
            env.log(env.level.INFO,'Driver','DriverInterface',`${msg}`);
        });
        if (_this.DriverApi.isOpen === undefined)
            _this.DriverApi.isOpen = false;
        if (_this.DriverApi.isKbOpen === undefined)
            _this.DriverApi.isKbOpen = false;
    }

    DriverInterface.prototype.CheckDriverInstall = function (drvType) {
        env.log("[MSG]","[Driver]","[CheckDriverInstall]",`[drvType:${drvType}]`);
    	return _this.DriverApi.CheckDriverInstall(drvType);
    };

    DriverInterface.prototype.OpenDriver = function () {
        if (!_this.DriverApi.isOpen){
    	   _this.DriverApi.OpenDriver();
           _this.DriverApi.isOpen = true;
        }
    };

    DriverInterface.prototype.CloseDriver = function () {
        env.log("[MSG]","[Driver]","[CloseDriver]",`[Close Driver]`);
        if (_this.DriverApi.isOpen){
            _this.DriverApi.CloseDriver();
            _this.DriverApi.isOpen = false;
        }
    };
    //macro为一个数组包含{KeyNum:1,MacroType:[1,4],MacroContent:[array]}
    DriverInterface.prototype.SetMacroKey = function (vid, pid, macro, sn, location) {
        if (!_this.DriverApi.isOpen)
            throw new Error("Please OpenDriver first.");
        return _this.DriverApi.SetMacroKey(vid, pid, macro, sn, location);
    };

    DriverInterface.prototype.SetAllMacroKey = function (vid, pid, macro, sn, location) {
        if (!_this.DriverApi.isKbOpen)
            throw new Error("Please Open Keyboard Driver first.");
        return _this.DriverApi.SetAllMacroKey(vid, pid, macro, sn, location);
    };

    DriverInterface.prototype.GetBitsOfn = function (n, l){
        return String("00000000" + n.toString(2)).slice(-l);
    };

    DriverInterface.prototype.IsOpen = function (){
        if (_this.DriverApi.isOpen === undefined)
            _this.DriverApi.isOpen = false;
        return _this.DriverApi.isOpen;
    };

    DriverInterface.prototype.AmmoxGetDPI = function (vid, pid, location){
        var dpis = _this.DriverApi.AmmoxGetDPI(vid, pid, location);
        var dpiRet = {};
        var dpi1 = _this.GetBitsOfn(dpis.DPI1, 8);
        var dpi3 = _this.GetBitsOfn(dpis.DPI2, 8);
        dpiRet.DPI1 = _this.AmmoxDPIVals[parseInt(dpi1.slice(5, 8), 2)];
        dpiRet.DPI2 = _this.AmmoxDPIVals[parseInt(dpi1.slice(2, 5), 2)];
        dpiRet.DPI3 = _this.AmmoxDPIVals[parseInt(dpi3.slice(5, 8), 2)];
        dpiRet.DPI4 = _this.AmmoxDPIVals[parseInt(dpi3.slice(2, 5), 2)];
        return dpiRet;
    };

    DriverInterface.prototype.AmmoxSetDPI = function (vid, pid, dpi1, dpi2, dpi3, dpi4, location){
        var funGetIndex = function(d){
            for (var i = 0; i < _this.AmmoxDPIVals.length; i++) {
                if (_this.AmmoxDPIVals[i] == d)
                    return i;
            }
            return -1;
        }
        dpi1 = funGetIndex(dpi1);
        if (dpi1 == -1)
            throw new Error('dpi1 error.');
        dpi2 = funGetIndex(dpi2);
        if (dpi2 == -1)
            throw new Error('dpi2 error.');
        dpi3 = funGetIndex(dpi3);
        if (dpi3 == -1)
            throw new Error('dpi3 error.');
        dpi4 = funGetIndex(dpi4);
        if (dpi4 == -1)
            throw new Error('dpi4 error.');
        var dpi1_2 = parseInt(parseInt("10" + _this.GetBitsOfn(dpi2, 3) + _this.GetBitsOfn(dpi1, 3), 2).toString(16)+"10", 16);
        var dpi3_4 = parseInt(parseInt("00" + _this.GetBitsOfn(dpi4, 3) + _this.GetBitsOfn(dpi3, 3), 2).toString(16)+"11", 16);
        return _this.DriverApi.AmmoxSetDPI(vid, pid, dpi1_2, dpi3_4, location);
    };

    DriverInterface.prototype.RegisterTransfer = function (vid, pid, sn, location, callback){
        return _this.DriverApi.RegisterTransfer(vid, pid, sn, location, callback);
    };

    DriverInterface.prototype.GetDriverVersion = function (){
        if (env.isMac){
            var cp = require('child_process');
            var rt = cp.execSync('/usr/sbin/kextstat -kl | awk \'/com.geniusnet.driver.DeviceFilter/\{printf \"%s\\n\",$7}\'');
            if (rt.length <= 0)
                return undefined;
            rt = rt.toString("utf8").replace('(','').replace(')','').replace(/^[\s\uFEFF\xA0]+|[\s\uFEFF\xA0]+$/g, '');
            return rt;
        }else if(env.isWindows){
            return _this.DriverApi.GetDriverVersion();
        }
    };

    DriverInterface.prototype.dispose = function () {
    	if (_this.DriverApi.isOpen)
            _this.CloseDriver();
    }

    DriverInterface.prototype.DriverApi = undefined;
    DriverInterface.prototype.AmmoxDPIVals = new Array(400,800,1200,1600,2000,2400,2800,3200);



    /////////////////////////// Keyboard ////////////////////////////////////
    DriverInterface.prototype.OpenKbDriver = function () {
        env.log("[MSG]","[Driver]","[OpenKbDriver]",`[Open Keyboard Driver]`);
        if (!_this.DriverApi.isKbOpen){
           _this.DriverApi.OpenKbDriver();
           _this.DriverApi.isKbOpen = true;
        }
    };

    DriverInterface.prototype.CloseKbDriver = function () {
        if (_this.DriverApi.isKbOpen){
            env.log("[MSG]","[Driver]","[CloseKbDriver]",`[Close Keyboard Driver]`);
            _this.DriverApi.CloseKbDriver();
            _this.DriverApi.isKbOpen = false;
        }
    };

    DriverInterface.prototype.SetKbFunctionAction = function (dev, KeyAction) {
        env.log("[MSG]","[Driver]","[SetKbFunctionAction]",`[Set Keyboard function action]`);
        if (!_this.DriverApi.isKbOpen)
            throw new Error("Please Open Keyboard Driver first.");
        else
        {
            _this.DriverApi.SetKbFunctionAction(parseInt(dev.BaseInf.VID), parseInt(dev.BaseInf.PID), KeyAction, dev.DeviceId, dev.LocationPath);
            
            if(dev.LocationPath2 != null)
            {
                env.log("[MSG]","[Driver]","[SetKbFunctionAction]",`[dev.LocationPath2 :${dev.LocationPath2}]`);
                _this.DriverApi.SetKbFunctionAction(parseInt(dev.BaseInf.VID), parseInt(dev.BaseInf.PID), KeyAction, dev.DeviceId, dev.LocationPath2);
            }
        }

    };
    ///////////////////////////////////////////////////////////////

    return DriverInterface;
})();

exports.Driver = DriverInterface;
exports.DriverInstance = new DriverInterface();
