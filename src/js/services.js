///<reference path='../../declare/angular.d.ts''/>
///<reference path='../../declare/jquery.d.ts'/>

'use strict';

 var remote = require('electron').remote;
 var env = remote.getGlobal('EnvVariate');
 var path = require('path');
 var fs = require('fs');
 //var nodemailer = require('nodemailer');
 var GeniusProtocol = remote.getGlobal('GeniusProtocol');
 var funcVar =require('./backend/generic/FunctionVariates');
 var evtVar =require('./backend/generic/EventVariates').EventTypes;
 //var ipc = require("ipc");
 var ipc = require('electron').ipcRenderer
 var os = require('os');
 var json = require('./backend/generic/JSON'); 
 var tools = remote.getGlobal('tools'); 
 var USBData = null;
 ipc.on(evtVar.ProtocolMessage,function(event,obj)
 {
   if (obj.Func == evtVar.RefreshDevice)              { env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on RefreshDevice begin..."); $("#txtAmmoxInstallDriverStatus").val(""); $('#RefreshDevice').click();}
   else if (obj.Func == evtVar.DPIChanged)            { env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on DPIChanged begin..."); $("#txtDPIChange").val(obj.SN+","+obj.Param.DPI+","+obj.Param.DpiValue+","+obj.Param.ProfileId);  $("#btnDPIChange").click();  }
   else if (obj.Func == evtVar.AppChanged)            
   {           
        env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on AppChanged begin..."+obj.Type);  
        if(obj.Type==2)
        {
             $("#txtAppChange").val( obj.SN + ","+ obj.Param);  $("#btnAppChange").click();
        }
        if(obj.Type==3)
        {
          var devicesId = $("#txtdeviceId").val();
          if((typeof $("#txtkbToggleProfileValue").val() != "undefined") && (devicesId == obj.SN))
          {
            $("#txtkbAppChange").val( obj.SN + ","+ obj.Param); 
            $("#btnkbAppChange").click();
          }else{
            $("#txtKbDeviceInfe").val(obj.SN + ","+ obj.Param);
            $("#btnKbRenovateApp").click();
          }
        }        
    }
   else if (obj.Func == evtVar.NotFormatSn)           { env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on NotFormatSn begin..."); }
   else if (obj.Func == evtVar.DownloadProgress)      
   {
         var per =Math.floor(Number(obj.Param.Current)*100 / Number(obj.Param.Total)) ;
         env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on DownloadProgress begin..."+per+"%"); 
         $("#prgsoftWare").html(""+per+"%"); $("#prgsoftWare").css("width", per+"%");
         if(per>=100){  $("#prgsoftWare").html("0%"); $("#prgsoftWare").css("width", "0%"); }
         var elem = document.getElementById('setWidth'); 
         if(elem !=null)   {    if(per<100){ elem.style.width = (per+60) + 'px'; } else {  elem.style.width = '0px';  }     }
   }
   else if (obj.Func == evtVar.InstallPackUpdate)     
   {   
         env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on InstallPackUpdate begin..."); 
         setTimeout(function(){ $("#downloadUrl").val(obj.Param.InstallPackDownloadURL+","+obj.Param.InstallPackTempPath); $('#btnexeAppVer').click(); }, 5000);
   }
   else if (obj.Func == evtVar.StartWriteFirmware)    
   {
         env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on StartWriteFirmware begin...");  var firmwareStatu = $("txt_update_firmware_state").val();
         if (firmwareStatu !== "1" ){  $('#dialog_update_firmware').modal({ show: true, backdrop: 'static' }); } 
    } //中断更时二次更新
   else if (obj.Func == evtVar.UpdateFirmwareProgess) 
   {
         env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on UpdateFirmwareProgess begin..."); 
         var per =Math.floor(Number(obj.Param.Current)*100 / Number(obj.Param.Total)) ; $("#prgUpdate").html(""+per+"%");  
         $("#prgUpdate").css("width", per+"%");   if (per == 100){ $("txt_update_firmware_state").val("") ;}  
   }
   else if (obj.Func == evtVar.UpdateFirmwareComplete)
   {
         env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on UpdateFirmwareComplete begin...");  var txt_div_update_info = $("#txt_div_update_info").val();  
         $("#div_update_info").html(txt_div_update_info);
   }
   else if (obj.Func == evtVar.CheckUpdateComplete)   
   {
         env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on CheckUpdateComplete begin...");
          if (obj.Param ==="fw")  {  $("#btnRefreshVer").click();  } else  if (obj.Param ==="app") {  $('#btnAppVer').click();  } 
   }
   else if (obj.Func == evtVar.BatteryStateChanged)   
   {
         env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on BatteryStateChanged begin..."); $("#txtBatteryStateChanged").val(obj.SN+","+obj.Param) ;
          $("#btnBatteryStateChanged").click();
   }
   else if (obj.Func == evtVar.USBError)              
   {    
         env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on USBError begin..."); $("#dialog_update_firmware").modal("hide"); 
         $("#dialog_update_boot").modal("hide"); $("#dialog_profile_restore").modal("hide"); 
         var html = $("#errorMsg").text();$("#div_error_message").html(html);  
         $("#dialog_update_error").modal({ show: true, backdrop: 'static' }); 
   }
   else if (obj.Func == evtVar.ChargingStateChanged)  
   {
         env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on ChargingStateChanged begin..."); $("#txtNx9000BtChargingStateChanged").val(obj.SN+","+obj.Param) ;
         $("#btnNx9000BtChargingStateChanged").click(); 
   }
   else if (obj.Func == evtVar.ADCStateChanged)       
   { 
        env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on ADCStateChanged begin...");  $("#txtNx9000BtADCStateChanged").val(obj.SN+","+obj.Param) ;
        $("#btnNx9000BtADCStateChanged").click(); 
   }
   else if (obj.Func == evtVar.DeviceDriverUpdate)    
   {
         env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on DeviceDriverUpdate begin..."); 
         // alert('DeviceMustDriver');
         angular.element(document.querySelector('[ng-controller=DeviceCtrl]')).scope().promptDeviceUpdate();   
   } 
   else if (obj.Func == evtVar.NewNotification)       
  {
         env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on NewNotification begin..."); $("#txtNotificationState").val(obj.Param); 
         $("#btnNotificationState").click(); 
  }
  else if (obj.Func == evtVar.InstallDriver)         
  {
         env.log('[MSG]','[services]','[ProtocolMessage]',"ipc on InstallDriver begin..."); var txtAmmoxInstallDriverStatus = $("#txtAmmoxInstallDriverStatus").val();  
         if (txtAmmoxInstallDriverStatus != ""){ $("#dialog_InstallDriver_prompt").modal({ show: true, backdrop: 'static' });　 } 
  } 
  else if (obj.Func == evtVar.NetWorkOnline)         
  {
     env.log('[MSG]','[services]','[ProtocolMessage]',"NetWorkOnline .................."); 
     $("#btnCheckLocaltion").click();
  } 
  else if (obj.Func == evtVar.NetWorkOffline)         
  {
     //env.log('[MSG]','[services]','[ProtocolMessage]',"NetWorkOffline ....................");  
  } 

 }) 


 var stateinterval = setInterval(function()
{
     var txtstateChangeSuccess= $("#txtstateChangeSuccess").val();
     if (txtstateChangeSuccess == "")
    { 
        $("#ng-cloak-profiles").attr("style","display:block");
        $("#ng-cloak-ammox-profiles").attr("style","display:block");
        $("#ng-cloak-nx7000").attr("style","display:block");
        $("#ng-cloak-nx7005").attr("style","display:block");
        $("#ng-cloak-nx7006").attr("style","display:block");
        $("#ng-cloak-nx7010").attr("style","display:block");
        $("#ng-cloak-nx7015").attr("style","display:block");
        $("#ng-cloak-traver9000").attr("style","display:block");
        $("#ng-cloak-nx7000v3").attr("style","display:block");
        $("#ng-cloak-nx7010v3").attr("style","display:block");
        $("#ng-cloak-nx7015v3").attr("style","display:block");
        clearInterval(stateinterval);
    }
     else
      {
           $("#ng-cloak-profiles").attr("style","display:block");
           $("#ng-cloak-ammox-profiles").attr("style","display:block");
           $("#ng-cloak-nx7000").attr("style","display:block");
           $("#ng-cloak-nx7005").attr("style","display:block");
           $("#ng-cloak-nx7006").attr("style","display:block");
           $("#ng-cloak-nx7010").attr("style","display:block");
           $("#ng-cloak-nx7015").attr("style","display:block");
           $("#ng-cloak-traver9000").attr("style","display:block");
           $("#ng-cloak-nx7000v3").attr("style","display:block");
           $("#ng-cloak-nx7010v3").attr("style","display:block");
           $("#ng-cloak-nx7015v3").attr("style","display:block"); 
      }
},2500);
 

 angular.module('app.services', [ 'ngResource' ])
 .factory('HotPlugDetect', function($rootScope){
    var HotPlugDetect = {};
    var Data = {};
    var flag = 0;
    ipc.on(evtVar.ProtocolMessage,function(event,obj){
      if(obj.Func == evtVar.RefreshDevice){  
          USBData = obj.Param;
          HotPlugDetect.VID = USBData.VID;
          HotPlugDetect.PID = USBData.PID;
          $rootScope.$emit('RefreshDevice', HotPlugDetect);
          flag = 1;
      }
    });
    HotPlugDetect.temp1 = function(){
      env.log('bbb','bbb','bbb',JSON.stringify(Data));
      if(flag==1)
        return HotPlugDetect;
      else
      {
        var TempUSBData={};
        TempUSBData.VID = 0;
        TempUSBData.PID = 0;
        return TempUSBData;
      }
    }
    return HotPlugDetect;
 })
 .factory('mySharedService', function($rootScope)
 {
    var sharedService = {};
    sharedService.prepForBroadcast = function()      {  $rootScope.$broadcast('RefreshDevice'); };
    sharedService.RefreshScoorpionMouse = function() {  $rootScope.$broadcast('RefreshScoorpionMouse'); };
    sharedService.RefreshXGMouse = function() {  $rootScope.$broadcast('RefreshXGMouse'); };
    sharedService.RefreshAmmoxMouse = function()     {  $rootScope.$broadcast('RefreshAmmoxMouse'); };
    sharedService.RefreshTravelerMousebattery = function() {  $rootScope.$broadcast('RefreshTravelerMousebattery'); }
    sharedService.RefreshNx7000 = function(){ $rootScope.$broadcast('RefreshNx7000'); }
    sharedService.RefreshNx7005 = function(){ $rootScope.$broadcast('RefreshNx7005'); }
    sharedService.RefreshNx7006 = function(){ $rootScope.$broadcast('RefreshNx7006'); }
    sharedService.RefreshNx7010 = function(){ $rootScope.$broadcast('RefreshNx7010'); }
    sharedService.RefreshNx7015 = function(){ $rootScope.$broadcast('RefreshNx7015'); }
    sharedService.RefreshNx9000bt = function(){ $rootScope.$broadcast('RefreshNx9000bt'); }
    sharedService.RefreshNx9000btV2 = function(){ $rootScope.$broadcast('RefreshNx9000btV2'); }
    sharedService.RefreshTraveler9000 = function(){ $rootScope.$broadcast('RefreshTraveler9000'); }
    sharedService.RefreshTraveler900S = function(){ $rootScope.$broadcast('RefreshTraveler900S'); }
    sharedService.RefreshwireMouse = function(){ $rootScope.$broadcast('RefreshwireMouse'); }
    sharedService.RefreshwiremousebkMouse = function(){ $rootScope.$broadcast('RefreshwiremousebkMouse');  }
    sharedService.Refreshnx7000v3 = function(){ $rootScope.$broadcast('Refreshnx7000v3');  } 
    sharedService.Refreshnx7015v3 = function(){ $rootScope.$broadcast('Refreshnx7015v3');  }
    sharedService.Refresheco7000 = function(){ $rootScope.$broadcast('Refresheco7000');  }
    sharedService.Refresheco8000 = function(){ $rootScope.$broadcast('Refresheco8000');  } 
    sharedService.RefreshNx9000BtChargingStateChanged = function() {  $rootScope.$broadcast('RefreshNx9000BtChargingStateChanged'); }
    sharedService.RefreshADCStateChanged = function() {  $rootScope.$broadcast('RefreshADCStateChanged'); }
    sharedService.Refresheco7015 = function(){ $rootScope.$broadcast('Refresheco7015');  }  
    sharedService.RefreshKBJatSeries = function(){ $rootScope.$broadcast('RefreshKBJatSeries'); }
    sharedService.RefreshKBMainSeries = function(){ $rootScope.$broadcast('RefreshKBMainSeries'); }
    return sharedService;
 })

 .factory('geniusScopionDataSharedService', function()
{
    var menusharedService = {};
    this.menuData = {   "menuKey1Data": "左键点击",
                        "menuKey2Data": "右键菜单",
                        "menuKey3Data": "中键设置",
                        "menuKey4Data": "网页前进",
                        "menuKey5Data": "网页后退",
                        "menuKey6Data": "DPI" };
   this.keyData  = [
                      { "keyType":1 ,"keyAction":1  },
                      { "keyType":1 ,"keyAction":2  },
                      { "keyType":1 ,"keyAction":4  },
                      { "keyType":1 ,"keyAction":8  },
                      { "keyType":1 ,"keyAction":6  },
                      { "keyType":1 ,"keyAction":32 }
                   ] ;
   this.macroData = [
                       { "macroId":1,   "macroUse":"off", "macroName":"macro1",   "macroContent":[]},
                       { "macroId":2,   "macroUse":"off", "macroName":"macro2",   "macroContent":[]},
                       { "macroId":3,   "macroUse":"off", "macroName":"macro3",   "macroContent":[]},
                       { "macroId":4,   "macroUse":"off", "macroName":"macro4",   "macroContent":[]},
                       { "macroId":5,   "macroUse":"off", "macroName":"macro5",   "macroContent":[]},
                       { "macroId":6,   "macroUse":"off", "macroName":"macro6",   "macroContent":[]},
                       { "macroId":7,   "macroUse":"off", "macroName":"macro7",   "macroContent":[]},
                       { "macroId":8,   "macroUse":"off", "macroName":"macro8",   "macroContent":[]},
                       { "macroId":9,   "macroUse":"off", "macroName":"macro9",   "macroContent":[]},
                       { "macroId":10,  "macroUse":"off", "macroName":"macro10",  "macroContent":[]},
                       { "macroId":11,  "macroUse":"off", "macroName":"macro11",  "macroContent":[]},
                       { "macroId":12,  "macroUse":"off", "macroName":"macro12",  "macroContent":[]},
                       { "macroId":13,  "macroUse":"off", "macroName":"macro13",  "macroContent":[]},
                       { "macroId":14,  "macroUse":"off", "macroName":"macro14",  "macroContent":[]},
                       { "macroId":15,  "macroUse":"off", "macroName":"macro15",  "macroContent":[]},
                       { "macroId":16,  "macroUse":"off", "macroName":"macro16",  "macroContent":[]},
                       { "macroId":17,  "macroUse":"off", "macroName":"macro17",  "macroContent":[]},
                       { "macroId":18,  "macroUse":"off", "macroName":"macro18",  "macroContent":[]},
                       { "macroId":19,  "macroUse":"off", "macroName":"macro19",  "macroContent":[]},
                       { "macroId":20,  "macroUse":"off", "macroName":"macro20",  "macroContent":[]}, 
                       { "macroId":21,  "macroUse":"off", "macroName":"macro21",  "macroContent":[]},
                       { "macroId":22,  "macroUse":"off", "macroName":"macro22",  "macroContent":[]},
                       { "macroId":23,  "macroUse":"off", "macroName":"macro23",  "macroContent":[]},
                       { "macroId":24,  "macroUse":"off", "macroName":"macro24",  "macroContent":[]},
                       { "macroId":25,  "macroUse":"off", "macroName":"macro25",  "macroContent":[]},
                       { "macroId":26,  "macroUse":"off", "macroName":"macro26",  "macroContent":[]},
                       { "macroId":27,  "macroUse":"off", "macroName":"macro27",  "macroContent":[]},
                       { "macroId":28,  "macroUse":"off", "macroName":"macro28",  "macroContent":[]},
                       { "macroId":29,  "macroUse":"off", "macroName":"macro29",  "macroContent":[]},
                       { "macroId":30,  "macroUse":"off", "macroName":"macro30",  "macroContent":[]}, 
                       { "macroId":31,  "macroUse":"off", "macroName":"macro31",  "macroContent":[]},
                       { "macroId":32,  "macroUse":"off", "macroName":"macro32",  "macroContent":[]},
                       { "macroId":33,  "macroUse":"off", "macroName":"macro33",  "macroContent":[]},
                       { "macroId":34,  "macroUse":"off", "macroName":"macro34",  "macroContent":[]},
                       { "macroId":35,  "macroUse":"off", "macroName":"macro35",  "macroContent":[]},
                       { "macroId":36,  "macroUse":"off", "macroName":"macro36",  "macroContent":[]},
                       { "macroId":37,  "macroUse":"off", "macroName":"macro37",  "macroContent":[]},
                       { "macroId":38,  "macroUse":"off", "macroName":"macro38",  "macroContent":[]},
                       { "macroId":39,  "macroUse":"off", "macroName":"macro39",  "macroContent":[]},
                       { "macroId":40,  "macroUse":"off", "macroName":"macro40",  "macroContent":[]},
                       { "macroId":41,  "macroUse":"off", "macroName":"macro41",  "macroContent":[]},
                       { "macroId":42,  "macroUse":"off", "macroName":"macro42",  "macroContent":[]},
                       { "macroId":43,  "macroUse":"off", "macroName":"macro43",  "macroContent":[]},
                       { "macroId":44,  "macroUse":"off", "macroName":"macro44",  "macroContent":[]},
                       { "macroId":45,  "macroUse":"off", "macroName":"macro45",  "macroContent":[]},
                       { "macroId":46,  "macroUse":"off", "macroName":"macro46",  "macroContent":[]},
                       { "macroId":47,  "macroUse":"off", "macroName":"macro47",  "macroContent":[]},
                       { "macroId":48,  "macroUse":"off", "macroName":"macro48",  "macroContent":[]},
                       { "macroId":49,  "macroUse":"off", "macroName":"macro49",  "macroContent":[]},
                       { "macroId":50,  "macroUse":"off", "macroName":"macro50",  "macroContent":[]},
                       { "macroId":51,  "macroUse":"off", "macroName":"macro51",  "macroContent":[]},
                       { "macroId":52,  "macroUse":"off", "macroName":"macro52",  "macroContent":[]},
                       { "macroId":53,  "macroUse":"off", "macroName":"macro53",  "macroContent":[]},
                       { "macroId":54,  "macroUse":"off", "macroName":"macro54",  "macroContent":[]},
                       { "macroId":55,  "macroUse":"off", "macroName":"macro55",  "macroContent":[]},
                       { "macroId":56,  "macroUse":"off", "macroName":"macro56",  "macroContent":[]},
                       { "macroId":57,  "macroUse":"off", "macroName":"macro57",  "macroContent":[]},
                       { "macroId":58,  "macroUse":"off", "macroName":"macro58",  "macroContent":[]},
                       { "macroId":59,  "macroUse":"off", "macroName":"macro59",  "macroContent":[]},
                       { "macroId":60,  "macroUse":"off", "macroName":"macro60",  "macroContent":[]},
                       { "macroId":61,  "macroUse":"off", "macroName":"macro61",  "macroContent":[]},
                       { "macroId":62,  "macroUse":"off", "macroName":"macro62",  "macroContent":[]},
                       { "macroId":63,  "macroUse":"off", "macroName":"macro63",  "macroContent":[]},
                       { "macroId":64,  "macroUse":"off", "macroName":"macro64",  "macroContent":[]},
                       { "macroId":65,  "macroUse":"off", "macroName":"macro65",  "macroContent":[]},
                       { "macroId":66,  "macroUse":"off", "macroName":"macro66",  "macroContent":[]},
                       { "macroId":67,  "macroUse":"off", "macroName":"macro67",  "macroContent":[]},
                       { "macroId":68,  "macroUse":"off", "macroName":"macro68",  "macroContent":[]},
                       { "macroId":69,  "macroUse":"off", "macroName":"macro69",  "macroContent":[]},
                       { "macroId":70,  "macroUse":"off", "macroName":"macro70",  "macroContent":[]}
                  ] ;
   this.appMappingData = [] ;
   this.Pointer  = [
                      {"TYPE":"WIN","Data":[2,   4,    6,    8,   10,   12,   14,    16,    18,    20]},
                      {"TYPE":"MAC","Data":[0,8192,32768,45056,57344,65536,98304,131072,163840,196608]}
                   ];
   this.Double  =  [
                      {"TYPE":"WIN","Data":[900,830,760,690,620,550,480,410,340,270]},
                      {"TYPE":"MAC","Data":[5,3,2,1.8,1.7,1.4,0.8,   0.5, 0.2,  0.15]}
                   ];
   return {  menuKey1Data      : "左键点击",
             menuKey2Data      : "右键菜单",
             menuKey3Data      : "中键设置",
             menuKey4Data      : "网页前进",
             menuKey5Data      : "网页后退",
             menuKey6Data      : "切换DPI",
             osType            :  "",
             GradVal           : 2 ,
             bootVersion       : "",
             fwVersion         : "",
             configbootVersion : "",
             configfwVersion   : "",
             menuData          : this.menuData,
             macroData         : this.macroData,
             PointerData       : this.Pointer  ,
             appMappData       : this.appMappingData,
             DoubleData        : this.Double ,
             macroNumber       : 0,
             xDpi              : 800 ,
             lButNum           : 1,
             PointerSpeed      : 6,
             wheelSpeed        : 6,
             DoubleSpeed       : 6,
             ProfileImageIndex : 1,
             dpiCount          : 0,
             selectMacroNo     : 0,
             keyAction         : 0,
             PCBAType         : 1,
             fwDir            : "",
             fwVer            : "",
             butCountOn       : 0
          };

})

 .factory('geniusKbScopionDataSharedService', function()
{
    var menusharedService = {};
    
   this.appMappingData = [] ;
   this.Pointer  = [
                      {"TYPE":"WIN","Data":[2,   4,    6,    8,   10,   12,   14,    16,    18,    20]},
                      {"TYPE":"MAC","Data":[0,8192,32768,45056,57344,65536,98304,131072,163840,196608]}
                   ];
   this.Double  =  [
                      {"TYPE":"WIN","Data":[900,830,760,690,620,550,480,410,340,270]},
                      {"TYPE":"MAC","Data":[5,3,2,1.8,1.7,1.4,0.8,   0.5, 0.2,  0.15]}
                   ];
   return {  
             osType            :  "",
             PointerData       : this.Pointer  ,
             appMappData       : this.appMappingData,
             DoubleData        : this.Double ,
             ProfileImageIndex : 1
          };

})

.factory('geniusKeyboardService',function($rootScope,$translate,geniusSystemService)
{
    var geniusKyeboardShareService = {};
    geniusKyeboardShareService.SetKeyboardKey = function(Sn,FkeyId,iKeyGroup,iFunction,Param1,Param2,id)
    {                
        // console.log("test*******************");
        env.log('[MSG]','[services]','[SetKeyboardKey]','FkeyId:'+FkeyId +" iKeyGroup:"+iKeyGroup+" iFunction:"+iFunction+" Param1:"+Param1+" Param2:"+Param2+" id:"+id);
         var Obj= { 
                       Type:funcVar.Types.Keyboard, 
                       SN:Sn,Func:funcVar.Names.SetKeyBoardKeyValue,
                       Param:{ProfileID:parseInt(id,10),keyId:parseInt(FkeyId,10),GroupID:parseInt(iKeyGroup,10),FuncTypeID:parseInt(iFunction,10),Param1:Param1,Param2:Param2 } 
                  };
         GeniusProtocol.RunFunction(Obj, function (error,data){ env.log('[ERR]','[services]','[SetKeyboardKey]',error); });
    } 

    geniusKyeboardShareService.SetKeyboardKeyByDevicesId = function(Sn){
      var profileJson = geniusSystemService.getDeviceProfiles(Sn);
      var keyBoardTemplateList = geniusSystemService.getKeyboardDevicesTemplateList(Sn);
      var groupId  = profileJson[0].keyMapping[0].GroupID;
      var funcId   = profileJson[0].keyMapping[0].FuncTypeID;
      var funcName = profileJson[0].keyMapping[0].FuncTypeName;
      var param1   = profileJson[0].keyMapping[0].Param1;
      var param2   = profileJson[0].keyMapping[0].Param2;
      geniusSystemService.cleanKbProfileItemIsNull(profileJson,keyBoardTemplateList,function(profileJson,keyBoardTemplateList){
        geniusSystemService.saveKeyboardDevicesJsonFile(Sn,0,profileJson);
        geniusKyeboardShareService.SetKeyboardKey(Sn,0,groupId,funcId,param1,"",0);
        // env.log();
      });
    }

    geniusKyeboardShareService.ModifyMacroNotice = function(mskbType,Sn,macroData)
    {
       env.log("ModifyMacroNotice begin..............."); 
       if (mskbType =='mouse')
       {
          var Obj = {
                       Type:funcVar.Types.Mouse, 
                       SN:Sn,
                       Func:funcVar.Names.MacroModifyNotice,
                       Param:macroData
                    }
          GeniusProtocol.RunFunction(Obj, function (error,data){ env.log('[ERR]','[services]','[ModifyMacroNotice]',error);});         
        } if (mskbType =='keyboard')
        {
          try{
            geniusKyeboardShareService.SetKeyboardKeyByDevicesId(Sn);
          }catch(e){
            var errorStr = "[KB MacroNotice1 error : "+e+"]";
            env.log('[ERR]','[services]','[ModifyMacroNotice]',"[KB MacroNotice error :"+errorStr+"]");
          }
        }   
    }


    geniusKyeboardShareService.DeleteMacroNotice = function(mskbType,Sn)
    { 
       env.log("DeleteMacroNotice begin...............");
       if (mskbType =='mouse')
       {
          var Obj = {
                       Type:funcVar.Types.Mouse, 
                       SN:Sn,
                       Func:funcVar.Names.MacroDeleteNotice,
                       Param:null
                    }
          GeniusProtocol.RunFunction(Obj, function (error,data){ env.log('[ERR]','[services]','[DeleteMacroNotice]',error);});         
        } if (mskbType =='keyboard')
        {
          try{
            geniusKyeboardShareService.SetKeyboardKeyByDevicesId(Sn);
          }catch(e){
            var errorStr = "[KB MacroNotice1 error : "+e+"]";
            env.log('[ERR]','[services]','[DeleteMacroNotice]',"[KB DeleteMacroNotice error :"+errorStr+"]");
          }
        }   
    }


    return geniusKyeboardShareService;
})
.factory('geniusScorpionMouseService',function($rootScope,$translate)
 {
      var geniusScoropMouseShareService = {};

      geniusScoropMouseShareService.SetDeviceButtonKey = function(vid,pid,sn,buttonNum,setType,setVal)
      {
           var Obj= {
                         Type:funcVar.Types.Mouse, 
                         SN:sn,Func:funcVar.Names.SetDeviceButtonKey, 
                         Param:{ buttonNum : parseInt(buttonNum,10), setType : parseInt(setType,10), setVal : setVal } 
                    };
           GeniusProtocol.RunFunction(Obj, function (error,data){ env.log('[ERR]','[services]','[SetDeviceButtonKey]',error);  });
      };

      geniusScoropMouseShareService.SetDeviceButtonKeyCostdown = function(vid,pid,sn,buttonNum,setType,setVal,macroMode,macroRepeat,macroid,isBool)
      {
           env.log("buttonNum:"+buttonNum+" setType:"+setType+" setVal:"+setVal+" macroMode:"+macroMode+" macroRepeat:"+macroRepeat+" macroid:"+macroid+" isBool:"+isBool);
           var Obj= {
                         Type:funcVar.Types.Mouse, 
                         SN:sn,Func:funcVar.Names.SetDeviceButtonKey, 
                         Param:{ buttonNum:parseInt(buttonNum,10),setType:parseInt(setType,10),setVal:setVal,Mode:macroMode,Repeat:macroRepeat,macroid:parseInt(macroid,10)}
                   };
            GeniusProtocol.RunFunction(Obj, function (error,data)
           {  
               env.log(" SetDeviceButtonKeyCostdown   error:"+error+" data:"+data);
              if (error == undefined)
             // if ( 1 == 1 )
              {   
                 if((parseInt(setType,10) ==4) && (isBool))
                 {
                    setTimeout(function()
                    { 
                        var Obj1={
                                     Type:funcVar.Types.Mouse, 
                                     SN:sn,Func:funcVar.Names.SetDeviceButtonKey, 
                                     Param:{ buttonNum:parseInt(buttonNum,10),setType:parseInt(16,10),setVal:setVal,Mode:macroMode,Repeat:macroRepeat,macroid:parseInt(macroid,10) } 
                                };
                        GeniusProtocol.RunFunction(Obj1, function (error1,data)
                        { 
                             env.log('[ERR]','[services]','[SetDeviceButtonKeyCostdown]',error1); 
                        });
                    }, 50);
                 }

              }
              else
              { 
                  env.log('[ERR]','[services]','[SetDeviceButtonKeyCostdown]',error); 
              }
          });
      };

      geniusScoropMouseShareService.SetDeviceWheelSpeed = function(vid,pid,sn,speed)
      {
          var Obj = {  
                      Type:funcVar.Types.Mouse,  
                      SN:sn,  
                      Func:funcVar.Names.SetDeviceWheelSpeed, 
                      Param :  parseInt(speed,10)  
                    }
          GeniusProtocol.RunFunction(Obj, function (error,data){   env.log('[ERR]','[services]','[SetDeviceWheelSpeed]',error);  });
      };
      geniusScoropMouseShareService.SetDeviceGradToggle = function(vid,pid,sn,gradVal)
      {
          var Obj = {  
                       Type:funcVar.Types.Mouse, 
                       SN:sn, 
                       Func:funcVar.Names.SetDeviceGradToggle, 
                       Param :  parseInt(gradVal,10)  
                   }
          GeniusProtocol.RunFunction(Obj, function (error,data){  env.log('[ERR]','[services]','[SetDeviceGradToggle]',error); });
      };
      geniusScoropMouseShareService.SetMousePointSpeed = function(vid,pid,sn,speed)
      {
            var Obj = {  
                          Type:funcVar.Types.Mouse,   
                          SN:sn,  
                          Func:funcVar.Names.SetMousePointSpeed, 
                          Param :  parseInt(speed,10)  
                      }
            GeniusProtocol.RunFunction(Obj, function (error,data){   env.log('[ERR]','[services]','[SetMousePointSpeed]',error);  });
      };
      geniusScoropMouseShareService.SetMouseDoubleClickSpeed = function(vid,pid,sn,speed)
      {
         var Obj = {  
                        Type:funcVar.Types.Mouse,  
                        SN:sn,  
                        Func:funcVar.Names.SetMouseDoubleClickSpeed,   
                        Param : speed          
                   }
         GeniusProtocol.RunFunction(Obj, function (error,data){   env.log('[ERR]','[services]','[SetMouseDoubleClickSpeed]',error);  });
      }; 　
      geniusScoropMouseShareService.SetDeviceWheelLedShowMode = function(vid,pid,sn,mode)
      {
         var Obj = {  
                      Type:funcVar.Types.Mouse,  
                      SN:sn,  
                      Func:funcVar.Names.SetDeviceWheelLedShowMode,   
                      Param : mode          
                   }
         GeniusProtocol.RunFunction(Obj, function (error,data){  env.log('[ERR]','[services]','[SetDeviceWheelLedShowMode]',error);  });
      };
      geniusScoropMouseShareService.SetDeviceLogoLedShowMode  = function(vid,pid,sn,mode)
      {
          var Obj = { 
                        Type:funcVar.Types.Mouse, 
                        SN:sn, 
                        Func:funcVar.Names.SetDeviceLogoLedShowMode,  
                        Param : mode  
                    }
          GeniusProtocol.RunFunction(Obj, function (error,data){  env.log('[ERR]','[services]','[SetDeviceLogoLedShowMode]',error);  });
      };
      geniusScoropMouseShareService.SetDeviceLogoLedBreathRate  = function(vid,pid,sn,breathRate)
      {
          var Obj = { 
                        Type:funcVar.Types.Mouse,  
                        SN:sn,
                        Func:funcVar.Names.SetDeviceLogoLedBreathRate,  
                        Param : parseInt(breathRate,10)  
                    }
          GeniusProtocol.RunFunction(Obj, function (error,data){  env.log('[ERR]','[services]','[SetDeviceLogoLedBreathRate]',error); });
      };
      geniusScoropMouseShareService.SetDeviceLogoLedColorCustom  = function(vid,pid,sn,colorR,colorG,colorB)
      {
          var Obj = {
                         Type:funcVar.Types.Mouse, 
                         SN:sn, 
                         Func:funcVar.Names.SetDeviceLogoLedColorCustom, 
                         Param : { colorR : colorR,colorG : colorG, colorB :colorB }  
                    }
          GeniusProtocol.RunFunction(Obj, function (error,data){  env.log('[ERR]','[services]','[SetDeviceLogoLedColorCustom]',error);  });
      };
      geniusScoropMouseShareService.SetDeviceLogoLedColorSystem  = function(vid,pid,sn,colorVal)
      {
          var Obj = { 
                        Type:funcVar.Types.Mouse, 
                        SN:sn,  
                        Func:funcVar.Names.SetDeviceLogoLedColorSystem,  
                        Param :  colorVal  
                    }
          GeniusProtocol.RunFunction(Obj, function (error,data){  env.log('[ERR]','[services]','[SetDeviceLogoLedColorSystem]',error);});
      };
      geniusScoropMouseShareService.SetDeviceLogoLedBreathGradeOn  = function(vid,pid,sn,colorNum)
      {
           var Obj = {  
                        Type:funcVar.Types.Mouse,  
                        SN:sn,  
                        Func:funcVar.Names.SetDeviceLogoLedBreathGradeOn,   
                        Param : parseInt(colorNum,10)   
                     }
           GeniusProtocol.RunFunction(Obj, function (error,data){ env.log('[ERR]','[services]','[SetDeviceLogoLedBreathGradeOn]',error);  });
      };
      geniusScoropMouseShareService.SetDeviceLogoLedBreathGradeOff  = function(vid,pid,sn,colorNum)
      {
           var Obj = { 
                        Type:funcVar.Types.Mouse,  
                        SN:sn,  
                        Func:funcVar.Names.SetDeviceLogoLedBreathGradeOff, 
                        Param : parseInt(colorNum,10)      
                     }
           GeniusProtocol.RunFunction(Obj, function (error,data){ env.log('[ERR]','[services]','[SetDeviceLogoLedBreathGradeOff]',error); });
      };
      geniusScoropMouseShareService.SetDeviceGradOff = function(vid,pid,sn,dpiNum)
      {
        var Obj = {  Type:funcVar.Types.Mouse,  SN:sn, Func:funcVar.Names.SetDeviceGradOff, Param : parseInt(dpiNum,10)    }
         GeniusProtocol.RunFunction(Obj, function (error,data){  env.log('[ERR]','[services]','[SetDeviceGradOff]',error); });
      };
      geniusScoropMouseShareService.SetDeviceGradOnValue = function(vid,pid,sn,dpiNum,syncMode,dpiX,dpiY)
      {
        var Obj = {  Type:funcVar.Types.Mouse, SN:sn, Func:funcVar.Names.SetDeviceGradOnValue,  Param : { dpiNum : parseInt(dpiNum,10), syncMode : syncMode, dpiX : parseInt(dpiX,10), dpiY : parseInt(dpiY,10)  } }
        GeniusProtocol.RunFunction(Obj, function (error,data){ env.log('[ERR]','[services]','[SetDeviceGradOnValue]',error); });
      };
      geniusScoropMouseShareService.SetDeviceReportRate = function(vid,pid,sn,val)
      {
        var Obj = { Type:funcVar.Types.Mouse, SN:sn, Func:funcVar.Names.SetDeviceReportRate, Param :  parseInt(val,10)  }
        GeniusProtocol.RunFunction(Obj, function (error,data){   env.log('[ERR]','[services]','[SetDeviceReportRate]',error); });
      };

      geniusScoropMouseShareService.AddProfile = function(sn,profileContent)
      {
        var Obj = {  Type:funcVar.Types.Mouse,  SN:sn, Func:funcVar.Names.AddProfile, Param :  profileContent   }
        GeniusProtocol.RunFunction(Obj, function (error,data){    env.log('[ERR]','[services]','[AddProfile]',error);  });
      };

      geniusScoropMouseShareService.ModifyProfile = function(sn,profileId,profileContent)
      {
        var Obj = {  Type:funcVar.Types.Mouse,  SN:sn, Func:funcVar.Names.ModifyProfile, Param : { profileId : parseInt(profileId),Content:profileContent}  }
        GeniusProtocol.RunFunction(Obj, function (error,data){    env.log('[ERR]','[services]','[ModifyProfile]',error); });
      }; 

      geniusScoropMouseShareService.RefreshProfile = function(sn)
      {
        var Obj = {  Type:funcVar.Types.Mouse,  SN:sn, Func:funcVar.Names.RefreshProfile, Param :  undefined  }
        GeniusProtocol.RunFunction(Obj, function (error,data){    env.log('[ERR]','[services]','[RefreshProfile]',error); });
      };

      geniusScoropMouseShareService.DeleteProfile = function(sn,profileId)
      {
          var Obj = { Type:funcVar.Types.Mouse,  SN:sn,  Func:funcVar.Names.DeleteProfile,  Param :  profileId      }
          GeniusProtocol.RunFunction(Obj, function (error,data){  env.log('[ERR]','[services]','[DeleteProfile]',error); });
      };

      geniusScoropMouseShareService.FactoryReset = function(sn,fn)
      {
         var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.FactoryReset,   Param :  undefined     }
         GeniusProtocol.RunFunction(Obj, function (error,data){  fn(error,data);  env.log('[ERR]','[services]','[FactoryReset]',error); });
      };

      geniusScoropMouseShareService.ModifyProfileAppList = function(sn,profileId,appList,ProfileName,imgName)
      {
         var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.ModifyProfileAppList,   Param :  {profileId : parseInt(profileId) , appList : appList ,profileName : ProfileName, imageName :imgName }     }
         GeniusProtocol.RunFunction(Obj, function (error,data){  env.log('[ERR]','[services]','[ModifyProfileAppList]',error); });
      };


      geniusScoropMouseShareService.updateFwVer = function(sn)
      {
         var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.UpdateFirmware,   Param :  null    }
         $("#txt_div_update_info").val($translate.instant("update.success"));
         GeniusProtocol.RunFunction(Obj, function (error,data)
         {              
                 if (error == undefined) { $("#div_update_info").html($translate.instant("update.success"));   }
                 else 
                 {
                       $("#div_error_message").html($translate.instant("update.error") + $translate.instant("code.genius.error.0x"+error.toString(16)));
                       $('#dialog_update_error').modal({ show: true, backdrop: 'static' }); 
                        env.log('[ERR]','[services]','[updateFwVer]',error);
                 } 
         });
      };

      geniusScoropMouseShareService.updateFwVerBoot = function(sn)
      {
         var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.UpdateFirmwareBoot,   Param :  null    }
         $("#txt_div_update_info").val($translate.instant("update.success"));
         GeniusProtocol.RunFunction(Obj, function (error,data)
         { 
               if (error == undefined) { $("#div_update_info").html($translate.instant("update.success")); env.log('App services',error,Obj.SN,Obj.Func );   }
               else 
               {
                     $("#div_error_message").html($translate.instant("update.error") + $translate.instant("code.genius.error.0x"+error.toString(16)));
                     $('#dialog_update_error').modal({ show: true, backdrop: 'static' }); 
                     env.log('[ERR]','[updateFwVerBoot]','[updateFwVer]',error);
               } 
          });
      };
      geniusScoropMouseShareService.SetDeviceMacro = function(vid,pid,sn,macroNum,macroContent)
      {
         var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.SetDeviceMacro,   Param :  { macroNum : parseInt(macroNum,10), aMacro : macroContent } }
         GeniusProtocol.RunFunction(Obj, function (error,data){ env.log('[ERR]','[services]','[SetDeviceMacro]',error);   });
      }

      geniusScoropMouseShareService.getApplicationIconMouse = function(sn,sourceDir,toDir,iSize)
      {
         var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.getApplicationIcon, Param :{from:sourceDir, To: toDir,Size:iSize } }
         GeniusProtocol.RunFunction(Obj, function (error,data){ env.log('[ERR]','[services]','[getApplicationIconMouse]',error);    });
      }

      geniusScoropMouseShareService.getApplicationIconKB = function(sn,sourceDir,toDir,iSize)
      {
         var Obj = { Type:funcVar.Types.Keyboard, SN:sn,Func:funcVar.Names.getApplicationIcon, Param :{from:sourceDir, To: toDir,Size:iSize } }
         GeniusProtocol.RunFunction(Obj, function (error,data){  env.log('[ERR]','[services]','[getApplicationIconKB]',error);   });
      }
    
      geniusScoropMouseShareService.SetDeviceNowDPIValue = function(vid,pid,sn,dpiNum,syncMode,dpiX,dpiY)
      {
        var Obj = {  Type:funcVar.Types.Mouse, SN:sn, Func:funcVar.Names.SetDeviceGradOnValue,  Param : { dpiNum : parseInt(dpiNum,10), syncMode : syncMode, dpiX : parseInt(dpiX,10), dpiY : parseInt(dpiY,10)  } }
        GeniusProtocol.RunFunction(Obj, function (error,data){ env.log('[ERR]','[services]','[SetDeviceGradOnValue]',error); });
      }

      geniusScoropMouseShareService.DeleteDeviceMacro = function(vid,pid,sn,macroNum)
      {
           var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.DeleteDeviceMacro,   Param : parseInt(macroNum,10) }
           GeniusProtocol.RunFunction(Obj, function (error,data){   env.log('[ERR]','[services]','[DeleteDeviceMacro]',error); });
      };

      geniusScoropMouseShareService.SetDeviceGradOnValueAll = function(vid,pid,sn,syncMode,ary,val)
      {
              if (ary.length > 0 )
              {
                  var dpiAry=[];
                  for(var i=0;i<ary.length;i++){
                    if(ary[i].onOff === 'on')
                    {
                      var dpiJson={};
                      dpiJson.Id=ary[i].Id;
                      dpiJson.DPI=ary[i].xDPI;
                      dpiAry.push(dpiJson);
                    }
                  }
                  var idx=0;
                  var doSet=function()
                  {
                    if(idx<dpiAry.length)
                    {
                          var Obj = {
                                        Type:funcVar.Types.Mouse,
                                        SN:sn, 
                                        Func:funcVar.Names.SetDeviceGradOnValue,  
                                        Param : { dpiNum :parseInt(dpiAry[idx].Id,10), syncMode : syncMode, dpiX : parseInt(dpiAry[idx].DPI,10), dpiY : parseInt(dpiAry[idx].DPI,10)  } 
                                    }
                          GeniusProtocol.RunFunction(Obj, function (error,data){  idx++; doSet(); });
                    }else
                    {
                         var Obj = { 
                                        Type:funcVar.Types.Mouse, 
                                        SN:sn, 
                                        Func:funcVar.Names.SetDeviceGradOnValue,  
                                        Param : { dpiNum :parseInt(dpiAry[val-1].Id,10), syncMode : syncMode, dpiX : parseInt(dpiAry[val-1].DPI,10), dpiY : parseInt(dpiAry[val-1].DPI,10)  }
                                  }
                        GeniusProtocol.RunFunction(Obj, function (error,data){  env.log('[ERR]','[services]','[SetDeviceGradOnValueAll]',error);  });
                    }
                  }
                  doSet();
              }
      }
      return geniusScoropMouseShareService;
 })

.factory('geniusTravelerMouseService', function()
{
    var geniusTravelerMouseShareService = {};

    geniusTravelerMouseShareService.GetDeviceGradToggle = function(sn,fn)
    {
       var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.GetDeviceGradToggle,   Param : null } ;
       GeniusProtocol.RunFunction(Obj,function (data) {  fn(data) ;  });
    }
    geniusTravelerMouseShareService.GetSwapButtonState = function(fn)
    {
       var Obj = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.GetSwapButtonState,   Param : null } ;
       GeniusProtocol.RunFunction(Obj,function (data) { fn(data)  ;   });
    }
     geniusTravelerMouseShareService.GetMouseDoubleClickSpeed = function(fn)
    {
       var Obj = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.GetMouseDoubleClickSpeed,   Param : null } ;
       GeniusProtocol.RunFunction(Obj,function (data) { fn(data) ; });
    }
    geniusTravelerMouseShareService.GetMousePointSpeed = function(fn)
    {
       var Obj = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.GetMousePointSpeed,   Param : null } ;
       GeniusProtocol.RunFunction(Obj,function (data) { fn(data) ;});
    }
    geniusTravelerMouseShareService.GetDeviceWheelSpeed = function(fn)
    {
       var Obj = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.GetDeviceWheelSpeed,   Param : null } ;
       GeniusProtocol.RunFunction(Obj,function (data) { fn(data) ; });
    }
    geniusTravelerMouseShareService.GetBatteryStatus = function(sn,fn)
    {
       var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.GetBatteryStatus,   Param : null } ;
       GeniusProtocol.RunFunction(Obj,function (data) { fn(data) ; });
    }

    geniusTravelerMouseShareService.SetSwapButtonState = function (btn)
    {
       var Obj = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.SetSwapButtonState,   Param : btn } ;
       GeniusProtocol.RunFunction(Obj,function (data) { env.log('App services',data,Obj.SN,Obj.Func,Obj.Param); });
    }
    geniusTravelerMouseShareService.SetMouseDoubleClickSpeed = function (btn)
    {
       var Obj = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.SetMouseDoubleClickSpeed,   Param : btn } ;
       GeniusProtocol.RunFunction(Obj,function (data) {  env.log('App services',data,Obj.SN,Obj.Func,Obj.Param);  });
    }
    geniusTravelerMouseShareService.SetMousePointSpeed = function (btn)
    {
       var Obj = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.SetMousePointSpeed,   Param : btn } ;
       GeniusProtocol.RunFunction(Obj,function (data) {  env.log('App services',data,Obj.SN,Obj.Func,Obj.Param);   });
    }
    geniusTravelerMouseShareService.SetDeviceWheelSpeed = function (btn)
    {
       var Obj = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.SetDeviceWheelSpeed,   Param : btn } ;
       GeniusProtocol.RunFunction(Obj,function (data) {  env.log('App services',data,Obj.SN,Obj.Func,Obj.Param);   });
    }
    geniusTravelerMouseShareService.SetDeviceGradToggle =function(sn,dpi)
    {
       var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.SetDeviceGradToggle,   Param : dpi } ;
       GeniusProtocol.RunFunction(Obj,function (data) {  env.log('App services',data,Obj.SN,Obj.Func,Obj.Param);   });
    }

   geniusTravelerMouseShareService.SetDeviceButtonKey = function(vid,pid,sn,buttonNum,setType,setVal)
   {
      var Obj={ Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.SetDeviceButtonKey, Param:{ buttonNum : parseInt(buttonNum,10), setType : parseInt(setType,10), setVal : setVal } };
      GeniusProtocol.RunFunction(Obj, function (error,data){ env.log('App services',error,Obj.SN,Obj.Func,Obj.Param.buttonNum,Obj.Param.setType,Obj.Param.setVal) });
   };

   geniusTravelerMouseShareService.GetBatteryStatus = function(sn,fn)
   {
      var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.GetBatteryStatus,   Param : null } ;
      GeniusProtocol.RunFunction(Obj,function (obj1) { fn(obj1) ;env.log('App services',obj1,Obj.SN,Obj.Func); });
   }

   geniusTravelerMouseShareService.GetChargeStatus = function(sn,fn)
   {
      var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.GetChargeStatus,   Param : null } ;
      GeniusProtocol.RunFunction(Obj,function (obj1) { fn(obj1) ;env.log('App services',obj1,Obj.SN,Obj.Func); });
   }

    geniusTravelerMouseShareService.getDeviceAllInfo = function(sn,fn)
    {
       var para = [] ;var Obj = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.GetSwapButtonState,   Param : null } ;
       GeniusProtocol.RunFunction(Obj,function (data1)
       {
           para.push(data1);var Obj1 = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.GetMouseDoubleClickSpeed,   Param : null } ;
           GeniusProtocol.RunFunction(Obj1,function (data2)
           {
              para.push(data2);var Obj2 = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.GetMousePointSpeed,   Param : null } ;
              GeniusProtocol.RunFunction(Obj2,function (data3)
              {
                 para.push(data3);var Obj3 = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.GetDeviceWheelSpeed,   Param : null } ;
                 GeniusProtocol.RunFunction(Obj3,function (data4)
                 {
                    para.push(data4);var Obj4 = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.GetDeviceGradToggle,   Param : null } ;
                    GeniusProtocol.RunFunction(Obj4,function (data5){ para.push(data5); fn(para)});
                 });
              });
           });
       });
    }

    return  geniusTravelerMouseShareService ;
})
.service('geniusSystemService', function($location,$translate,$rootScope)
{
      this.keyCodeData = [] ;
      var _this = this ;

      this.getkeyCodes = function()
      {
         var data = fs.readFileSync(path.join(env.appRoot,'App','Generic','KeyCode.json'), 'utf-8');
         this.keyCodeData = eval("(" + data + ")");
         return angular.fromJson(this.keyCodeData);
      }　
      this.getSelectedDevice = function() {   return 0;    }
      this.setSelectedDevice = function(selected) {   }
      this.getSupportModels = function()
      {
         var supModels = [] ;
         var SupportModels =path.join(env.deviceSettingsHome,'SupportModels.json');
         var data = fs.readFileSync(SupportModels, 'utf-8');
         supModels =eval("("+data+")");
         return angular.fromJson(supModels);
      }

      this.getAreaLocation = function()
      {
          try
          {
               var areacode = ""; var AppSettings =path.join(env.appSettingsHome,'AppSettings.json');
               if (fs.existsSync(AppSettings))
               {
                   var data = fs.readFileSync(AppSettings, 'utf-8');  
                   var t = eval("("+data+")");
                   if (t.hasOwnProperty("Location"))
                   {
                      areacode = data.Location.code;
                   }
                   else
                   {
                      areacode = "en";
                   }
               }
               else
               {
                   areacode = "en";
               }
               return areacode;
          }catch(e){
               areacode = "en";
          }
      }

      this.getMacros = function()
      {
         var macroData = [];
         try
         {
            var macro =path.join(env.deviceSettingsHome,'userMacro.json');
            if (fs.existsSync(macro))
            {
              var data = fs.readFileSync(macro, 'utf-8');  
              macroData =eval("("+data+")"); 
            }
            else
            {
                var  t = {}; t.macro = [];  
                [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19].forEach(function(i)
                {
                  var mt = {};
                  mt.macroId = i+1;
                  mt.macroName ="Macro"+(i+1);
                  mt.macroContent = [];
                  mt.MacroMode = 1;
                  mt.MacroRepeatTimes =1;
                  t.macro.push(mt);
                });
                this.saveDeviceProfilestoJson("userMacro",t); 
                macroData = t;
            }
          }catch(e){

          }  
         return angular.fromJson(macroData); 
      }

      this.saveJsonfile = function(path,data)
      { 
         tools.SaveJsonFile(path,data);
      }

      this.getDevices = function()
      {
           var deviceData = [] ; var macroData = [];
           try
           {
               var devices =path.join(env.deviceSettingsHome,'PlugDevices.json');
               var data = fs.readFileSync(devices, 'utf-8');  
               var t = eval("("+data+")"); 
               var supModels = _this.getSupportModels();
               if (t.Mouse.length > 0 )
               {
                   for (var i = 0 ; i < t.Mouse.length ; i++)
                   {
                      for (var j = 0 ; j < supModels.length ; j++)
                      { 
                          var tt = {} ; var iNo = Number(t.Mouse[i].DeviceTypeId) ;                  
                          if( parseInt(t.Mouse[i].VID) === parseInt(supModels[j].VID)  &&
                              parseInt(t.Mouse[i].PID) === parseInt(supModels[j].PID)  &&
                              parseInt(supModels[j].ModelType)  === 1 )
                          {  
                             tt.deviceId = t.Mouse[i].DeviceId ;
                             tt.fwVersion = t.Mouse[i].FwVersion ;
                             tt.bootVersion = t.Mouse[i].BootVersion ;
                             tt.VID  = t.Mouse[i].VID ;
                             tt.PID = t.Mouse[i].PID ;
                             tt.DeviceTypeId = t.Mouse[i].DeviceTypeId ;
                             tt.DeviceConnectState = t.Mouse[i].DeviceConnectState ;
                             tt.PCBAType = t.Mouse[i].DeviceTypeId ;
                             tt.model = "mouse" ;
                             for(var k = 0 ; k < supModels[j].ModelTypeNames.length ; k++)
                             {
                                 if ( parseInt(supModels[j].ModelTypeNames[k].ID) == parseInt(iNo) )
                                 {
                                     tt.modelName = supModels[j].ModelTypeNames[k].ModelName ;
                                     tt.imageUrl = supModels[j].ModelTypeNames[k].ImageUrl ;
                                     tt.MaxDPI = supModels[j].ModelTypeNames[k].MaxDPI ;
                                     tt.Keys = supModels[j].ModelTypeNames[k].Keys ;
                                     tt.controller = supModels[j].ModelTypeNames[k].Controller ;
                                     tt.AppPathDir = supModels[j].ModelTypeNames[k].AppPathDir ;
                                     tt.modelLabelName = supModels[j].ModelTypeNames[k].ModelLabelName ;
                                     tt.ObjProperty =     supModels[j].ModelTypeNames[k].ObjProperty ;  
                                     if (supModels[j].ModelTypeNames[k].ObjProperty.isGame == 1)
                                     {
                                         $rootScope.$broadcast('RefreshNavMacro'); 
                                     }
                                     deviceData.push(tt);
                                     break;
                                 }
                             }
                          }
                      }
                   }
               }

               if (t.Keyboard.length > 0 )
               {
                   for (var i = 0 ; i < t.Keyboard.length ; i++)
                   {
                      for (var j = 0 ; j < supModels.length ; j++)
                      {
                          var kb = {} ; var iNo = Number(t.Keyboard[i].DeviceTypeId) ;
                          if( parseInt(t.Keyboard[i].VID) === parseInt(supModels[j].VID)  &&
                              parseInt(t.Keyboard[i].PID) === parseInt(supModels[j].PID)  &&
                              parseInt(supModels[j].ModelType)  === 2   )
                          {
                             kb.deviceId = t.Keyboard[i].DeviceId ;
                             kb.fwVersion = t.Keyboard[i].FwVersion ;
                             kb.bootVersion = t.Keyboard[i].BootVersion ;
                             kb.VID  = t.Keyboard[i].VID ;
                             kb.PID = t.Keyboard[i].PID ;
                             kb.DeviceTypeId = t.Keyboard[i].DeviceTypeId ;
                             kb.DeviceConnectState = t.Keyboard[i].DeviceConnectState ;
                             kb.PCBAType = t.Keyboard[i].DeviceTypeId ;
                             kb.model = "keyboard" ;
                             for(var k = 0 ; k < supModels[j].ModelTypeNames.length ; k++)
                             {
                                 if ( parseInt(supModels[j].ModelTypeNames[k].ID) == parseInt(iNo) )
                                 {
                                     kb.modelName = supModels[j].ModelTypeNames[k].ModelName ;
                                     kb.imageUrl = supModels[j].ModelTypeNames[k].ImageUrl ;
                                     kb.MaxDPI = supModels[j].ModelTypeNames[k].MaxDPI ;
                                     kb.Keys = supModels[j].ModelTypeNames[k].Keys ;
                                     kb.controller = supModels[j].ModelTypeNames[k].Controller ;
                                     kb.AppPathDir = supModels[j].ModelTypeNames[k].AppPathDir ;
                                     kb.modelLabelName = supModels[j].ModelTypeNames[k].ModelLabelName ;
                                     kb.ObjProperty =     supModels[j].ModelTypeNames[k].ObjProperty ;
                                     if (supModels[j].ModelTypeNames[k].ObjProperty.isGame == 1)
                                     {
                                         $rootScope.$broadcast('RefreshNavMacro');
                                     }
                                     deviceData.push(kb);
                                     break;
                                 }
                             }
                          }
                      }
                   }
               }
           }catch(e)
           {
              env.log("App service getDevices error" +e);
           }
           return angular.fromJson(deviceData);
      }

      this.getDeviceProfiles = function(deviceId)
      {       
          var profileData = [] ;  var macroData = [];
          var macro = path.join(env.deviceSettingsHome,'userMacro.json'); 
          try
          {
              var SnProfile =path.join(env.deviceSettingsHome,deviceId+'.json');            
              if (fs.existsSync(SnProfile))
              {
                 var data = fs.readFileSync(SnProfile, 'utf-8'); 
                 profileData =eval("("+data+")"); 
                 if (profileData.hasOwnProperty("templateList")) //KB profile
                 { 
                    profileData.templateList.forEach(function(e)
                    {    
                        if (e.GroupID == 4)
                        {
                             if (fs.existsSync(macro))
                             {
                                  e.categoryFunc = [];
                                  var mdata =  fs.readFileSync(macro, 'utf-8');
                                  macroData = eval("("+mdata+")"); 
                                  macroData.macro.forEach(function(m)
                                  {    
                                      var t = {};
                                      t.FuncTypeName = m.macroName;
                                      t.FuncTypeID  = m.macroId;
                                      t.ProgramAction = 4;
                                      t.Param1 ="";
                                      t.Param2 ="";
                                      t.MacroMode =m.MacroMode;
                                      t.MacroRepeatTimes =m.MacroRepeatTimes;
                                      t.macroContent = m.macroContent;
                                      e.categoryFunc.push(t);  
                                  });   
                             } 
                          
                        }
                    });    
                    profileData.profiles.forEach(function(kbp)
                    {
                          if(!kbp.hasOwnProperty("activeProfile"))
                          {
                             kbp.activeProfile = 0;
                          }
                    });
                 }
                 else   //Mouse Proifle
                 {
                     if (fs.existsSync(macro))
                     {
                          var mdata =  fs.readFileSync(macro, 'utf-8');
                          macroData = eval("("+mdata+")"); 
                          profileData.profiles.forEach(function(e)
                          {    
                            if(!e.hasOwnProperty("activeProfile")) { e.activeProfile = 0;  } 
                            e.macro = macroData.macro;
                             delete e.$$hashKey ;   
                          });   
                     } 
                 }                 
              }
              else
              {
                  env.log('App services','error','getDeviceProfiles',' file not exists ',SnProfile);$location.path("/app/profile");$location.replace();
              }

          }catch(e)
          {
          }
         
          this.saveDeviceProfilestoJson(deviceId,profileData);
                    
          return angular.fromJson(profileData.profiles);
    　}



      this.getDeviceProfilesN = function(deviceId)
      {
          var profileData = [] ;  var macroData = [];
          var macro = path.join(env.deviceSettingsHome,'userMacro.json'); 
          try
          {
              var SnProfile =path.join(env.deviceSettingsHome,deviceId+'.json');            
              if (fs.existsSync(SnProfile))
              {
                 var data = fs.readFileSync(SnProfile, 'utf-8'); 
                 profileData =eval("("+data+")"); 
                 if (profileData.hasOwnProperty("templateList")) //KB profile
                 { 
                    profileData.templateList.forEach(function(e)
                    {    
                        if (e.GroupID == 4)
                        {
                             if (fs.existsSync(macro))
                             {
                                  e.categoryFunc = [];
                                  var mdata =  fs.readFileSync(macro, 'utf-8');
                                  macroData = eval("("+mdata+")"); 
                                  macroData.macro.forEach(function(m)
                                  {    
                                      var t = {};
                                      t.FuncTypeName = m.macroName;
                                      t.FuncTypeID  = m.macroId;
                                      t.ProgramAction = 4;
                                      t.Param1 ="";
                                      t.Param2 ="";
                                      t.MacroMode =m.MacroMode;
                                      t.MacroRepeatTimes =m.MacroRepeatTimes;
                                      t.macroContent = m.macroContent;
                                      e.categoryFunc.push(t);  
                                  });   
                             } 
                          
                        }
                    });    
                    profileData.profiles.forEach(function(kbp)
                    {
                          if(!kbp.hasOwnProperty("activeProfile"))
                          {
                             kbp.activeProfile = 0;
                          }
                    });
                 }
                 else   //Mouse Proifle
                 {
                     if (fs.existsSync(macro))
                     {
                          var mdata =  fs.readFileSync(macro, 'utf-8');
                          macroData = eval("("+mdata+")"); 
                          profileData.profiles.forEach(function(e)
                          {    
                            if(!e.hasOwnProperty("activeProfile")) { e.activeProfile = 0;  } 
                            e.macro = macroData.macro;
                             delete e.$$hashKey ;   
                          });   
                     } 
                 }                 
              }
              else
              {
                  env.log('App services','error','getDeviceProfiles',' file not exists ',SnProfile); 
              }

          }catch(e)
          {
          }
         
          this.saveDeviceProfilestoJson(deviceId,profileData);
                    
          return angular.fromJson(profileData.profiles);   
      }

      this.getKeyboardDevicesTemplateList = function(deviceId){
          // var templateList = path.join(env.deviceSettingsHome,'userMacro.json');
          var KboardJson = {};
          var templateList = path.join(env.deviceSettingsHome,deviceId+'.json');
          if (fs.existsSync(templateList)){
            var data = fs.readFileSync(templateList, 'utf-8');
            KboardJson = JSON.parse(data);
          }
          return angular.fromJson(KboardJson.templateList);
      }

      this.getKeyboardDevicesCurProfileId = function(deviceId){
          var KboardJson = {};
          var templateList = path.join(env.deviceSettingsHome,deviceId+'.json');
          if (fs.existsSync(templateList)){
            var data = fs.readFileSync(templateList, 'utf-8');
            KboardJson = JSON.parse(data);
          }
          return angular.fromJson(KboardJson.curProifleId);
      }

      this.getKeyboardInitProfile = function(){
          // var templateList = path.join(env.deviceSettingsHome,'userMacro.json');
          var KboardJson = {};
          var templateList = path.join(env.deviceSettingsHome,'NewModelsFactory.json');
          if (fs.existsSync(templateList)){
            var data = fs.readFileSync(templateList, 'utf-8');
            KboardJson = JSON.parse(data);
          }
          return angular.fromJson(KboardJson.Keyboard.profiles[0]);
      }

      this.getKeyboardFristPluginState = function(deviceId){
          var KboardJson = {};
          var templateList = path.join(env.deviceSettingsHome,deviceId+'.json');
          if (fs.existsSync(templateList)){
            var data = fs.readFileSync(templateList, 'utf-8');
            KboardJson = JSON.parse(data);
          }
          return angular.fromJson(KboardJson.FristPlugin);
      }

      this.getKeyboardMyFavoriteLastId = function(deviceId,index){
          // var templateList = path.join(env.deviceSettingsHome,'userMacro.json');
          var KboardJson = {};
          var funcId = 0;
          var templateList = path.join(env.deviceSettingsHome,deviceId+'.json');
          if (fs.existsSync(templateList)){
            var data = fs.readFileSync(templateList, 'utf-8');
            KboardJson = JSON.parse(data);
          }
          var length = KboardJson.templateList[index].categoryFunc.length;
          // console.log(length);
          if(length!=0){
            funcId = KboardJson.templateList[index].categoryFunc[length-1].FuncTypeID; 
          }else{
            funcId = 0;
          }
          return funcId;
      }

      this.checkKbProfileName = function(deviceId,pName,type){
        var result = "";
        var KboardJson = {};
        var kbPath = path.join(env.deviceSettingsHome,deviceId+'.json');
        if (fs.existsSync(kbPath)){
          var kbData = fs.readFileSync(kbPath, 'utf-8');
          KboardJson = JSON.parse(kbData);
        }
        var wordLength = pName.length;
        var sameCount = 1;
        // console.log(KboardJson.profiles);
        KboardJson.profiles.forEach(function(e){
          // console.log("profileName :"+e.profileName.substr(0,wordLength));
          // console.log("pName :"+ pName);
          // console.log("profileName"+e.profileName.substr(0,wordLength));
          if (e.profileName.substr(0,wordLength) == pName ){
            if(e.profileName == pName){
              sameCount = sameCount+1;
            }else{
              if(e.profileName.length == (wordLength + 3) &&  e.profileName.substring(e.profileName.length-1,e.profileName.length)==")"){
                sameCount = sameCount+1;
              }
            }
          }
        });
        if(sameCount > 1){
          result = pName + "(" + sameCount.toString() + ")";
        } else{
          result = pName;
        }
        return result;
      }

      this.checkSettingIsIni = function(deviceId){
        var resultBool = true;
        var KboardJson = {};
        var kbPath = path.join(env.deviceSettingsHome,deviceId+'.json');
        if (fs.existsSync(kbPath)){
          var kbData = fs.readFileSync(kbPath, 'utf-8');
          KboardJson = JSON.parse(kbData);
        }
        // console.log(KboardJson);
        var index = KboardJson.curProifleId;
        var modelJson = {};
        var modelPath = path.join(env.deviceSettingsHome,'NewModelsFactory.json');
        if (fs.existsSync(modelPath)){
          var modelData = fs.readFileSync(modelPath, 'utf-8');
          modelJson = JSON.parse(modelData);
        }
        // kbPath.profiles[index].keyMapping.forEach(function(e){
        //   if(e.GroupID == modelJson.Keyboard.profiles[0].keyMapping.GroupID && 
        //     e.FuncTypeID == modelJson.Keyboard.profiles[0].keyMapping.FuncTypeID){

        //   }
        // });
        for(var i in KboardJson.profiles[index].keyMapping){
          if(KboardJson.profiles[index].keyMapping[i].GroupID == modelJson.Keyboard.profiles[0].keyMapping[i].GroupID){
            if(KboardJson.profiles[index].keyMapping[i].FuncTypeID != modelJson.Keyboard.profiles[0].keyMapping[i].FuncTypeID){
              resultBool = false;
            }
          }else{
            resultBool=false;
          }
        }
        return resultBool;
      }

      this.cleanKbProfileItemIsNull = function(profileJson,keyBoardTemplateList,callback){
          var isRemoveSettint = true;
          var settingIndex = 0;
          var osType = this.GetOsVer();
          profileJson.forEach(function(profile){
              settingIndex = 0;
              profile.keyMapping.forEach(function(keyMapping){
                  isRemoveSettint=true;
                  if(keyMapping.GroupID != 0){
                      keyBoardTemplateList.forEach(function(group){
                          if(keyMapping.GroupID == group.GroupID){
                              group.categoryFunc.forEach(function(item){
                                  if(keyMapping.GroupID != 4){
                                      if(item.FuncTypeID == keyMapping.FuncTypeID){
                                          isRemoveSettint = false;
                                          if(item.FuncTypeName != keyMapping.FuncTypeName){
                                              keyMapping.FuncTypeName = item.FuncTypeName;
                                              keyMapping.ProgramAction = item.ProgramAction;
                                              keyMapping.Param1 = item.Param1;
                                              keyMapping.Param2 = item.Param2;
                                          }else{
                                              if(keyMapping.Param1.indexOf("|") != -1){
                                                  var strList =  keyMapping.Param1.split("|");
                                                  if(osType=="WIN"){
                                                      // keyMapping.Param1 = $scope.apppathModel[strList[0]]+strList[0];
                                                      keyMapping.Param1 = strList[0];
                                                  }else{
                                                      keyMapping.Param1 = "/Applications/"+strList[1];
                                                  }
                                              }
                                          }
                                      } else if(keyMapping.FuncTypeID == 9999){
                                          isRemoveSettint = false;
                                      }
                                  }else{
                                      if(item.FuncTypeID == keyMapping.FuncTypeID && item.macroContent.length>0){
                                          var isRemoveMacro = true;
                                          if(item.FuncTypeName != keyMapping.FuncTypeName){
                                              if(item.macroContent.length == keyMapping.macroContent.length){
                                                  for(var i=0;i< keyMapping.macroContent.length;i++){
                                                      if(item.macroContent[i].keyCode != keyMapping.macroContent[i].keyCode){
                                                          isRemoveMacro = false;
                                                      }
                                                  }
                                              }
                                          }
                                          if(isRemoveMacro){
                                              isRemoveSettint = false;
                                              keyMapping.FuncTypeName = item.FuncTypeName;
                                              keyMapping.ProgramAction = item.ProgramAction;
                                              keyMapping.Param1 = item.Param1;
                                              keyMapping.Param2 = item.Param2;
                                              keyMapping.MacroMode = item.MacroMode;
                                              keyMapping.MacroRepeatTimes = item.MacroRepeatTimes;
                                              keyMapping.macroContent = item.macroContent;
                                          } 
                                      }
                                  } 
                              });
                          }
                      });
                      if(isRemoveSettint){
                          keyMapping.FuncGroup = ""
                          keyMapping.GroupID = 0;
                          keyMapping.FuncTypeName = "";
                          keyMapping.FuncTypeID = 0;
                          keyMapping.ProgramAction = 0;
                          keyMapping.Param1 = "";
                          keyMapping.Param2 = "";
                          keyMapping.MacroMode = 0;
                          keyMapping.MacroRepeatTimes = 0;
                          keyMapping.macroContent = [];
                          // $scope.saveJsonFile();
                      }
                  } 
                  settingIndex = settingIndex + 1;
              });
          });
          // $scope.saveJsonFile();
          callback(profileJson,keyBoardTemplateList);
      }

      this.removeKeyboardMyFavorite = function(deviceId,index,tempIndex){
        var saveDate = {};
        var templateList = path.join(env.deviceSettingsHome,deviceId+'.json');
        if (fs.existsSync(templateList)){
          var profilesData = fs.readFileSync(templateList, 'utf-8');
          saveDate = JSON.parse(profilesData);
        }
        // console.log(saveDate);
        // var tempData = saveDate.templateList[5].categoryFunc;
        // for(var index in tempData){
        //   if(tempData[index].FuncTypeID==funcID){
        saveDate.templateList[tempIndex].categoryFunc.splice(index,1);
        //   }
        // }
        var kbProfile =path.join(env.deviceSettingsHome,deviceId+'.json');
        fs.writeFileSync(kbProfile, JSON.stringify(saveDate, null, "\t"));
      }

      this.addKeyboardMyFavorite = function(deviceId,data,index){
        var saveDate = {};
        var templateList = path.join(env.deviceSettingsHome,deviceId+'.json');
        if (fs.existsSync(templateList)){
          var profilesData = fs.readFileSync(templateList, 'utf-8');
          saveDate = JSON.parse(profilesData);
        }
        // console.log(saveDate);
        saveDate.templateList[index].categoryFunc.push(data);
        var kbProfile =path.join(env.deviceSettingsHome,deviceId+'.json');
        fs.writeFileSync(kbProfile, JSON.stringify(saveDate, null, "\t"));
      }

      this.changeProfileForApp = function(deviceId,index){
        var saveDate = {};
        var templateList = path.join(env.deviceSettingsHome,deviceId+'.json');
        if (fs.existsSync(templateList)){
          var profilesData = fs.readFileSync(templateList, 'utf-8');
          saveDate = JSON.parse(profilesData);
        }
        // console.log(saveDate);
        saveDate.curProifleId = index;
        var kbProfile =path.join(env.deviceSettingsHome,deviceId+'.json');
        fs.writeFileSync(kbProfile, JSON.stringify(saveDate, null, "\t"));
      }

      this.saveKeyboardTemplist = function(deviceId,data){
        var saveDate = {};
        var templateList = path.join(env.deviceSettingsHome,deviceId+'.json');
        if (fs.existsSync(templateList)){
          var profilesData = fs.readFileSync(templateList, 'utf-8');
          saveDate = JSON.parse(profilesData);
        }
        // console.log(saveDate);
        saveDate.templateList = data;
        var kbProfile =path.join(env.deviceSettingsHome,deviceId+'.json');
        fs.writeFileSync(kbProfile, JSON.stringify(saveDate, null, "\t"));
      }

      this.saveKeyboardFristPluginState =function(deviceId,value){
        var saveDate = {};
        var templateList = path.join(env.deviceSettingsHome,deviceId+'.json');
        if (fs.existsSync(templateList)){
          var profilesData = fs.readFileSync(templateList, 'utf-8');
          saveDate = JSON.parse(profilesData);
        }
        saveDate.FristPlugin = value;
        // console.log(saveDate.FristPlugin);
        var kbProfile =path.join(env.deviceSettingsHome,deviceId+'.json');
        fs.writeFileSync(kbProfile, JSON.stringify(saveDate, null, "\t"));
      }

      this.saveKeyboardDevicesJsonFile = function(deviceId,index,data){
          var saveDate = {};
          var templateList = path.join(env.deviceSettingsHome,deviceId+'.json');
          if (fs.existsSync(templateList)){
            var profilesData = fs.readFileSync(templateList, 'utf-8');
            saveDate = JSON.parse(profilesData);
          }
          saveDate.profiles = data;
          var kbProfile =path.join(env.deviceSettingsHome,deviceId+'.json');
          saveDate.curProifleId = index;
          // delete saveDate.$$hashKey;
          fs.writeFileSync(kbProfile, JSON.stringify(saveDate, null, "\t"));
      }

    　this.getSelectedProfile = function() {    return 0;  }
      this.setSelectedProfile = function(selected) {  }

      this.saveDeviceProfilestoJson = function(deviceId,data)
      {
          var SnProfile =path.join(env.deviceSettingsHome,deviceId+'.json');  
          fs.writeFileSync(SnProfile, JSON.stringify(data, null, "\t"));    
      }

      this.saveKbDeviceProfilestoJson = function(deviceId,data){
          var saveDate = {};
          var templateList = path.join(env.deviceSettingsHome,deviceId+'.json');
          if (fs.existsSync(templateList)){
            var profilesData = fs.readFileSync(templateList, 'utf-8');
            saveDate = JSON.parse(profilesData);
          }
          saveDate.profiles = data;
          var kbProfile =path.join(env.deviceSettingsHome,deviceId+'.json');
          fs.writeFileSync(kbProfile, JSON.stringify(saveDate, null, "\t"));
      }

      this.SaveMacroToJson = function(macroid,data,bool)
      {
         console.log("macroid:"+macroid+" data:"+data + " bool:"+bool);
         try
        { 
              var aMacrofilePath = path.join(env.deviceSettingsHome, 'userMacro.json');var macrofiles ;
              var t = [];  t =eval("("+data+")"); var iNo = -1 ;
              if (bool)
              {
                    if(fs.existsSync(aMacrofilePath))
                    {            
                       macrofiles = JSON.parse(fs.readFileSync(aMacrofilePath,'UTF-8'));
                    } 
                    macrofiles.macro.forEach(function(e)
                    { 
                        if (e.macroId == t.macroId)
                        {
                              delete e.macroContent;
                              e.macroName = t.macroName;
                              e.MacroMode =t.MacroMode;
                              e.MacroRepeatTimes =t.MacroRepeatTimes;
                              e.macroContent = t.macroContent;
                        }
                    })
              }
              else
              {  //import macro            
                    macrofiles = eval("("+fs.readFileSync(aMacrofilePath,'UTF-8')+")");  
                    macrofiles.macro.forEach(function(e)
                    { 
                        if (e.macroName === t.macroName) {iNo =e.macroId;}
                    });  
                    if (iNo == -1) 
                    { 
                        macrofiles.macro.push(t);
                    }
                    else
                    {
                         delete  macrofiles.macro[Number(iNo)-1].macroContent;
                         macrofiles.macro[Number(iNo)-1].macroName = t.macroName;
                         macrofiles.macro[Number(iNo)-1].MacroMode =t.MacroMode;
                         macrofiles.macro[Number(iNo)-1].MacroRepeatTimes =t.MacroRepeatTimes;
                         macrofiles.macro[Number(iNo)-1].macroContent = t.macroContent;
                    } 
               }
               
              fs.writeFileSync(aMacrofilePath, JSON.stringify(macrofiles, null, "\t")); 
        }catch(e){
            env.log(" [services] profileImport unknown error " + e);
        }
      }

      this.getDevicesfileslist = function()
      {
          var path = env.deviceSettingsHome;var files =[];
          var results = []
          var list = fs.readdirSync(path)
          list.forEach(function(file) {
              file = path + '/' + file
              var stat = fs.statSync(file)
              if (stat && stat.isDirectory()) results = results.concat(walk(file))
              else results.push(file)
          })
          return results; 
      }

      this.GetOsVer = function ()
      {
         var osVer = "" ;
         if (env.isWindows) { osVer ="WIN" ; }
         else if (env.isMac)     { osVer ="MAC" ; }
         else if (env.isLinux)   { osVer ="LIN" ; }
         return osVer ;
      }

       this.getZoomStatus =function(deviceId)
       {
             var  keyType =  tools.getMouseZoomStatus(deviceId);  
             return keyType ;
       }

       this.getalarmPara = function()
       {

          var para = "" ;
          var parafile =path.join(env.appSettingsHome,'AppSettings.json');
          if (fs.existsSync(parafile))
          {
            var data = fs.readFileSync(parafile, 'utf-8'); var profileData =eval("("+data+")");
            if (profileData.hasOwnProperty('eyePara'))
            {
                var t =  !isNaN(parseFloat(profileData.eyePara.times)) && isFinite(profileData.eyePara.times) ? profileData.eyePara.times : "60";
                para = profileData.eyePara.OnOff == true ?  "1,"+ t : "0,"+ t ;
            }
            else
            {
                para = "1,60";
            } 
          }
          else
          {
            para = "1,60";
          }
         return para ;
       }

      this.saveAlarmPara = function(bswith,time)
      {
        try
        {  
          var para =path.join(env.appSettingsHome,'AppSettings.json');
          var data = fs.readFileSync(para, 'utf-8'); 
           var p = eval("("+data+")");
           p.eyePara = {};var t = {} ; t.OnOff = bswith ;t.times =time ;
           p.eyePara = t;
           fs.writeFileSync(para, JSON.stringify(p, null, "\t")); 
           
        }catch(e){env.log(e);}
      }

      this.ToggleProfile = function(sn,profileId)
      {
          try
          {
             $("#dialog_Toggle_Profile").modal({ show: true, backdrop: 'static' });$("#txtToggleProfile").val("1");
             var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.ToggleProfile,   Param : profileId } ;
             GeniusProtocol.RunFunction(Obj,function (error,data)
                                            {
                                               if (error === undefined)
                                               {
                                                   setTimeout(function(){$("#dialog_Toggle_Profile").modal("hide");},500);$("#txtToggleProfile").val("");
                                               }
                                               else
                                               {
                                                  setTimeout(function(){$("#dialog_Toggle_Profile").modal("hide");},500);$("#txtToggleProfile").val("");
                                               }
                                            });
          }catch(e){console.log(e);}
       }

       this.getFirmwareinfo = function()
       {
           var firmwareData = []
           try
           {
              var data = fs.readFileSync( env.updateVersionPath, 'utf-8');
              firmwareData =   eval("(" + data + ")");
              if(!firmwareData.hasOwnProperty('AppDesc'))
              {
                 var t = {} ;
                 t.AppVer = env.getCurrentDataTimeFormat('yyyyMMdd');
                 t.AppShowVerNew = env.version ;
                 t.AppDesc = [];
                 t.SupportModelsFileVer = env.getCurrentDataTimeFormat('yyyyMMdd'); 
                 t.ModelsFactoryVer = env.getCurrentDataTimeFormat('yyyyMMdd');
                 t.FirmwareVers = [];
                 t.DriverVer = '1.8.3.0';
                 fs.writeFileSync(env.updateVersionPath, JSON.stringify(t, null, "\t")); 
              }
           }catch(e)
           {
             console.log(e);
           }
           return angular.fromJson(firmwareData);
       }

       this.softWareDownload= function(url,dest)
       {
          var Obj = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.DownloadInstallPackage,   Param : {InstallPackDownloadURL :url ,InstallPackTempPath :dest} } ;
          GeniusProtocol.RunFunction(Obj,function (error,data)
          {
             if( error != undefined)
             {
                $('#dialog_software_update').modal("hide");
                $("#div_error_message").html($translate.instant("update.error") + $translate.instant("code.genius.error.0x"+error.toString(16)));
                $('#dialog_update_error').modal({ show: true, backdrop: 'static' });
                env.log('App services',error,Obj.SN,Obj.Func,Obj.Param);
             }
              env.log('App services',error,Obj.SN,Obj.Func,Obj.Param);
          });
       }
       this.UpdateDriver = function(fn)
       {
          try
          {  var Obj = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.UpdateDriver,   Param : null } ;
             GeniusProtocol.RunFunction(Obj,function (data) {  fn(data);  });
          }catch(e){console.log(e);}

       }
       this.CheckDriverInstall = function(sn,fn)
       {
          try
          {  var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.CheckDriverInstall,   Param : null } ;
             GeniusProtocol.RunFunction(Obj,function (data) {   fn(data);  });
          }catch(e){console.log(e);}
       }

       this.DownloadInstallDriver = function(fn)
       {
           var Obj = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.DownloadInstallDriver,   Param:{DriverDownloadURL : null} } ;
           GeniusProtocol.RunFunction(Obj,function (data)
           {
                $("#dialog_ammox_Install_Driver_DownloadProgress").modal("hide");
                $("#dialog_nx7010_Install_Driver_DownloadProgress").modal("hide");
                $("#dialog_nx7015_Install_Driver_DownloadProgress").modal("hide");
                if( data != undefined)
                {
                   $("#div_error_message").html($translate.instant("update.error") + $translate.instant("code.genius.error.0x"+data.toString(16)));
                   $('#dialog_update_error').modal({ show: true, backdrop: 'static' });
                   fn(data);
                }

            });
       }

       this.saveProfile = function(paths,data,vpid,fn)
       {
         try
         {
             tools.saveProfileforJson(paths,data,vpid,fn)
             env.log('App Services saveProfile  Save ',paths);
         }catch(e)
         {
            env.log('App Services saveProfile  Save error',paths+'   '+e);
            fn('ng');
         }
       }

       this.saveKbProfile = function(paths,data,vpid,fn)
       {
         try
         {
            env.log("saveKbProfile begin ......");
            tools.saveKBProfileforJson(paths,data,vpid,fn);
            env.log('App Services saveKBProfile  Save ',paths);
         }catch(e)
         {
            env.log('App Services saveKBProfile  Save error',paths+'   '+e);
            fn('ng');
         }
       }

      
       this.getDecryData = function(fn)
       {
          try
          {
             var tpath = $("#txt_import_file").val();
             tools.readImportProfile(tpath,function(data)
             {
                 if(data !="") {  tools.DecryptData(data,fn); }
                 else          {  fn(undefined);              }
             }); 
          }catch(e){env.log("getDecryData error :"+e); } 
       }

       this.getDecryKBData = function(tpath,fn)
       {
          try
          {
            tools.readImportProfile(tpath,function(data)
            {
               if(data !="") {  tools.DecryptData(data,fn); }
               else          {  fn(undefined);              }
            }); 
          }catch(e){env.log("getDecryKBData error :"+e); } 
       }
      
       this.unzipfile = function(file,callback)
       {
          try
          {
             var tpath = $("#txt_import_file").val(); 
              tools.readImportProfile(tpath,function(data)
             {
                 var fn = function(d)
                 {
                    var a= json.isJSON(d) ;
                    callback(a); 
                 };
                 tools.DecryptData(data,fn);
             });   
          }catch(e){  callback(e); env.log("unzipfile error "+e);}
       }
 
       this.unzipkbfile = function(file,callback)
       {
          try
          {
            var tpath = file[0];
            tools.readImportProfile(tpath,function(data)
            {
              // console.log(data);
              var fn = function(d)
              {
                var a= json.isJSON(d) ;
                callback(a); 
              };
              tools.DecryptData(data,fn);
            });   
          }catch(e){  callback(e); env.log("unzipfile error "+e);}
       }

       this.autoCheckUpdate = function()
       {
          env.log("App services autoCheckUpdate [CheckUpdateFromNet ] begin.")
          var Obj = { Type:funcVar.Types.System, SN:null,Func:funcVar.Names.CheckUpdateFromNet,   Param: null };
          GeniusProtocol.RunFunction(Obj,function (data)  { env.log("App services autoCheckUpdate [CheckUpdateFromNet ] end."+data) });
       }

       this.InitGetSNProfiles = function(sn,fn)
       {
           var Obj = { Type:funcVar.Types.Mouse, SN:sn,Func:funcVar.Names.InitGetSNProfiles,   Param: undefined };
           GeniusProtocol.RunFunction(Obj,function (data)  { fn(data); env.log("App services InitGetSNProfiles end."+data) });
       }

      this.ScanFileNameForWin = function(fpath,fname,callback)
      {
         tools.ScanFileNameForWin(fpath,fname,callback);
      }
      
      this.ScanFileNameForMac = function(fpath,fname,callback)
      {
         tools.ScanFileNameForMac(fpath,fname,callback);
      }

      this.CheckKbKeyIsLock = function(keyName){
        // var lockKeyList=["F12"];
        var lockKeyList=[];
        if(lockKeyList.indexOf(keyName) == -1){
          return true;
        }
        return false;
      }
 
      this.CheckKbKeyIsLockInUi = function(keyName){
        // var lockKeyList=["F12"];
        var lockKeyList=["F11","F12"];
        if(lockKeyList.indexOf(keyName) == -1){
          return true;
        }
        return false;
      }

      this.checkIsSpecialKey = function(keyName){
        //param 1:Genius Website 2:Feedback
        var specialFunkey = [
          "Feedback",
          "用戶回饋",
          "用户回馈"
        ];
        if(keyName == "Genius Website"){
          specialFunkey = ["Genius Website"];
          return specialFunkey;
        }else{
          if(specialFunkey.indexOf(keyName) != -1){
            return specialFunkey;
          }
        }
        specialFunkey =[];
        return specialFunkey;
        
      }

      this.CheckDevicesJsonVer = function(deviceId,callback){
        var KboardJson = {};
        var modelJson  = {};
        var customItem = [];
        // var result = false;
        var isHasKey = false;
        var isSystemSet = true;
        var changeKeyName = "";
        var templateList = path.join(env.deviceSettingsHome,deviceId+'.json');

        if (fs.existsSync(templateList)){
          var data = fs.readFileSync(templateList, 'utf-8');
          KboardJson = JSON.parse(data);
        }
        var templateList2 = path.join(env.deviceSettingsHome,'NewModelsFactory.json');
        if (fs.existsSync(templateList2)){
          var data = fs.readFileSync(templateList2, 'utf-8');
          modelJson = JSON.parse(data).Keyboard;
        }
        // if(KboardJson.FristPlugin==0){
        //   KboardJson.profiles[0].keyMapping.forEach(function(item){
        //     // if(_this.CheckKbKeyIsLock(item.keyName)){
        //       item.FuncGroup        = "";
        //       item.FuncTypeName     = ""; 
        //       item.GroupID          = 0;
        //       item.FuncTypeID       = 0;
        //       item.ProgramAction    = 0;
        //       item.Param1           = "";
        //       item.Param2           = "";
        //       item.MacroMode        = 0;
        //       item.MacroRepeatTimes = 0;
        //       item.IsCustom         = [];
        //     // }
        //   });
        //   var kbProfilePath =path.join(env.deviceSettingsHome,deviceId+'.json');
        //   fs.writeFileSync(kbProfilePath, JSON.stringify(KboardJson, null, "\t"));
        // }else{
          if((KboardJson.jsonVersion != modelJson.jsonVersion)||(typeof KboardJson.jsonVersion == "undefind")){
            //Change country to check is need ini data
            if(KboardJson.jsonVersion.substring(0,2)!=modelJson.jsonVersion.substring(0,2)){
                KboardJson.profiles[0].keyMapping.forEach(function(item){
                  // if(item.GroupID!=0 && item.keyName!="F12"){
                  if(item.GroupID!=0 && _this.CheckKbKeyIsLock(item.keyName)){
                    isSystemSet = false;
                  }
                });
                if(!isSystemSet){
                  KboardJson.profiles[0].keyMapping = modelJson.profiles[0].keyMapping;
                }else{
                  for(var i in modelJson.profiles[0].keyMapping){
                    if(KboardJson.FristPlugin==0){
                      KboardJson.profiles[0].keyMapping.forEach(function(item){
                        // if(_this.CheckKbKeyIsLock(item.keyName)){
                          item.FuncGroup        = "";
                          item.FuncTypeName     = ""; 
                          item.GroupID          = 0;
                          item.FuncTypeID       = 0;
                          item.ProgramAction    = 0;
                          item.Param1           = "";
                          item.Param2           = "";
                          item.MacroMode        = 0;
                          item.MacroRepeatTimes = 0;
                          item.IsCustom         = [];
                        // }
                      });
                    }else{
                      if(!_this.CheckKbKeyIsLock(modelJson.profiles[0].keyMapping[i].keyName)){
                        KboardJson.profiles[0].keyMapping[i].FuncGroup = modelJson.profiles[0].keyMapping[i].FuncGroup;
                        KboardJson.profiles[0].keyMapping[i].FuncTypeName = modelJson.profiles[0].keyMapping[i].FuncTypeName;
                        KboardJson.profiles[0].keyMapping[i].GroupID = modelJson.profiles[0].keyMapping[i].GroupID;
                        KboardJson.profiles[0].keyMapping[i].FuncTypeID = modelJson.profiles[0].keyMapping[i].FuncTypeID;
                        KboardJson.profiles[0].keyMapping[i].ProgramAction = modelJson.profiles[0].keyMapping[i].ProgramAction;
                        KboardJson.profiles[0].keyMapping[i].Param1 = modelJson.profiles[0].keyMapping[i].Param1;
                        KboardJson.profiles[0].keyMapping[i].Param2 = modelJson.profiles[0].keyMapping[i].Param2;
                        KboardJson.profiles[0].keyMapping[i].MacroMode = modelJson.profiles[0].keyMapping[i].MacroMode;
                        KboardJson.profiles[0].keyMapping[i].MacroRepeatTimes = modelJson.profiles[0].keyMapping[i].MacroRepeatTimes;
                      }
                    }
                  }
                }  
            }

            for(var key1 in modelJson){
              isHasKey = false;
              changeKeyName = "";
              for(var key2 in KboardJson){

                if(key1 == key2){
                  isHasKey = true;
                  changeKeyName = key2;

                  switch(key2){
                    case "templateList":
                      //upback custom item
                      KboardJson[key2].forEach(function(e){
                        e.categoryFunc.forEach(function(i){
                          var objItem = {
                              "FuncGroup": "",
                              "item":{
                                "FuncTypeName": "",
                                "FuncTypeID":"",
                                "ProgramAction": 0,
                                "Param1": "",
                                "Param2": "",
                                "MacroMode": 0,
                                "MacroRepeatTimes": 0
                              }
                          };
                          //if(i.Param2 == "icon-uniF0AC"||i.Param2 == "icon-credit-card"){
                          if((typeof i.IsCustom !="undefined") && i.IsCustom){  
                            objItem.FuncGroup             = e.FuncGroup;
                            objItem.item.FuncTypeName     = i.FuncTypeName; 
                            objItem.item.FuncTypeID       = i.FuncTypeID;
                            objItem.item.ProgramAction    = i.ProgramAction;
                            objItem.item.Param1           = i.Param1;
                            objItem.item.Param2           = i.Param2;
                            objItem.item.MacroMode        = i.MacroMode;
                            objItem.item.MacroRepeatTimes = i.MacroRepeatTimes;
                            objItem.item.IsCustom         = i.IsCustom;
                            customItem.push(objItem);
                          }
                        });
                      });
                      KboardJson[key2] = modelJson[key1];
                      // add upback item
                      var funcId = 0;
                      customItem.forEach(function(e){
                        KboardJson[key2].forEach(function(i){
                          if(i.FuncGroup == e.FuncGroup){
                            var length = i.categoryFunc.length;
                            if(length!=0){
                              funcId = i.categoryFunc[length-1].FuncTypeID; 
                            }
                            e.item.FuncTypeID = funcId + 1;
                            i.categoryFunc.push(e.item);
                          }

                        });
                      });
                      KboardJson[key2].forEach(function(k){
                        KboardJson.profiles.forEach(function(e){
                          e.keyMapping.forEach(function(i){
                            k.categoryFunc.forEach(function(s){
                              var tempSpecialKeyList= _this.checkIsSpecialKey(i.FuncTypeName);
                              if(tempSpecialKeyList.length < 1){
                                if(i.FuncTypeName == s.FuncTypeName && i.FuncGroup == k.FuncGroup){
                                  i.FuncGroup = k.FuncGroup;
                                  i.GroupID = k.GroupID;
                                  i.FuncTypeID = s.FuncTypeID;
                                  i.ProgramAction = s.ProgramAction;
                                  i.Param1 = s.Param1;
                                  i.Param2 = s.Param2;
                                  i.MacroMode = s.MacroMode;
                                  i.MacroRepeatTimes = s.MacroRepeatTimes;
                                }
                              }else{
                                modelJson.templateList[7].categoryFunc.forEach(function(modelFunc){
                                  if(tempSpecialKeyList.indexOf(modelFunc.FuncTypeName) != -1){
                                    i.FuncTypeName = modelFunc.FuncTypeName;
                                    i.FuncTypeID = modelFunc.FuncTypeID;
                                    // i.Param1 = modelFunc.Param1;
                                    // i.Param2 = modelFunc.Param2;
                                  }
                                });
                              }
                            });
                          });
                        });
                      });  
                    break;
                  }
                }
              }
              if(!isHasKey){//參數添加
                KboardJson[key1] = modelJson[key1];
                isHasKey = false;
                changeKeyName = "";
              }
            }
            KboardJson.jsonVersion = modelJson.jsonVersion;
            var kbProfile =path.join(env.deviceSettingsHome,deviceId+'.json');
            fs.writeFileSync(kbProfile, JSON.stringify(KboardJson, null, "\t"));
            // result = true;
          } 
        // }
        callback();   
      }

  }) ;
