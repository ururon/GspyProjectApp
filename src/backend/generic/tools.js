 var req = require('ajax-request');  
 var env = require('./env'); 
 var path = require('path');
 var fs = require('fs');
 var AdmZip = require('adm-zip');
 var os = require('os');
 var zlib = require('zlib'); 
 var jsn = require('./JSON'); 
 var scanFS = require('scan-fs' ).create();
 var cp = require('child_process');
 var ffs = require('final-fs');


function getAjaxJsonData(websiteurl,callback)
{ 
	 req({
		  url: websiteurl,
		  method: 'GET', 
		  json: true
		 }, function(err, res, body) 
		 {  
		  	callback(err,body);
		});
};

exports.getAjaxJsonData = getAjaxJsonData;


function encryptionData(data,callback)
{
   var input = "";
   zlib.deflate(data, function(err, buffer)
   {
        if (!err) { input = buffer.toString('base64');callback(input);}
         else  { callback(undefined) }
    })
};
exports.encryptionData = encryptionData;

function  DecryptData(data,callback)
{
	var input = "";var buffer = new Buffer(data, 'base64');
    zlib.unzip(buffer, function(err, buffer)
    {
        if (!err) {    input= buffer.toString();callback(input);    }
        else      {     callback(undefined);          	            }
    });
};

exports.DecryptData = DecryptData;

function readImportProfile(tpath,callback)
{
	var osVer ="";   var bry = new Array(); 
	if (env.isWindows) { osVer ="WIN" ; }
  else  if (env.isMac)     { osVer ="MAC" ; }
  else  if (env.isLinux)   { osVer ="LIN" ; }
 
	
    if (osVer !== "WIN") {   bry[0] = tpath.substring(tpath.lastIndexOf("/")+1,tpath.length);  }
    else  {    bry = tpath.split("\\");   }  

    var iNo = bry.length; 
    var filename =bry[iNo-1];
    var ary = new Array(); 
    ary =filename.split("."); 
    var AdmZip = require('adm-zip');
    var zip = new AdmZip(tpath); 
    zip.extractAllTo(os.tmpdir(), true);
    var bexists = fs.existsSync(path.join(os.tmpdir(),ary[0]+'_a.gns'));
    if (!bexists) { callback(""); }
    else
    {
    	var data = fs.readFileSync(path.join(os.tmpdir(),ary[0]+'_a.gns'), 'utf-8');
    	callback(data);
    }
}

exports.readImportProfile = readImportProfile;


function  saveProfileforJson(paths,data,vpid,callback)
{
  try
  {
    var tmpa = {}; var ary = new Array(); ary =paths.split("."); var bry = new Array();
    bry =paths.split("\\");var i = bry.length; var filename =bry[i-1];
    for( var i=0;i<jsn.Templet.length;i++)
    {
        if(jsn.Templet[i].ID === vpid )
        {
             tmpa.ID = vpid ; 
             tmpa.deviceType = "Mouse";
             tmpa.MaxDPI = jsn.Templet[i].MaxDPI;
             tmpa.Keys = jsn.Templet[i].Keys; 
             tmpa.profiles = [];  
             tmpa.profiles = data;
            zlib.deflate(JSON.stringify(tmpa), function(err, buffer)
            {
               if (!err)
              {
                  var  input = buffer.toString('base64');fs.writeFileSync(ary[0]+'_a.gns',input);var zip = new AdmZip();
                  zip.addLocalFile(ary[0]+'_a.gns'); zip.writeZip(paths); fs.unlinkSync(ary[0]+'_a.gns') ; callback('ok');
              }
              else {  callback('ng'); }
            }) ;
            break;
        }
    }
  }catch(e){
    env.log("saveProfileforJson error:"+e);
  }  
}

exports.saveProfileforJson = saveProfileforJson;

function  saveKBProfileforJson(paths,data,vpid,callback)
{
    var tmpa = {};
    var ary = new Array(); 
    ary =paths.split(".");
    var bry = new Array();
    bry =paths.split("\\");
    var i = bry.length; 
    var filename =bry[i-1];
    for( var i=0;i<jsn.KbtTemplet.length;i++)
    {
            // if(jsn.KbtTemplet[i].ID === vpid )
            // {
                 tmpa.ID = vpid ;
                 tmpa.deviceType = "Keyboard"; 
                 tmpa.profiles = [];  
                 tmpa.profiles = data;
                zlib.deflate(JSON.stringify(tmpa), function(err, buffer)
                {
                   if (!err)
                  {
                      var  input = buffer.toString('base64');
                      fs.writeFileSync(ary[0]+'_a.gns',input);
                      var zip = new AdmZip();
                      zip.addLocalFile(ary[0]+'_a.gns'); 
                      zip.writeZip(paths); 
                      fs.unlinkSync(ary[0]+'_a.gns') ; callback('ok');
                  }
                  else {  callback('ng'); }
                }) ;
                break;
            // }
    }
}

exports.saveKBProfileforJson = saveKBProfileforJson;

function getMouseZoomStatus(deviceId)
{
     var  keyType = "" ;  var profileData = [] ;
     var SnProfile =path.join(env.deviceSettingsHome,deviceId+'.json');
     if (fs.existsSync(SnProfile))
     {
             var data = fs.readFileSync(SnProfile, 'utf-8');  profileData =eval("("+data+")");
             keyType = profileData.profiles[0].keyMapping[2].keyType
     }
     return keyType ;
}

exports.getMouseZoomStatus = getMouseZoomStatus;

function SaveJsonFile(spath,data)
{
   var SnProfile =path.join(spath);  
   fs.writeFileSync(SnProfile, JSON.stringify(data, null, "\t")); 
}
exports.SaveJsonFile = SaveJsonFile;

function saveFavicon(data,name)
{
   var p = path.join(env.appHome,'img','icon',name); 
    fs.writeFileSync(p, data);
}

exports.saveFavicon = saveFavicon;

function checkBatteryPopupInfo(deviceId,callback)
{
   var data = fs.readFileSync(env.appSettingsPath, 'utf-8');aData =eval("("+data+")"); 
   var hasDeivceId = false ; 
   aData.SystemConfig.BatteryPopup.devices.forEach(function(d)
   {
       if(d.deviceid === deviceId )
       {
          try
          {
            var tm1 = d.UpdateTime ; hasDeivceId = true; var tmp = tm1+':00'; var tm3 = env.getCurrentDateTime();
            var tm2 = env.getUpdateTimeAdd(tmp,aData.SystemConfig.BatteryPopup.popueInterval);  
            if (tm3 > tm2 )
            {
                 d.deviceid = deviceId; d.UpdateTime =env.getCurrentDateTime(); 
                 fs.writeFileSync(env.appSettingsPath, JSON.stringify(aData, null, "\t")); 
                 callback(true);
            }
            else
            {
                 callback(false);
            }
          }catch(e){
            env.log(e);
          }  
       }
   });

   if(!hasDeivceId)
   {
      var t = {}; t.deviceid = deviceId; t.UpdateTime =env.getCurrentDateTime();
       aData.SystemConfig.BatteryPopup.devices.push(t);
       fs.writeFileSync(env.appSettingsPath, JSON.stringify(aData, null, "\t")); 
       callback(true);
   }
}
exports.checkBatteryPopupInfo = checkBatteryPopupInfo;

function ScanFileNameForWin(fpath,fname,callback)
{ 
   env.log("[MSG]",'[tools]','[ScanFileNameForWin]',"ScanFileNameForWin begin ..........."+fpath+"\\"+fname);
   var GetFileName = function(filepath)
   {
        if (filepath != "") 
        { 
            names = filepath.split("\\");
            return names[names.length - 1];
        }
   };

    var isFond = false;   
    ffs.dirFiles(fpath).then(function (filesOnly) 
    {
        filesOnly.forEach(function(f)
        {
            if (fname == f ) {  isFond = true; }
        });
        if (isFond){ callback(fpath+"\\"+fname); }
        else { callback(""); }
    }).otherwise(function (err) {
        callback("");
    }); 
    
}
exports.ScanFileNameForWin = ScanFileNameForWin;


function ScanFileNameForMac(fpath,fname,callback)
{ 
      var GetFileName = function(filepath)
      {
        if (filepath != "") 
        { 
            names = filepath.split("/");
            return names[names.length - 1];
        }
      };

    var isFond = false;   
    ffs.dirFilesapp(fpath).then(function (filesOnly) 
    {
        filesOnly.forEach(function(f)
        {
           if (fname == f ) {  isFond = true; }
        });
        if (isFond){ callback(fpath+"/"+fname); }
        else { callback(""); }
    }).otherwise(function (err) {
        callback("");
    }); 

}
exports.ScanFileNameForMac = ScanFileNameForMac;


function getMacAppIcns(appath,outpath,exeName,callback)
{
   try
   {
      var execPath;var arrgs = []; execPath = '/bin/sh';
      arrgs.push(path.join(env.appHome, 'getMacAppIcns.sh'));
      arrgs.push(appath);
      arrgs.push(outpath);
      arrgs.push(exeName);
      var child;
      child = cp.execFile(execPath, arrgs, function(err,result)
      {
           child.unref();
           callback(result);
      });
  }catch(e){
              env.log("getMacAppicns error "+e);
  }

}
exports.getMacAppIcns = getMacAppIcns;

function GetOsVer()
{
  var osVer = "" ;
 if (env.isWindows) { osVer ="WIN" ; }
 else if (env.isMac)     { osVer ="MAC" ; }
 else if (env.isLinux)   { osVer ="LIN" ; }
 return osVer ;
}

exports.GetOsVer = GetOsVer ;


function SaveProfilesToJson(deviceId,data)
{
  return new Promise(function (resolve, reject) 
  {
      var SnProfile =path.join(env.deviceSettingsHome,deviceId+'.json');  
      fs.writeFileSync(SnProfile, JSON.stringify(data, null, "\t")); 
      resolve(); 
  });
}
exports.SaveProfilesToJson = SaveProfilesToJson ;


function ReadJsonProfiles(deviceId)
{
  return new Promise(function (resolve, reject) 
  {
      var SnProfile =path.join(env.deviceSettingsHome,deviceId+'.json');  
      if(!fs.existsSync(SnProfile)){
            reject(Error(`ReadJsonProfiles Not deviceId Factory File : ${deviceId}.`));
      }
      else
      {        
         var mdata =  fs.readFileSync(SnProfile, 'utf-8');
         resolve(mdata);
      } 
  });
}

exports.ReadJsonProfiles = ReadJsonProfiles ;

function ReadJsonMacros()
{
  return new Promise(function (resolve, reject) 
  {
      var SnProfile =path.join(env.deviceSettingsHome,'userMacro.json');  
      if(!fs.existsSync(SnProfile)){
            reject(Error(`ReadJsonProfiles Not deviceId Factory File : ${SnProfile}.`));
      }
      else
      {        
         var mdata =  fs.readFileSync(SnProfile, 'utf-8');
         resolve(mdata);
      } 
  });
}

exports.ReadJsonMacros = ReadJsonMacros ;

function mergeJsonfile(profileData,macroData)
{
  return new Promise(function (resolve, reject) 
  {
      if (profileData.hasOwnProperty("templateList")) //KB profile
      { 
            profileData.templateList.forEach(function(e)
            {    
                if (e.GroupID == 4)
                {                             
                      e.categoryFunc = []; 
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
            });    
            profileData.profiles.forEach(function(kbp)
            {
                  if(!kbp.hasOwnProperty("activeProfile"))
                  {
                     kbp.activeProfile = 0;
                  }
            });
      }
      else
      {
           profileData.profiles.forEach(function(e)
          {    
            if(!e.hasOwnProperty("activeProfile")) { e.activeProfile = 0;  } 
            e.macro = macroData.macro;
             delete e.$$hashKey ;   
          });  
      }
      resolve(profileData);
  });
}
exports.mergeJsonfile = mergeJsonfile ;

 