/*
 *  KeyCommon.h
 *  GCompanyTablet
 *
 *  Created by System Administrator on 9/30/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 * @Author:roger
 * @Time:2008.9.30
 * 
 *	N:number S:string k:string
 */

#ifndef _KEY_COMMON_H
#define _KEY_COMMON_H

#pragma mark -
#pragma mark Driver Data

#define N_MAX_BUTTON		32
#define kNormalCursorSpeedLevel 4
#define kNormalDoubleSpeedLevel 4


typedef struct
{
	bool bAppActive;
	int  nDebugLevel;
    
}sIDDriverInfo;

typedef struct
{
    int     nDeviceID;
    UInt32  nLocationID;


}sIDUsbInterfaceInfo;

typedef struct
{
	SInt16 ButtonType ;
	SInt32 ButtonIndex;
	UInt8 FunctionGroup;
	UInt8 FunctionName; 
	SInt32 Param1; 
	SInt32 Param2;
		    
}sIDButtonInfo;

typedef struct
{
	UInt8 CoursorSpeedLevel;
	sIDButtonInfo ButtonData[N_MAX_BUTTON];
			    
}sIDDeviceInfo;


typedef struct
{
    
    UInt8       bmRequestType;
    UInt8       bRequest;
    UInt16      wValue;
    UInt16      wIndex;
    UInt16      wLength;
    
    UInt32      wLenDone; 
    
    UInt8       intData[64];

}sUSBCommandInfo;


typedef struct
{
	SInt16 ButtonType;
	SInt32 ButtonIndex;
	SInt32 ButtonValue1;
	SInt32 ButtonValue2;
    
    UInt8 ReportIDData[8];
		    
}sIDButtonMsg;

typedef struct 
{	
	UInt8 shootingMode;
	UInt8 modifity_Key;
	UInt8 key_Code;
	UInt8 min_time;
	UInt8 sec_time;
	UInt8 msec_time;
}sCarboKeyStroke;


//add by Ricky 2011/07/03
typedef struct 
{
    int Type;
    int Value;
}batteryStatus;





#endif //_KEY_COMMON_H