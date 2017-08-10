/*!---------------------------------------------------------
* Copyright (C) Genius Corporation. All rights reserved.
*--------------------------------------------------------*/
'use strict';

var FuncName = {
	//设备初始化
    InitDevice : "InitDevice",
    
    abctest:"abctest" 
};

var FuncType = {
    System : 0x01,
    Mouse : 0x02,
    Keyboard :0x03
};

exports.Names = FuncName;
exports.Types = FuncType;