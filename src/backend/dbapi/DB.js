/*!---------------------------------------------------------
* Copyright (C) Genius Corporation. All rights reserved.
*--------------------------------------------------------*/

'use strict';

var __extends = this.__extends || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    __.prototype = b.prototype;
    d.prototype = new __();
};

var DBstore = require('nedb');
var path = require('path');


var DB = (function () 
{   
    var _this; 
    function DB() {
      	_this = this;
        _this.db = {};   
    } 
 
    DB.prototype.getMax = function(tbname,onFind){
        var  db = _this.getDB(tbname);
        db.find({}).sort({id:-1}).limit(1).exec(function (err, docs) {
            if (docs !== undefined && docs.constructor === Array && docs.length > 0) {
                onFind(err, docs[0].id);
            }
            else{
                onFind(err, docs['id']);
            } 
        });
    }

    DB.prototype.queryCmd = function(tbname,m,callback) 
    {
        try
        {
            var  db = _this.getDB(tbname);
            db.find(m, function (err, docs) {
                if(err)
                    callback(err);
                else
                    callback(docs);
            });
        }catch(e){
            console.log(e);
        }
    };

    DB.prototype.insertCmd = function(tbname,m,callback) 
    {
        var  db = _this.getDB(tbname); var newIndex = 0;
        _this.getMax(tbname,function(err,maxValue){
            if(maxValue == undefined) {
                newIndex = 1 ;
            }else{
                newIndex = maxValue+1;
            }
            if(m["id"] ==null) 
               m["id"] = newIndex;                
            db.insert(m, function(err, docs) { 
                db.persistence.compactDatafile();
                callback(err); 
           }); 
        });
    };  

    DB.prototype.updateCmd = function(tbname,om,nm,callback) 
    {
        var  db = _this.getDB(tbname);
        db.update(om,{$set: nm}, { upsert: true,multi: true }, function (err, numReplaced) {
            db.persistence.compactDatafile();
            callback(err);
        });
    }; 

    DB.prototype.updateDataCmd = function(tbname,om,nm,callback) 
    {
        var  db = _this.getDB(tbname);
        db.update(om,{$push: nm},{multi: true}, function (err, numReplaced) {
            db.persistence.compactDatafile();
            callback(err);
        });
    };

    DB.prototype.deleteCmd = function(tbname,m,callback) 
    {
        var  db = _this.getDB(tbname);
        db.remove(m, { multi: true }, function (err, numRemoved) {
            db.persistence.compactDatafile();
            callback(err);
        });
    };  

    DB.prototype.ensureIndex = function(tbname,_fieldName,callback){
        var  db = _this.getDB(tbname);
        db.ensureIndex({ fieldName:_fieldName,unique:true,sparse: true}, function (err) {
            callback(err);
        });
    } 

    DB.prototype.getDB = function(tbname){
        if(_this.db[tbname]){
            _this.db[tbname].loadDatabase();
            return _this.db[tbname]
        }else 
        { 
                 var dbPath = path.resolve(__dirname, '../../data/'+tbname+".db");     
                 var thisDb = new DBstore({ filename: dbPath, corruptAlertThreshold: 1 });
                 thisDb.loadDatabase();
                 _this.db[tbname] = thisDb ;
                 return thisDb; 
        }
    };    



   DB.prototype.db = undefined; 

   return DB;
})();

exports.DB = DB;