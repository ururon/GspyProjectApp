declare var System;
let remote = System._nodeRequire('electron').remote; 
let evtVar = System._nodeRequire('./backend/generic/EventVariates');
let funcVar = System._nodeRequire('./backend/generic/FunctionVariates');

export class AppProtocol {

    protocol:any;

    constructor(){
       this.protocol = remote.getGlobal('GeniusProtocol');
    }

     public RunSetFunction(obj:any,callback:any){
         console.log('RunSetFunction')
        if (obj.Type === funcVar.Types.System ){
            this.RunSetFunctionSystem(obj,callback);
        } else if (obj.Type === funcVar.Types.Mouse ){
            
        } else if (obj.Type === funcVar.Types.Keyboard){
           
        }
    }
   
    public RunGetFunction(obj:any,callback:any){
        if (obj.Type === funcVar.Types.System ){
           this.RunGetFunctionSystem(obj,callback);
        } else if (obj.Type === funcVar.Types.Mouse ){
           
        } else if (obj.Type === funcVar.Types.Keyboard){
           
        }
    }  

    private checkGetSystemFunction(fnName:string){
        var fnlist =['abctest'];
        return fnlist.lastIndexOf(fnName) >=0;
    }

    private checkSetSystemFunction(fname:string){
        var fnlist =['abctest'];
        return fnlist.lastIndexOf(fname) >=0;
    }

    private RunGetFunctionSystem(obj:any,callback:any){
       if(this.checkGetSystemFunction(obj.Func))
       {   
           var Obj1 = { Type:funcVar.Types.System, SN:null,Func:obj.Func, Param : obj.Param } ;
           this.protocol.RunFunction(Obj1,(data)=> { callback(data); });
       }else{
           callback("functionNameError");
       }
    }

    private RunSetFunctionSystem(obj:any,callback:any) {
        console.log(JSON.stringify(obj));
        if(this.checkSetSystemFunction(obj.Func)){
            var Obj1 = { Type:funcVar.Types.System, SN:null,Func:obj.Func, Param : obj.Param } ;
            this.protocol.RunFunction(Obj1, (err,data) => { callback(err); });
        }else{
             callback("functionNameError");
        } 
    } 

}