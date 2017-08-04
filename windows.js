/*!---------------------------------------------------------
* Copyright (C) Genius Corporation. All rights reserved.
*--------------------------------------------------------*/

var __extends = this.__extends || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    __.prototype = b.prototype;
    d.prototype = new __();
};

'use strict';
const electron = require('electron');
var app = electron.app;
var BrowserWindow =electron.BrowserWindow;
var events = require('events');
var env = require('./backend/generic/env');
 

var SmartWindow = (function (_super) {
    __extends(SmartWindow, _super);
    var _this;
    function SmartWindow() {
        _super.call(this);
        _this = this;
        _this.isCrashed = false;
        _this.isCloseDevice = false;
        env.log(env.level.INFO,'window','SmartWindow'," New SmartWindow INSTANCE. ");
        if(env.winSettings == void 0)
            throw new Error('Default windows settings is null.');
        _this.createWindow();
    }
    Object.defineProperty(SmartWindow.prototype, "win", {
        get: function () {
            return _this._win;
        },
        enumerable: true,
        configurable: true
    });
    SmartWindow.prototype.createWindow = function () {
        var options = env.winSettings;
        // if (env.isMac)
        //     options.height -= 30;
        // Create the browser window.
        _this._win = new BrowserWindow(options);
        _this.registerListeners();
    };
    SmartWindow.prototype.registerListeners = function () {

        _this._win.on('close', function (event){ 
            try
            {    if (!global.CanQuit){  
                      _this._win.hide();
                    if (env.isMac)
                        app.dock.hide();
                    global.CanQuit = true;
                    event.preventDefault();
                }else{
                       if (!_this.isCloseDevice && global.GeniusProtocol !== undefined){
                        global.GeniusProtocol.CloseAllDevice(function(){
                            setTimeout(function() 
                            {
                                _this.isCloseDevice = true;
                                global.CanQuit = true;
                                
                                app.quit();

                            }, 0);
                        });
                        global.TrayIcon.destroy();
                        global.TrayIcon = null;
                        event.preventDefault();
                    }
                }
            }catch(e){
                 env.log(env.level.ERROR,'window','registerListeners'," registerListeners close "+e+" .");
             }    
        });
     
        // Handle code that wants to open links
        _this._win.webContents.on('new-window', function (event, url) {
           // event.preventDefault();
           // Shell.openExternal(url);
        });
        //did-finish-load
        _this._win.webContents.on('did-finish-load', function (){
            _this.emit('finish-load', _this.isCrashed);
            _this.isCrashed = false;
        });
        // Window Failed to load
        _this._win.webContents.on('did-fail-load', function (event, errorCode, errorDescription) {
      
        });
        //crashed
        _this._win.webContents.on('crashed', function (){
          //  env.log('[MSG]','[window]','[webContents.on(crashed)]','Renderer process SmartWindow is crashed.');
            _this._win.destroy();
            _this._win = null;
           // _this.isCrashed = true;
           // _this.createWindow();
           // _this.load();
        });
        //plugin-crashed
        _this._win.webContents.on('plugin-crashed', function (event, name, version){

        });
    };
    SmartWindow.prototype.load = function () {
        _this._win.loadURL(_this.getUrl());
        // 打开开发工具界面
        //_this._win.openDevTools();
    };
    SmartWindow.prototype.getUrl = function () {
        var url = 'file://' + __dirname.replace(/\\/g, '/') + '/index.html';
        return url;
    };
    SmartWindow.prototype.dispose = function () {
        _this._win = null; // Important to dereference the window object to allow for GC
    };
    return SmartWindow;
})(events.EventEmitter);
exports.SmartWindowClass = SmartWindow;
