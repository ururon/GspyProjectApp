/*!---------------------------------------------------------
* Copyright (C) Genius Corporation. All rights reserved.
*--------------------------------------------------------*/

var __extends = this.__extends || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    __.prototype = b.prototype;
    d.prototype = new __();
};

var events = require('events');
var fs = require('fs');
var env = require('../../backend/generic/env');
var funcVar = require('../../backend/generic/FunctionVariates');
var evtType = require('../../backend/generic/EventVariates').EventTypes;
var errCode = require('../../backend/generic/ErrorCode').ErrorCode;
var pSystem = require('../Platform/SysApi');

'use strict';

var SystemApi = (function (_super) {
    __extends(SystemApi, _super);
    var _this;
    function SystemApi() {
    	try{
    		_this = this;
            _super.call(this);
	        env.log(env.level.INFO,'System','SystemApi','New SystemApi INSTANCE.');
	        if (_this.System === undefined){
	        	_this.System = pSystem.SysApiInstance;
	        	_this.System.RegisterAppChange(_this.AppChangeCallback);
	        	_this.System.RegisterHotplug(_this.HotplugCallback);
                if (env.isWindows)
	        	  _this.System.RegisterSessionChange(_this.SessionChangeCallback);
	        }
	    }catch(ex){
            env.log(env.level.DEBUG,System,'SystemApi','[SystemApi Error]',ex.message);
        }
    }

    SystemApi.prototype.AppChangeCallback = function (path) {
    	_this.emit(evtType.AppChanged, path);
    };

    SystemApi.prototype.HotplugCallback = function (baseInfo, extInfo) {
        env.log(env.level.INFO,'System','HotplugCallback', JSON.stringify(baseInfo),extInfo)
    	_this.emit(evtType.HotPlug, baseInfo, extInfo);
    };

    SystemApi.prototype.SessionChangeCallback = function (changeType) {
    	_this.emit(evtType.SessionChanged, changeType);
    };

    SystemApi.prototype.GetForegroundApp = function () {
        return this.System.GetForegroundApp();
    };

    SystemApi.prototype.RunFunction = function (Obj,callback) {
    	try{
            env.log(env.level.INFO,'System','RunFunction',JSON.stringify(Obj))
	    	if (Obj.Type !== funcVar.Types.System)
	    		throw new Error('Type must be System.');

	    	var fn = _this[Obj.Func];

	    	if (fn === undefined || !funcVar.Names.hasOwnProperty(Obj.Func))
	    		throw new Error(`Func error of ${Obj.Func}`);
	    	fn(Obj.Param, callback);
	    }catch(ex){
            env.log(env.level.DEBUG,'System','RunFunction',`SystemApi.RunFunction error : ${ex.message}.`);
	    	callback(errCode.ValidateError, ex);
	    }
    };

    SystemApi.prototype.GetMousePointSpeed = function (p, callback) {
        callback(_this.System.GetMouseSpeed());
    };

    SystemApi.prototype.SetMousePointSpeed = function (speed, callback) {
        _this.System.SetMouseSpeed(Number(speed));
        callback();
    };

    SystemApi.prototype.GetMouseDoubleClickSpeed = function (p, callback) {
        callback(_this.System.GetDoubleClickTime());
    };

    SystemApi.prototype.SetMouseDoubleClickSpeed = function (tmr, callback) {
        _this.System.SetDoubleClickTime(Number(tmr));
        callback();
    };

    SystemApi.prototype.GetSwapButtonState = function (p, callback) {
        callback(_this.System.GetSwapButton());
    };

    SystemApi.prototype.SetSwapButtonState = function (btn, callback) {
        _this.System.SetSwapButton(Number(btn));
        callback();
    };

    SystemApi.prototype.GetDeviceWheelSpeed = function (p, callback) {
        callback(_this.System.GetWheelScrollLines());
    };

    SystemApi.prototype.SetDeviceWheelSpeed = function (scroll, callback) {
        _this.System.SetWheelScrollLines(Number(scroll));
        callback();
    };

    //设置语言
    SystemApi.prototype.SetLanguage = function (lang, callback) {
        try{
            if(lang === undefined)
            {
                env.log(env.level.INFO,System,'System','SetLanguage','SetLanguage error : lang is null.');
                callback();
                return;
            }
            var appSettings = JSON.parse(fs.readFileSync(env.appSettingsPath,'UTF-8'));
            lang = lang.toLowerCase();
            if(appSettings.SystemConfig.Language != lang)
            {
                appSettings.SystemConfig.Language = lang;
                if(env.isTestMode){
                    if(appSettings.SystemConfig.Language == "cn")
                        appSettings.UpdateURI.UpdateURLLast = appSettings.UpdateURI.UpdateURLTestCN;
                    else if(appSettings.SystemConfig.Language == "tw")
                        appSettings.UpdateURI.UpdateURLLast = appSettings.UpdateURI.UpdateURLTestTW;
                    else
                        appSettings.UpdateURI.UpdateURLLast = appSettings.UpdateURI.UpdateURLTestEN;
                }else{
                    if(appSettings.SystemConfig.Language == "cn")
                        appSettings.UpdateURI.UpdateURLLast = appSettings.UpdateURI.UpdateURLCN;
                    else if(appSettings.SystemConfig.Language == "tw")
                        appSettings.UpdateURI.UpdateURLLast = appSettings.UpdateURI.UpdateURLTW;
                    else
                        appSettings.UpdateURI.UpdateURLLast = appSettings.UpdateURI.UpdateURLEN;
                }
                fs.writeFileSync(env.appSettingsPath, JSON.stringify(appSettings, null, "\t"));
            }
            callback();
        }catch(e){
            env.log(env.level.DEBUG,'System','SetLanguage','SetLanguage error : '+ e.message);
            callback();
        }
    };

    //系统函数
    SystemApi.prototype.System = undefined;	
    
    return SystemApi;
})(events.EventEmitter);

exports.System = SystemApi;