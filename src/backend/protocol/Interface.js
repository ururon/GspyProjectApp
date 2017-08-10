var events = require('events');
var env = require('../generic/env');
var fs = require('fs');
var path = require('path');
var funcVar = require('../generic/FunctionVariates');
var evtType = require('../generic/EventVariates').EventTypes;
var errCode = require('../generic/ErrorCode').ErrorCode;
var system = require('./System/System');
var AppObj = require("../dbapi/AppDB");


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
            _this.SystemApi.on(evtType.AppChanged, _this.OnAppChange);  
            _this.AppDB = new AppObj.AppDB();     
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
                _this.InitDevice(callback);
                return;
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
    

    //初始化设备列表
    SmartProtocol.prototype.InitDevice = function (callback) { 
        env.log(env.level.DEBUG,'Interface','InitDevice',' begin.');
        try{       
            // _this.AppDB.insertDevice(function(doc){
            //     env.log('777','777','777',JSON.stringify(doc));
            // })
            if(_this.SupportDevice == undefined){
                _this.AppDB.getDevice2(function(data){
                    _this.SupportDevice = data;
                    env.log('777','777','777',JSON.stringify(_this.SupportDevice));
                })
            }
            else{

            }
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
        USBData.PlugType = baseInfo.PlugType;
        try{
            for(var device of _this.SupportDevice){
                if( Number(device.VID)== Number(baseInfo.VID) && Number(device.PID)== Number(baseInfo.PID))
                {
                    USBData.DeviceName = device.DeviceName;
                }
            }

            var Obj={
                        Type : funcVar.Types.System,
                        SN : null,
                        Func : evtType.RefreshDevice,
                        Param : USBData
                    };
            _this.emit(evtType.ProtocolMessage, Obj);

        }catch(ex){ 
            env.log(env.level.ERROR,'Interface','OnHotPlug',`[ex:${ex.message}]`);
            _this.IsRefreshDevice = false;
        }
        
    };

    //App切换时，应用Profile
    SmartProtocol.prototype.OnAppChange = function (aAppPath) {  
        try{
            if(_this.AppChangeWaitNextEventTimeoutId !== undefined){
                env.log(env.level.DEBUG,'Interface','OnAppChange',`appPath:${aAppPath}`); 
                clearTimeout(_this.AppChangeWaitNextEventTimeoutId);
                _this.AppChangeWaitNextEventTimeoutId = undefined;
            }
            var isGeniusAppForeground = (aAppPath.slice(aAppPath.length - 10).toLowerCase().slice(0, 6) === 'genius');     
            if (!isGeniusAppForeground && !_this.IsAppChangeSettingProfile  )
            { 
                if(aAppPath !='')
                {
                    try
                    {    
                         if(env.isWindows)
                         { 
                            var PathArray = aAppPath.split("\\"); var i = PathArray.length;                          
                            var iconArray = PathArray[i-1].split(".");var icon = iconArray[0]+".ico";
                            var o = {}; o.appName = PathArray[i-1]; o.appPath = aAppPath; o.iconName = icon;
                            _this.Historydb.addHistory(o,function(err)
                            {
                                if(!err)
                                {
                                    let t = {}; t.from = aAppPath; t.To = path.join(env.appRoot,'img','icon',icon); t.Size = 48;
                                    _this.SupportMouse.getApplicationIcon(t); 
                                    var Obj = {
                                        Type : funcVar.Types.Mouse,
                                        SN : '',
                                        Func : evtType.RefreshMacroShortCutKeyData,
                                        Param : ''
                                    };
                                    _this.emit(evtType.ProtocolMessage, Obj);
                                }else{
                                }                         
                            })              
                        }
                        else
                        {
                            var PathArray = aAppPath.split("/");  var i = PathArray.length;      
                            var iconArray = PathArray[i-1].split(".");var icon = iconArray[0];
                            var tmpAppPath;
                            var TempIndex = aAppPath.indexOf(".app");

                            if(TempIndex != 0)
                                tmpAppPath = aAppPath.substr(0, TempIndex+4);
                            else
                                tmpAppPath = aAppPath;

                            var o = {}; o.appName = PathArray[i-1]; o.appPath = tmpAppPath; o.iconName = icon.replace(/\s+/g,"")+".ico";
                            _this.Historydb.addHistory(o,function(err){
                                if(!err){ 
                                    var outPath = path.join(env.appRoot,'img','icon');
                                    tools.getMacAppIcns(tmpAppPath,outPath+"/",icon.replace(/\s+/g,""),function(icon){});
                                    var Obj = {
                                        Type : funcVar.Types.Mouse,
                                        SN : '',
                                        Func : evtType.RefreshMacroShortCutKeyData,
                                        Param : ''
                                    };
                                    _this.emit(evtType.ProtocolMessage, Obj);
                                }
                            }); 
                        }
                    }catch(e){
                    }
                }
                _this.AppChangeWaitNextEventTimeoutId = setTimeout(function(){
                    try{ 
                        // _this.IsAppChangeSettingProfile = true;
                        // _this.ForegroundAppPath = aAppPath;  
                        // Promise.all([_this.SupportMouse.AppChangeSetProfile(_this.ForegroundAppPath),_this.SupportKeyboard.AppChangeSetProfile(_this.ForegroundAppPath)]).then(function (SetResults) {
                        //     SetResults.reduce(function (isSet) {
                        //         if (isSet)
                        //         {
                        //             env.log(env.level.INFO,'Interface','OnAppChange',`SetProfile Complete : ${_this.ForegroundAppPath}`);  
                        //             return;
                        //         }
                        //     });
                        //     _this.IsAppChangeSettingProfile = false;
                        // }); 

                    }catch(ex){ 
                         env.log(env.level.ERROR,'Interface','OnAppChange',`ex:${ex.message}`); 
                        _this.IsAppChangeSettingProfile = false;
                    }
                }, 1500);
            }
        }catch(ex){
            env.log(env.level.ERROR,'Interface','OnAppChange',`ex1:${ex.message}`);  
            _this.IsAppChangeSettingProfile = false;
        }
    };
    
 
    //支援的Mouse
    SmartProtocol.prototype.SupportMouse = undefined;
    //系统函数
    SmartProtocol.prototype.SystemApi = undefined;
    //支援机种
    SmartProtocol.prototype.SupportDevice = undefined;
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
