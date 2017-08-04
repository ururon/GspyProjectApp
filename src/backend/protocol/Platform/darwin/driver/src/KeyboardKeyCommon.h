/*
 *  KeyCommon.h
 *  GCompanyTablet
 *
 *  Created by System Administrator on 9/30/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 * @Author:roger
 * @Time:2008.9.30
 * 
 *  N:number S:string k:string
 */

#ifndef _KEYBOARD_KEY_COMMON_H
#define _KEYBOARD_KEY_COMMON_H

#pragma mark -
#pragma mark Driver Data

#define N_MAX_KEY           12



enum
{
    KB_BT_COMMAND_INDEX_CLOSE_APP = -1,
    KB_BT_COMMAND_INDEX_SCROLL_ZOOM_MODE = 1,
};





typedef struct
{
    bool bAppActive;
    int  nDebugLevel;
    
}sIDKbDriverInfo;

typedef struct
{
    int     nDeviceID;
    UInt32  nLocationID;


}sIDKbUsbInterfaceInfo;



typedef struct
{
    SInt32  FilterKeyIndex;
    UInt8   FilterModifyKey; //combo key, 0 = no combo key.
    UInt8   FilterKeyCode;
    bool    runKey;
    bool    findFilterKey;

    
}sKeyboardKeyStatus;


typedef struct
{
    SInt32  FilterKeyIndex;
    UInt8   FilterModifyKey;  //combo key, 0 = no combo key.
    UInt8   FilterKeyCode;    //key code
    //UInt8   FunctionName;
    //SInt32  Param1;
    //SInt32  Param2;
    
}sKeyboardKeyInfo;

typedef struct
{
    UInt8 KeyboardRepeatLevel;
    sKeyboardKeyInfo KeyData[N_MAX_KEY];
    
}sIDKbDeviceInfo;



typedef struct
{
    SInt32 FilterKeyIndex;
    UInt8  FilterModifyKey;  //combo key, 0 = no combo key.
    UInt8  FilterKeyCode;    //key code
    SInt32 KeyValue1;
    SInt32 KeyValue2;
    
}sIDKeyboardKeyMsg;


typedef struct
{
    UInt8       bmRequestType;
    UInt8       bRequest;
    UInt16      wValue;
    UInt16      wIndex;
    UInt16      wLength;
    
    UInt32      wLenDone; 
    
    UInt8       intData[64];

}sKbUSBCommandInfo;


#endif //_KEY_COMMON_H