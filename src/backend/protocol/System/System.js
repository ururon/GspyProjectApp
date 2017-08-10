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
var env = require('../../generic/env');
var funcVar = require('../../generic/FunctionVariates');
var evtType = require('../../generic/EventVariates').EventTypes;
var errCode = require('../../generic/ErrorCode').ErrorCode;
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

    SystemApi.prototype.abctest = function (p, callback) {
        env.log(env.level.INFO,'system','abctest',JSON.stringify(p));
        callback('success','abc');
    };

    //系统函数
    SystemApi.prototype.System = undefined;	
    
    return SystemApi;
})(events.EventEmitter);

exports.System = SystemApi;