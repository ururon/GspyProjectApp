var events = require('events');
var env = require('../generic/env');
var fs = require('fs');
var path = require('path');
var funcVar = require('../generic/FunctionVariates');
var evtType = require('../generic/EventVariates').EventTypes;
var errCode = require('../generic/ErrorCode').ErrorCode;
var system = require('./System/System');


'use strict';

var __extends = this.__extends || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    __.prototype = b.prototype;
    d.prototype = new __();
};

var SmartProtocol = (function (_super) {
    __extends(SmartProtocol, _super);
    var _this;
    function SmartProtocol() {
        try{
            _this = this;
            _super.call(this);
            env.log(env.level.INFO,'Interface','SmartProtocol'," New SmartProtocol INSTANCE. "); 
            _this.SystemApi = new system.System();
            _this.ForegroundAppPath = _this.SystemApi.GetForegroundApp();
            _this.SystemApi.on(evtType.SessionChanged, _this.OnSessionChange);
            _this.SystemApi.on(evtType.HotPlug, _this.OnHotPlug);
            _this.SystemApi.on(evtType.UIViewInited, _this.OnUIViewInited);            
            // _this.SupportMouse.on(evtType.UpdateFirmwareReConnectTimeout, _this.OnUpdateFirmwareReConnectTimeout);
            // _this.SupportMouse.on(evtType.ProtocolMessage, _this.OnProtocolMessage);
            // _this.SupportKeyboard.on(evtType.ProtocolMessage, _this.OnProtocolMessage);
            // _this.SupportKeyboard.on(evtType.KeyboardHelpNotification, _this.OnKeyboardHelpNotification);//add by Ricky               
        }catch(ex){ 
            env.log(env.level.ERROR,'Interface','SmartProtocol',`ex:${ex.message}`); 
        }
    }

    SmartProtocol.prototype.OnProtocolMessage = function (Obj) {
        _this.emit(evtType.ProtocolMessage, Obj);
    }

    //add by Ricky
    SmartProtocol.prototype.OnKeyboardHelpNotification = function (Obj) {
        _this.emit(evtType.KeyboardHelpNotification, Obj);
    }

    //运行函数
    SmartProtocol.prototype.RunFunction = function (Obj, callback) {
        try{  
            env.log(env.level.DEBUG,'Interface','RunFunction',JSON.stringify(Obj)); 
        	if(!_this.CheckParam(Obj)){
        		callback(errCode.ValidateError,'SmartProtocol.RunFunction');
        		return;
        	}
            if (Obj.Func == funcVar.Names.InitDevice){
                //_this.InitDevice(callback);
                return;
            }
            if (_this.UpdateManager !== undefined){
                if (Obj.Func == funcVar.Names.DownloadInstallPackage){
                    _this.UpdateManager.DownloadInstallPackage(Obj.Param.InstallPackDownloadURL, Obj.Param.InstallPackTempPath, callback);
                    return;
                }
                if (Obj.Func == funcVar.Names.DownloadInstallDriver){
                    _this.UpdateManager.DownloadInstallDriver(Obj.Param.DriverDownloadURL, callback);
                    return;
                }
                if (Obj.Func == funcVar.Names.UpdateDriver){
                    _this.UpdateManager.UpdateDriver(null, callback);
                    return;
                }
                if (Obj.Func == funcVar.Names.CheckUpdateFromNet){
                    _this.UpdateManager.CheckUpdateFromNet(false, callback);
                    return;
                }
            }
            if (Obj.Func == funcVar.Names.UpdateFirmwareBoot){
                _this.IsUpdatingFirmware = true;
            }
            if (Obj.Func == funcVar.Names.UpdateFirmware){
                _this.IsUpdateFirmwareRefreshDevice = true;
            }

        	switch(Obj.Type){
        		case funcVar.Types.System:
                    _this.SystemApi.RunFunction(Obj, callback);
        		break;
        		case funcVar.Types.Mouse:
        			_this.SupportMouse.RunFunction(Obj, callback);
        		break;
        		case funcVar.Types.Keyboard:
                    //console.log("funcVar.Types.Keyboard begin ................");
                    _this.SupportKeyboard.RunFunction(Obj, callback); //Test by Ricky
        		break;

        		default:
    	    		callback(errCode.FormatError,Obj.Type);
    	    		return;
        	}
        }catch(ex){ 
            env.log(env.level.ERROR,'Interface','RunFunction',` ex:${ex.message}`); 
        }
    };
    //检查参数正确性
    SmartProtocol.prototype.CheckParam = function (Obj) {
    	if (Obj === null || Obj === undefined || typeof Obj !== 'object')
	        return false;
	    if (!Obj.hasOwnProperty('Type'))
	    	return false;
	    if (Obj.Type === null || Obj.Type === undefined || typeof Obj.Type !== 'number')
	    	return false;
	    if (!Obj.hasOwnProperty('SN'))
	    	return false;
	    if (!Obj.hasOwnProperty('Func'))
	    	return false;
	    if (!Obj.hasOwnProperty('Param'))
	    	return false;
	    return true;
    };
    //初始化更新管理
    SmartProtocol.prototype.InitUpdateManager = function () {
         env.log(env.level.DEBUG,'Interface','InitUpdateManager',' begin.'); 
         if (_this.UpdateManager === undefined){
             _this.UpdateManager = new um.UpdateManagerClass();
             _this.UpdateManager.on(evtType.ProtocolMessage, _this.OnProtocolMessage);
             _this.UpdateManager.StartCheckUpdate();
         }
    };

    //初始化更新管理
    SmartProtocol.prototype.InitNotifyManager = function () { 
        env.log(env.level.DEBUG,'Interface','InitNotifyManager',' begin.'); 
        if (_this.NotifyManager === undefined){
            _this.NotifyManager = new notify.NotificationManagerClass();
            _this.NotifyManager.on(evtType.ProtocolMessage, _this.OnProtocolMessage);
            _this.NotifyManager.StartCheckMessage();
        }
    };

    //初始化设备列表
    SmartProtocol.prototype.InitDevice = function (callback) { 
        env.log(env.level.DEBUG,'Interface','InitDevice',' begin.');
        try{       
            _this.IsRefreshDevice = true;

            if(_this.SupportModels == undefined)
            {
                _this.ModelsdbClass.getTemplateDevice('','','',function(sm){          
                    if (sm === undefined)  
                        env.log(env.level.WARN,'Interface','InitDevice',' Read SupportModels file undefined.');
                    _this.SupportModels = sm;

                    Promise.all([_this.SupportMouse.InitDevice(_this.SupportModels)]).then(function() {
                        //mouse success
                        callback();
                        Promise.all([_this.SupportKeyboard.InitDevice(_this.SupportModels)]).then(function() {
                            //mouse success and keyboard success
                            _this.IsRefreshDevice = false;
                            callback();
                        }, function() {
                            //mouse success and keyboard fail
                            _this.IsRefreshDevice = false;
                            callback();
                        });   
                    }, function() {
                        //mouse fail
                        callback();
                        Promise.all([_this.SupportKeyboard.InitDevice(_this.SupportModels)]).then(function() {
                            //mouse fail and keyboard success
                            _this.IsRefreshDevice = false;
                            callback();
                        }, function() {
                            //mouse fail and keyboard fail
                            _this.IsRefreshDevice = false;
                            callback();
                        });   
                    });
                });
            }
            else
            {
                Promise.all([_this.SupportMouse.InitDevice(_this.SupportModels)]).then(function() {
                    callback();
                    Promise.all([_this.SupportKeyboard.InitDevice(_this.SupportModels)]).then(function() {
                        _this.IsRefreshDevice = false;
                        callback();
                    }, function() {
                        _this.IsRefreshDevice = false;
                        callback();
                    });   
                }, function() {
                    callback();
                    Promise.all([_this.SupportKeyboard.InitDevice(_this.SupportModels)]).then(function() {
                        _this.IsRefreshDevice = false;
                        callback();
                    }, function() {
                        _this.IsRefreshDevice = false;
                        callback();
                    });   
                });
            }

            _this.InitUpdateManager();
            _this.DownloadServerFWVersion();
            //_this.AutoUpload();
        }catch(ex){ 
            env.log(env.level.ERROR,'Interface','InitDevice',` ex:${ex.message}`); 
            _this.IsRefreshDevice = false;
            callback();
        }
    };

    //关闭所有设备
    SmartProtocol.prototype.CloseAllDevice = function (callback) {
        try{ 
            env.log(env.level.DEBUG,'Interface','CloseAllDevice',` Begin Close Device `); 
            Promise.all([_this.SupportMouse.CloseDevice()]).then(function() {
                callback();
            }, function() {
                env.log(env.level.ERROR,'Interface','CloseAllDevice',` Close Mouse fail `); 
                callback();
            });

            Promise.all([_this.SupportKeyboard.CloseDevice()]).then(function() {
                callback();
            }, function() {
                env.log(env.level.ERROR,'Interface','CloseAllDevice',` Close HidDevice fail `);  
                callback();
            });

        }catch(ex){
            env.log(env.level.ERROR,'Interface','CloseAllDevice',`ex:${ex.message}`);  
            callback();
        }
    };

    //UI initialize finish, to check device status
    SmartProtocol.prototype.OnUIViewInited = function () {
        env.log(env.level.DEBUG,'Interface','OnUIViewInited',`begin OnUIViewInited`);
        
        if(!_this.IsRefreshDevice)
        {
            var Obj={
                Type : funcVar.Types.System,
                SN : null,
                Func : evtType.RefreshDevice,
                Param : null
            };
            _this.emit(evtType.ProtocolMessage, Obj);
            env.log(env.level.INFO,'Interface','OnUIViewInited',`Send RefreshDevice event to UI`);
        }
    };

    
    //在关机，登出，登入时需要重新刷新HID设备
    SmartProtocol.prototype.OnSessionChange = function (changeType) {        
        env.log(env.level.DEBUG,'Interface','OnSessionChange',`Begin.`);
        try{
            if (env.isLessThenWin81){
                if(changeType === 0x2 || changeType === 0x4 || changeType === 0x6 || changeType === 0x8){
    				_this.CloseAllDevice();
    			}
                if (!_this.IsRefreshDevice){
                    clearTimeout(_this.RefreshDeviceWaitNextEventTimeoutId);
                    _this.RefreshAllGeniusDevice(3500);
                }
                env.log(env.level.INFO,'Interface','OnSessionChange',`Send RefreshDevice event to UI`);                
            }
        }catch(ex){
            env.log(env.level.ERROR,'Interface','OnSessionChange',`ex :${ex.message}.`);
            _this.IsRefreshDevice = false;
        }
    };

    //baseInfo.PlugType : 0，移除，1，插入；
    //baseInfo.DeviceType : 1：HID，2：USB，3：NXP,0: 错误
    SmartProtocol.prototype.OnHotPlug = function (baseInfo, extInfo)
    {   
        env.log(env.level.INFO,'interface','OnHotPlug','OnHotPlug')
        var USBData = {};
        USBData.VID = baseInfo.VID.toString(16); 
        USBData.PID = baseInfo.PID.toString(16);  
        try{
            var Obj={
                        Type : funcVar.Types.System,
                        SN : null,
                        Func : evtType.RefreshDevice,
                        Param : USBData
                    };
            _this.emit(evtType.ProtocolMessage, Obj);
            
            env.log(env.level.INFO,'interface','OnHotPlug',JSON.stringify(Obj))
            // if(baseInfo.PlugType === 1 && ((Number(baseInfo.VID) === 0x1fc9 && Number(baseInfo.PID) === 0x000f) || baseInfo.DeviceType === 3)){
            //     _this.SupportMouse.UpdateNXPFirmware(_this.UpdateFirmwareBootComplete);
            //     return;
            // }
        
            // if (baseInfo.DeviceType === 2 || baseInfo.DeviceType === 3)
            //     return;

            // var supModel = undefined;
            // for (var model of _this.SupportModels){
            //     if(Number(model.VID) === Number(baseInfo.VID) && Number(model.PID) === Number(baseInfo.PID)){          
            //         env.log(env.level.DEBUG,'Interface','OnHotPlug',`Find Genius Device : `+JSON.stringify(model));              
            //         supModel = model;
            //         break;
            //     }
            // }
            
            // if (supModel !== undefined && _this.IsUpdatingFirmware && baseInfo.PlugType === 0){
            //     return;
            // }

            // if (supModel !== undefined && !_this.IsRefreshDevice){
            //     if ((baseInfo.PlugType === 1 && (extInfo === undefined || extInfo.HidUsage == supModel.EnumLastInterface)) || baseInfo.PlugType === 0){
            //         clearTimeout(_this.RefreshDeviceWaitNextEventTimeoutId);
            //         _this.RefreshAllGeniusDevice(1500);
            //     }
            // }
        }catch(ex){ 
            env.log(env.level.ERROR,'Interface','OnHotPlug',`[ex:${ex.message}]`);
            _this.IsRefreshDevice = false;
        }
        
    };
    
 
    //支援的Mouse
    SmartProtocol.prototype.SupportMouse = undefined;
    //系统函数
    SmartProtocol.prototype.SystemApi = undefined;
    //支援机种
    SmartProtocol.prototype.SupportModels = undefined;
    //等待下次刷新的时间
    SmartProtocol.prototype.RefreshDeviceWaitNextEventTimeoutId = undefined;
    //是否拔插时正刷新设备列表
    SmartProtocol.prototype.IsRefreshDevice = false;
    //等待App切换时间
    SmartProtocol.prototype.AppChangeWaitNextEventTimeoutId = undefined;
    //是否App切换，正应用配置
    SmartProtocol.prototype.IsAppChangeSettingProfile = false;
    //当前最前程序路径
    SmartProtocol.prototype.ForegroundAppPath = undefined;
    //更新
    SmartProtocol.prototype.UpdateManager = undefined;
    //通知
    SmartProtocol.prototype.NotifyManager = undefined;

    //是否在更新分位
    SmartProtocol.prototype.IsUpdatingFirmware = false;
    //是否在更新分位后，刷新设备列表
    SmartProtocol.prototype.IsUpdateFirmwareRefreshDevice = false;

    //Support All Genius Hid Deivce
    SmartProtocol.prototype.SupportKeyboard = undefined;
    //Support All Genius Hid Deivce
    SmartProtocol.prototype.ModelsdbClass = undefined;
    //Support All Genius Hid Deivce
    SmartProtocol.prototype.Historydb = undefined;
    SmartProtocol.prototype.nedbObject = undefined;
    
    return SmartProtocol;
})(events.EventEmitter);

exports.SmartProtocolClass = SmartProtocol;
