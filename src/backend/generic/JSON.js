'use strict';
var fs = require('fs');
var path = require('path');  

var VerificationCode = 
{
    // 0x2000 : 合法的JSON文件 0x2001:非法的JSON文件 0x2002:文件损坏
    // 0x2003 :DPI值超出范围 0x2004:按键不匹配
    Legitimate  : 0x2000,  
    IllegalJson : 0x2001,
    injure      : 0x2002,
    dpiOver     : 0x2003,
    btnMismatch : 0x2004 
};

var Templet = [
                  {
                    "ID" : "0x0458_0x0177_0",
                    "MaxDPI":"5000",
                    "ModelName":"Scorpion M2-630",
                    "Keys":[ {"KeyNum":"1","OnOff":"off"},{"KeyNum":"2","OnOff":"off"},{"KeyNum":"3","OnOff":"off"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"off"}]
                  },
                  {
                    "ID" : "0x0458_0x0178_0",
                    "MaxDPI":"5000",
                    "ModelName":"Scorpion M6-400",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"on"}]
                  },
                  {
                    "ID" : "0x0458_0x0178_1",
                    "MaxDPI":"5000",
                    "ModelName":"Scorpion M6-400",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"on"}]
                  },
                  {
                    "ID" : "0x0458_0x0179_0",
                    "MaxDPI":"5000",
                    "ModelName":"Scorpion M6-600",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  },
                  {
                    "ID" : "0x0458_0x0179_1",
                    "MaxDPI":"5000",
                    "ModelName":"Scorpion M6-600",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  },
                  {
                    "ID" : "0x0458_0x017A_0",
                    "MaxDPI":"5000",
                    "ModelName":"Scorpion M6-610",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  },
                  {
                    "ID" : "0x0458_0x017A_1",
                    "MaxDPI":"5000",
                    "ModelName":"Scorpion M6-610",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  },
                  {
                    "ID" : "0x0458_0x017B_0",
                    "MaxDPI":"5000",
                    "ModelName":"Scorpion M8-400",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  },
                  {
                    "ID" : "0x0458_0x017B_1",
                    "MaxDPI":"5000",
                    "ModelName":"Scorpion M8-400",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  }, 
                  {
                    "ID" : "0x0458_0x017C_0",
                    "MaxDPI":"6400",
                    "ModelName":"Scorpion M8-600",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  },
                  {
                    "ID" : "0x0458_0x017C_1",
                    "MaxDPI":"6400",
                    "ModelName":"Scorpion M8-600",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  }, 
                  {
                    "ID" : "0x0458_0x017D_0",
                    "MaxDPI":"8200",
                    "ModelName":"Scorpion M8-610",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  },
                  {
                    "ID" : "0x0458_0x017D_1",
                    "MaxDPI":"8200",
                    "ModelName":"Scorpion M8-610",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  },
                  {
                    "ID" : "0x0458_0x017E_0",
                    "MaxDPI":"8200",
                    "ModelName":"Scorpion M8-800",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  },
                  {
                    "ID" : "0x0458_0x017E_1",
                    "MaxDPI":"8200",
                    "ModelName":"Scorpion M8-800",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  },
                   {
                    "ID" : "0x0458_0x0191_0",
                    "MaxDPI":"8200",
                    "ModelName":"Scorpion M8-800",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  },
                   {
                    "ID" : "0x0458_0x0192_0",
                    "MaxDPI":"8200",
                    "ModelName":"Scorpion M8-800",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"on"},{"KeyNum":"5","OnOff":"on"},{"KeyNum":"6","OnOff":"on"}]
                  },
                  {
                    "ID" : "0x0458_0x0185_1",
                    "MaxDPI":"800",
                    "ModelName":"NX-7000",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"off"}]
                  },
                  {
                    "ID" : "0x0458_0x0185_2",
                    "MaxDPI":"1200",
                    "ModelName":"Micro Traveler 9000R",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"off"}]
                  },
                  {
                    "ID" : "0x0458_0x0185_3",
                    "MaxDPI":"1600",
                    "ModelName":"NX-7010",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"off"}]
                  },
                  {
                    "ID" : "0x0458_0x0185_4",
                    "MaxDPI":"1600",
                    "ModelName":"NX-7015",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"off"}]
                  },
                  {
                    "ID" : "0x0458_0x0185_5",
                    "MaxDPI":"1200",
                    "ModelName":"NX-7005",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"off"}]
                  },
                  {
                    "ID" : "0x0458_0x0185_6",
                    "MaxDPI":"1600",
                    "ModelName":"NX-7006",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"off"}]
                  },
                  {
                    "ID" : "0x0458_0x0188_0",
                    "MaxDPI":"3200",
                    "ModelName":"Ammox X1-400",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"off"}]
                  },
                  {
                    "ID" : "0x0458_0x0181_0",
                    "MaxDPI":"800",
                    "ModelName":"NX-7000",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"off"}]
                  },
                   {
                    "ID" : "0x0458_0x0184_0",
                    "MaxDPI":"1200",
                     "ModelName":"NX-9000BT",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"off"}]
                  },
                  {
                    "ID" : "0x04F3_0x0235_0",
                    "MaxDPI":"1000",
                    "ModelName":"Wire Mouse",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"off"}]
                  },
                  {
                    "ID" : "0x0458_0x0190_1",
                    "MaxDPI":"1000",
                    "ModelName":"Wire Mouse",
                    "Keys":[ {"KeyNum":"1","OnOff":"on"},{"KeyNum":"2","OnOff":"on"},{"KeyNum":"3","OnOff":"on"},{"KeyNum":"4","OnOff":"off"},{"KeyNum":"5","OnOff":"off"},{"KeyNum":"6","OnOff":"off"}]
                  }  
              ]

var KbtTemplet = [
                  {
                    "ID" : "0x0C45_0x7603_0",
                    "deviceType" : "Keyboard",
                    "ModelName":"K220",
                    "keyMapping":[]
                  },
                  {
                    "ID" : "0x0458_0x018E_1",
                    "deviceType" : "Keyboard",
                    "ModelName":"BK-Keyboard",
                    "keyMapping":[]
                  },
                  {
                    "ID" : "0x04D9_0x1603_1",
                    "deviceType" : "Keyboard",
                    "ModelName":"KB-135",
                    "keyMapping":[]
                  }
              ]

function isJSON(obj)
{   
    var b = VerificationCode.Legitimate ;
    try
    {
       var o = JSON.parse(obj); 
       b = VerificationCode.Legitimate;
    }
    catch(e)
    {
        b = VerificationCode.IllegalJson;
    }
    return b ;
}

function VerifJsonParaBut(obj,pvid)
{        
    var t = JSON.parse(obj); if (!t.hasOwnProperty("ID")){ return VerificationCode.injure; }
    var iNo = 0; 
    for (var i = 0; i< Templet.length; i++) {  if (Templet[i].ID === pvid ){  iNo = i ; break ; } } 
    for(var j = 0; j <Templet[iNo].Keys.length; j++ )
    {
       if(Templet[iNo].Keys[j].OnOff !== t.Keys[j].OnOff ) { return VerificationCode.btnMismatch; } 
    }
    return  VerificationCode.Legitimate;  
}

function VerifJsonParaDPI(obj,pvid)
{        
    var t = JSON.parse(obj); if (!t.hasOwnProperty("ID")){ return VerificationCode.injure; }
    var iNo = 0; 
    for (var i = 0; i< Templet.length; i++) {  if (Templet[i].ID === pvid ){  iNo = i ; break ; } }
    if (Number(Templet[iNo].MaxDPI) < Number(t.MaxDPI)) { return VerificationCode.dpiOver; }    
    return  VerificationCode.Legitimate;  
}

exports.KbtTemplet = KbtTemplet;
exports.Templet = Templet;
exports.isJSON = isJSON;
exports.VerifJsonParaBut = VerifJsonParaBut;
exports.VerifJsonParaDPI = VerifJsonParaDPI;