var env = require('./backend/generic/env');
var funcVar = require('./backend/generic/functionVariates');
var evtVar = require('./backend/generic/EventVariates').EventTypes;
// 应用控制模块
const electron = require('electron');
const app = electron.app;
const {Menu,Tray,ipcMain,globalShortcut,ipcRenderer} = require('electron');

// 创建窗口模块
const BrowserWindow = electron.BrowserWindow;

// 创建窗口模块
//var BrowserWindow = require('browser-window'),

// 主窗口
var mainWindow;

// 初始化并准备创建主窗口
app.on('ready', function () {
    env.log(env.level.INFO,'Main','Main','Ready');
    var _initDev = false;  
    // 创建一个宽800px 高700px的窗口
    // mainWindow = new BrowserWindow({
    //     width: 800,
    //     height: 700,
    // });
    // global.EnvVariate = env;
    // 载入应用的inde.html
    // mainWindow.loadURL('file://' + __dirname.replace(/\\/g, '/') + '/index.html');
    // env.log('666','666','666','file://' + __dirname.replace(/\\/g, '/') + '/index.html');

    // var windows = require('./windows');
    // global.MainWindow = new windows.SmartWindowClass();
    // global.MainWindow.load();

    // 打开开发工具界面
    //mainWindow.openDevTools();


    // 窗口关闭时触发
    // mainWindow.on('closed', function(){
    //     env.log(env.level.INFO,'Main','Main','Close');
    //     // 想要取消窗口对象的引用， 如果你的应用支持多窗口，你需要将所有的窗口对象存储到一个数组中，然后在这里删除想对应的元素
    //     mainWindow = null            
    // });    
});


var windows = require('./windows');
var cp = require('child_process');
var fs = require('fs');
var ipc = require('electron').ipcRenderer;
var net = require('net');
var tools = require('./backend/generic/tools');
var protoInterface = require('./backend/protocol/Interface');


var isAppReady = false;
app.once('ready', function () { return isAppReady = true; });
function onAppReady(cb) {
    if (isAppReady) {
        cb();
    }
    else {
        app.once('ready', cb);
    }
}

var Smart = (function () {
    var _this;
    function Smart(opts) {
        env.log('New Smart INSTANCE.');        
        _this = this;

        if (opts.closeApp){
            app.terminate();
            return;
        }
        _this.registerListeners();
        _this.startRunningInstanceListener();

        global.EnvVariate = env;  
        global.tools = tools;
        global.CanQuit = true;   
        global.MainWindow = new windows.SmartWindowClass();

        global.MainWindow.on('finish-load', function (isCrashed) {   
            _this._initDev = false;         
            if(global.GeniusProtocol !== undefined){
                _this.initDevice();
            }else{
                global.GeniusProtocol = new protoInterface.SmartProtocolClass();
                _this.initDevice();
            }
            global.GeniusProtocol.on(evtVar.ProtocolMessage, function(obj){
                env.log(env.level.INFO,'main','GeniusProtocol',JSON.stringify(obj))
				global.MainWindow.win.webContents.send(evtVar.ProtocolMessage, obj);
			});
            if (isCrashed === null || isCrashed === undefined || !isCrashed){
                if (!opts.noShow){
                    global.CanQuit = false;
                    global.MainWindow.win.show();
                }else{
                    if (env.isMac)
                        app.dock.hide();
                    global.CanQuit = true;
                }
            }
        }); 
        global.MainWindow.load();
        //
        if(global.GeniusProtocol === undefined)
            global.GeniusProtocol = new protoInterface.SmartProtocolClass();   
    }
    
    Smart.startup = function (opts) {
        if (!!Smart.INSTANCE) {
            throw new Error('Can only ever have one instance at the same time');
        }
        Smart.INSTANCE = new Smart(opts);
    };
    
    Smart.prototype.registerListeners = function () {
        app.on('will-quit', function () {
            env.log('App#will-quit: deleting running instance handle');
            env.DeleteLogFile();
            _this.deleteRunningInstanceHandle(); 
            globalShortcut.unregister('ctrl+i'); 
            globalShortcut.unregisterAll();              
        });
        app.on('activate-with-no-open-windows', function(){
            if (env.isMac && global.MainWindow !== undefined){
                app.dock.show();
                global.CanQuit = false;
                global.MainWindow.win.show();
            }
        }); 

        app.on('gpu-process-crashed', function(){
            env.log('App#gpu-process-crashed.');
        });
        //ipc
        ipcMain.on("app_langue_change", function (event, labs){ 
            _this.installTray(labs);        
        });
        ipcMain.on("NotifyWindow", function (event, iWidth, iHeight, sPath, timeOutClose){ 
            _this.popUpNotifyWindow(iWidth, iHeight, sPath, timeOutClose);        
        });
        ipcMain.on("NotifyWindowMouseEnter", function (event){
            _this.notifyWindowFocus = true;
            _this.NotifyWindowClearCloseTime();
        });
        ipcMain.on("NotifyWindowMouseLeave", function (event){
            _this.NotifyWindowSetCloseTime();
        });
        ipcMain.on("LinkAppUpdate", function (event){
           if (env.isMac)  app.dock.show();
           global.CanQuit = false;
           global.MainWindow.win.show();
           global.MainWindow.win.send("MainLinkAppUpdate");
        });

        ipcMain.on("testclearIntervalAlarmWindows", function (event,sPath,bSwitch,time)
        {
            if (_this.alarmnotifyWindow !== undefined && _this.alarmnotifyWindow !== null)
            {
                    _this.alarmnotifyWindow.close(); _this.alarmnotifyWindow = null;
            } 
            _this.popAlarmWindow(sPath);env.log("main Server test popAlarmWindow begin.");
        });

        ipcMain.on("clearIntervalAlarmWindows", function (event,sPath,bSwitch,time)
        {
            if (_this.alarmcloseNofifyWindowTimeId !== undefined)
            {
               clearTimeout(_this.alarmcloseNofifyWindowTimeId);
               clearInterval(_this.alarmcloseNofifyWindowTimeId);
            }

            if (_this.alarmnotifyWindow !== undefined && _this.alarmnotifyWindow !== null){
                _this.alarmnotifyWindow.close();
                _this.alarmnotifyWindow = null;
            } 

            if (bSwitch)
            {
                 time = !isNaN(parseFloat(time)) && isFinite(time) ? time: 50;
                _this.alarmcloseNofifyWindowTimeId = setInterval(function()
                {
                    _this.popAlarmWindow(sPath); env.log("main Server popAlarmWindow begin.");
                },time*60000) ; 
            }
            else
            {
                 clearTimeout(_this.alarmcloseNofifyWindowTimeId);clearInterval(_this.alarmcloseNofifyWindowTimeId);
                 if (_this.alarmnotifyWindow !== undefined && _this.alarmnotifyWindow !== null)
                 {
                    _this.alarmnotifyWindow.close(); _this.alarmnotifyWindow = null;
                 } 
            }
        });

        
        ipcMain.on("setAlarmDisturb",function(event)
        { 
            if (_this.alarmcloseNofifyWindowTimeId !== undefined)
            {
               clearTimeout(_this.alarmcloseNofifyWindowTimeId);
               clearInterval(_this.alarmcloseNofifyWindowTimeId);
            }

            if (_this.alarmnotifyWindow !== undefined && _this.alarmnotifyWindow !== null){
                _this.alarmnotifyWindow.close();
                _this.alarmnotifyWindow = null;
            } 
 
           if (env.isMac)  
                app.dock.show();
           global.CanQuit = false;
           global.MainWindow.win.show();
           global.MainWindow.win.send("MainsetAlarmDisturb");
        });

        ipcMain.on("LinkBattyPage", function (event){
           if (env.isMac)  
                app.dock.show();
           global.CanQuit = false;
           global.MainWindow.win.show();
           global.MainWindow.win.send("MainLinkBattyPage");
        }); 

        ipcMain.on("LinkSettingPage", function (event){
           if (env.isMac)  
                app.dock.show();
           global.CanQuit = false;
           global.MainWindow.win.show();
           global.MainWindow.win.send("MainLinkSettingPage");
        }); 


        ipcMain.on("NotifyAlarmWindow", function (event,sPath){ 
            console.log("NotifyAlarmWindow");
            _this.popAlarmWindow(sPath);        
        });
    };
    
    Smart.prototype.NotifyWindowClearCloseTime = function () {
        if (_this.closeNofifyWindowTimeId !== undefined){
            clearTimeout(_this.closeNofifyWindowTimeId);
            _this.closeNofifyWindowTimeId = undefined;
        }
    };
    Smart.prototype.alarmNotifyWindowClearCloseTime = function () {
        if (_this.alarmcloseNofifyWindowTimeId !== undefined){
            clearTimeout(_this.alarmcloseNofifyWindowTimeId);
            _this.alarmcloseNofifyWindowTimeId = undefined;
        }
    };

    Smart.prototype.NotifyWindowSetCloseTime = function () {
        if (_this.closeNofifyWindowTimeout > 0 && _this.notifyWindow !== null && _this.notifyWindow !== undefined){
            _this.closeNofifyWindowTimeId = setTimeout(function() {
                _this.notifyWindow.close();
                _this.closeNofifyWindowTimeId = undefined;
            }, _this.closeNofifyWindowTimeout);
        }
    };
    
    Smart.prototype.initDevice = function () {
        if(!_this._initDev){
            _this._initDev = true;
            var Obj={
                Type:funcVar.Types.System,
                SN:null,
                Func:funcVar.Names.InitDevice,
                Param:null
            };
            global.GeniusProtocol.RunFunction(Obj, function (error,data){
                try
                {
                    global.MainWindow.win.webContents.send('InitDevice');
                    env.log('InitDevice callback',error,data);
                }catch(ex){
                    env.log(`GeniusProtocol.InitDevice Error : ${ex}`);
                }
            });
        }
    };
 

    Smart.prototype.popAlarmWindow = function(sPath)
    {
        try{   
            var disp = require('screen').getPrimaryDisplay();
            var size = disp.bounds;
            var pointX = size.width ; console.log("pointX:"+pointX);
            var pointY = 0 ;   console.log("pointY:"+size.height +"    "+disp.workAreaSize.height); 

            if (_this.alarmnotifyWindow !== undefined && _this.alarmnotifyWindow !== null){
                _this.alarmnotifyWindow.close();
                _this.alarmnotifyWindow = null;
            }  

            _this.alarmnotifyWindow = new BrowserWindow({
                                                  width: size.width, 
                                                  height: size.height, 
                                                  show: false ,
                                                  x: 0, 
                                                  y: 0, 
                                                  frame: false,
                                                  resizable: false,
                                                  transparent: true,
                                                  "always-on-top": true,
                                                  "skip-taskbar": true 
                                              }); 

            _this.alarmnotifyWindow.on('close', function(){  
                _this.alarmnotifyWindow = null;
            });  

            _this.alarmnotifyWindow.loadUrl('file://' + sPath);
             
            _this.alarmnotifyWindow.show();
           
            
        }catch(ex){
            env.log(`Smart#popAlarmWindow Error : ${ex.message}.`);
        }
    };

    Smart.prototype.popUpNotifyWindow = function(iWidth, iHeight, sPath, timeOutClose)
    {
        try{
            if (iWidth === null || iWidth === undefined || iWidth < 1)
                iWidth = 300;
            if (iHeight === null || iHeight === undefined || iHeight < 1)
                iHeight = 100;
            if (sPath === null || sPath === undefined || !fs.existsSync(sPath)){
                env.log(`notifyWindow not found path : ${sPath}.`);
                return;
            }
            _this.notifyWindowFocus = false;
            if (timeOutClose === null || timeOutClose === undefined || timeOutClose < 0)
                timeOutClose = 0;
            var disp = require('screen').getPrimaryDisplay();
            var size = disp.bounds;
            var pointX = size.width - iWidth;

            var pointY = 0 ;
            if (env.isMac){
                pointY = size.y + 20; 
            }else{
                pointY = disp.workAreaSize.height - iHeight - 20 ;  
            }

            if (_this.notifyWindow !== undefined && _this.notifyWindow !== null){
                _this.notifyWindow.close();
                _this.notifyWindow = null;
            }
            if (_this.closeNofifyWindowTimeId !== undefined){
                clearTimeout(_this.closeNofifyWindowTimeId);
                _this.closeNofifyWindowTimeId = undefined;
            }
            _this.notifyWindow = new BrowserWindow({
                                                  width: iWidth, 
                                                  height: iHeight, 
                                                  show: false ,
                                                  x: size.width, 
                                                  y: pointY, 
                                                  frame: false,
                                                  resizable: false,
                                                  transparent: true,
                                                  "always-on-top": true,
                                                  "skip-taskbar": true,
                                                  "type": "notification"
                                              });        
            _this.notifyWindow.on('close', function(){
                _this.NotifyWindowClearCloseTime();
                _this.notifyWindow = null;
            });
            _this.notifyWindow.on('focus', function(){
                _this.notifyWindowFocus = true;
                _this.NotifyWindowClearCloseTime();
            });
            _this.notifyWindow.on('blur', function(){
                _this.NotifyWindowSetCloseTime();
            });
            _this.closeNofifyWindowTimeout = timeOutClose;
            _this.notifyWindow.webContents.on('did-finish-load', function (){
                var moveIntervalId = setInterval(function(){
                    var nowPoint = _this.notifyWindow.getPosition();
                    if (nowPoint[0] <= pointX){
                        clearInterval(moveIntervalId);
                        if (!_this.notifyWindowFocus)
                            _this.NotifyWindowSetCloseTime();
                        return;
                    }
                    _this.notifyWindow.setPosition(nowPoint[0] - 10, pointY);
                },50); 
            });
            _this.notifyWindow.loadUrl('file://' + sPath);
            _this.notifyWindow.show();
        }catch(ex){
            env.log(`Smart#popUpNotifyWindow Error : ${ex.message}.`);
        }
    };
    
    Smart.prototype.installTray = function (labs) { 
        if (labs === null || labs === undefined || labs.constructor !== Array || labs.length !== 2)
            labs = ['Open','Exit'];
        if (global.TrayIcon !== undefined){
            global.TrayIcon.destroy();
            global.TrayIcon = null;
        }
        global.TrayIcon = new Tray(path.join(env.appRoot,'App','logo.png')); 
        var TrayMenu = Menu.buildFromTemplate(
            [
                { 
                    label: labs[0],
                    icon:path.join(env.appRoot,'App','img','topmost.png'),
                    click:function(){
                        if (env.isMac)
                            app.dock.show();
                        global.CanQuit = false;
                        global.MainWindow.win.show();
                    }
                },
                {
                    label: labs[1],
                    icon:path.join(env.appRoot,'App','img','exit.png'),
                    click:function(){
                        global.CanQuit = true;
                        app.quit();
                    }
                } 
            ]
        );        
        global.TrayIcon.setContextMenu(TrayMenu);
        global.TrayIcon.on('clicked', function(){
            if (env.isMac)
                app.dock.show();
            global.CanQuit = false;
            global.MainWindow.win.show() ;
        });
    };
    
    Smart.prototype.startRunningInstanceListener = function () {
        // We are the one to own this
        _this.deleteRunningInstanceHandle();
        // Listen to other instances talking to us
        var runningInstance = net.createServer(function (connection) {
            connection.on('data', function (data) {
                try{
                    var otherInstanceArgs = data.toString().toLowerCase();
                    env.log('Received data from other instance', otherInstanceArgs);
                    if(otherInstanceArgs.startsWith('reopen')){
                        global.CanQuit = false;
                        global.MainWindow.win.show();
                        if (env.isMac)
                            app.dock.show();
                    }else if(otherInstanceArgs.startsWith('closeapp')){
                        setTimeout(function () {
			                global.CanQuit = true;
			                return app.terminate(); 
			            }, 0); 
                    }
                }catch(ex){
                    env.log(`Smart.connection#data : ${ex.message}`);
                }
            });
        });
        runningInstance.listen(env.runningInstanceHandle);
        // This can happen when multiple apps fight over the same connection
        runningInstance.on('error', function (error) {
            env.log('Terminating because running instance listener failed:', error.toString());
            setTimeout(function () {
                global.CanQuit = true;
                return app.terminate(); 
            }, 0); // we cannot allow multiple apps, so we quit
        });
    };
    
    Smart.prototype.deleteRunningInstanceHandle = function () {
        if (env.isWindows) {
            return;
        }
        if (fs.existsSync(env.runningInstanceHandle)) {
            try {
                return fs.unlinkSync(env.runningInstanceHandle);
            }
            catch (e) {
                if (e.code !== 'ENOENT') {
                    env.log('Fatal error deleting running instance handle', e);
                    throw e;
                }
            }
        }
    };
    
    Smart.prototype._initDev = false;
    return Smart;
})();

function main() {

    app.commandLine.appendSwitch('--disable-gpu'); 
    app.commandLine.appendSwitch(' --enable-logging');

    env.log('SmartGenius App main');

    // Ready for creating browser windows
    onAppReady(function () 
    {
        var args = Array.prototype.slice.call(process.argv, 1);
        var opts = args.filter(function (a) { return /^-/.test(a); })
                        .map(function (a) { return a.replace(/^-*/, ''); })
                        .reduce(function (r, a) {  r[a] = true;  return r; }, {});
        // On Mac/Unix we can rely on the server handle to be deleted if the instance is not running
        if (!env.isWindows && !fs.existsSync(env.runningInstanceHandle)) {
            env.log('Mac/Linux: starting smart because handle does not exist');
            Smart.startup(opts);
        }
        else { 
                    env.log(" runningInstanceHandle: "+env.runningInstanceHandle);
                    env.log(JSON.stringify(opts));
                    var con = net.connect({
                        path: env.runningInstanceHandle
                    }, function () {
                        // Another instance is running, we talk to it and ask it to open a window
                        var msg = opts.closeApp ? 'closeApp' : 'ReOpen';
                        con.write(msg, function () {
                            con.end();
                            env.log('Sending args to running instance and terminating');
                            setTimeout(function () {
                                global.CanQuit = true;
                                return app.terminate(); 
                            }, 0); // terminate later to prevent bad things from happen
                        });
                    });
                    con.on('error', function (error) {
                        env.log('No instance running,Starting smart.'+error);
                        Smart.startup(opts);
                    }); 
            }

         var ret = globalShortcut.register('ctrl+i', function() 
         {
              global.MainWindow.win.toggleDevTools();
          });
   
    });
}
// On Darwin, the PATH environment will not be correctly set when double clicking
// the application. Need to fix it up before anything starts up.
function fixUnixEnvironment(cb) {
    var didReturn = false;
    var done = function () {
        if (didReturn) {
            return;
        }
        didReturn = true;
        cb();
    };
    var child = cp.spawn(process.env.SHELL, ['-ilc', 'env'], {
        detached: true,
        stdio: ['ignore', 'pipe', process.stderr],
    });
    child.stdout.setEncoding('utf8');
    child.on('error', done);
    var buffer = '';
    child.stdout.on('data', function (d) {
        buffer += d;
    });
    child.on('close', function (code, signal) {
        if (code !== 0) {
            return done();
        }
        var hash = Object.create(null);
        buffer.split('\n').forEach(function (line) {
            var p = line.split('=', 2);
            var key = p[0];
            var value = p[1];
            if (!key || hash[key]) {
                return;
            }
            hash[key] = true;
            process.env[key] = value;
        });
        done();
    });
}
if (env.isMac || env.isLinux) {
    fixUnixEnvironment(main);
}
else {
    main();
}