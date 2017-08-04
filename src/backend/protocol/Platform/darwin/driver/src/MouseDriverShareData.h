/*
 *  DriverShareData.h
 *  GCompanyTablet
 *
 *  Created by System Administrator on 9/30/08.
 *  Copyright © 2015 KYE System Corp. All rights reserved.
 *
 * @Author:Ricky Chou
 * @Time:2015.07.27
 * 
 *	share the userClient & DeviceInterface communicate data
 */
 
 
#ifndef _DRIVER_SHAREDATA_H
#define _DRIVER_SHAREDATA_H

#define MAX_DEVICE	 8



//#define kMouseDriversClassName "Agama_MouseDriver"
 
 // User client method dispatch selectors.
enum {
	
	kGetUsbDeviceInfo,
	kSetDriverInfo,         //kIOUCScalarIStructI,
    kSetDeviceData,         //kIOUCScalarIStructI,
    kSetScrollZoomMode,
    kSetUSBCommand,
    kGetUSBCommand,
	kNumberOfMouseMethods   //Must be last
};



#endif //_DRIVER_SHAREDATA_H

