declare var System;
import {Injectable,EventEmitter} from '@angular/core';  
import {AppProtocol} from '../protocol/protocolfunction';

@Injectable() 
export class protocolService {

    AppPro:any;
    constructor(){
        this.AppPro = new AppProtocol();
    }

    public RunSetFunction(obj:any){
      var _this  = this;
      console.log('protocolService');
      return new Promise(function (resolve, reject) {           
         return _this.AppPro.RunSetFunction(obj,(err,data)=>{
             resolve(err);
         });
      }); 
   }

   public RunGetFunction(obj:any){
      var _this  = this;
      return new Promise(function (resolve, reject) { 
           return _this.AppPro.RunGetFunction(obj,(data)=>{
             resolve(data);
         });
      });    
   }

}