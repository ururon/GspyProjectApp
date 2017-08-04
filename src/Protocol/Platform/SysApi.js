/*!---------------------------------------------------------
* Copyright (C) Genius Corporation. All rights reserved.
*--------------------------------------------------------*/

var env = require('../../backend/generic/env');

'use strict';

var SysInterface = (function () {
    var _this;
    function SysInterface() {
        env.log(env.level.DEBUG,"[SysApi]","[SysInterface]",`[New SysInterface INSTANCE]`);
    	_this = this;
    	if (env.isWindows){
    		_this.sysApi = require(`./win32/Release/${env.arch}_sysWin.node`);
    	}else if (env.isMac){
    		_this.sysApi = require('./darwin/Release/sysDarwin.node');
    	}else{
            env.log("[MSG]","[Interface]","[SysInterface]",`[This platform not support]`);
    	}
        if (_this.sysApi === undefined)
        	throw new Error('SysInterface can not init.');

        _this.sysApi.RegisterLogMessage(function(msg){
            env.log(env.level.DEBUG,'SysApi','SysInterface',`${msg}`);
        });

    }
    //返回路径
    SysInterface.prototype.GetForegroundApp = function () {
    	return _this.sysApi.GetForegroundApp();
    };

    SysInterface.prototype.GetMouseSpeed = function () {
        return _this.sysApi.GetMouseSpeed();
    };

    SysInterface.prototype.SetMouseSpeed = function (speed) {
        return _this.sysApi.SetMouseSpeed(speed);
    };

    SysInterface.prototype.GetDoubleClickTime = function () {
        return _this.sysApi.GetDoubleClickTime();
    };

    SysInterface.prototype.SetDoubleClickTime = function (tmr) {
        return _this.sysApi.SetDoubleClickTime(tmr);
    };

    SysInterface.prototype.GetSwapButton = function () {
        return _this.sysApi.GetSwapButton();
    };
    //win btn为1,0
    SysInterface.prototype.SetSwapButton = function (btn) {
        return _this.sysApi.SetSwapButton(btn);
    };

    SysInterface.prototype.GetWheelScrollLines = function () {
        return _this.sysApi.GetWheelScrollLines();
    };

    SysInterface.prototype.SetWheelScrollLines = function (lines) {
        return _this.sysApi.SetWheelScrollLines(lines);
    };

    SysInterface.prototype.RegisterAppChange = function (callback) {
        return _this.sysApi.RegisterAppChange(callback);
    };

    SysInterface.prototype.RegisterHotplug = function (callback) {
        return _this.sysApi.RegisterHotplug(callback);
    };

    SysInterface.prototype.RegisterSessionChange = function (callback) {
        return _this.sysApi.RegisterSessionChange(callback);
    };
   
    SysInterface.prototype.getApplicationIcon = function(sourceDir,toDir,iSize)
    {
        return _this.sysApi.getApplicationIcon(sourceDir,toDir,iSize);
    }

    SysInterface.prototype.SearchCommand = function(command) {
        var e, filename, fs, p, path, paths, _i, _len;
        if (command[0] === '/') {
          return command;
        }
        fs = require('fs');
        path = require('path');
        paths = process.env.PATH.split(path.delimiter);
        for (_i = 0, _len = paths.length; _i < _len; _i++) {
          p = paths[_i];
          try {
            filename = path.join(p, command);
            if (fs.statSync(filename).isFile()) {
              return filename;
            }
          } catch (_error) {
            e = _error;
          }
        }
        return '';
    };
    
    SysInterface.prototype.Runas = function (command, args, options) {
        if (args == null || args == undefined) {
            args = [];
        }
        if (options == null || options == undefined) {
            options = {};
        }
        if (options.hide == null || options.hide == undefined) {
            options.hide = true;
        }
        if (options.admin == null || options.admin == undefined) {
            options.admin = false;
        }
        if (env.isMac && options.admin === true) {
            command = _this.SearchCommand(command);
        }
        return _this.sysApi.Runas(command, args, options);
    };

    SysInterface.prototype.dispose = function () {
    	_this.sysApi = undefined;
    }

    SysInterface.prototype.sysApi = undefined;
    
    return SysInterface;
})();

exports.SysApi = SysInterface;
exports.SysApiInstance = new SysInterface();