#ifndef _GENIUS_DRIVER_H
#define _GENIUS_DRIVER_H

#include <shellapi.h>
#include <winioctl.h>
#include <usbioctl.h>

#define IOCTL_INDEX						            0x0000
#define IOCTL_SIMULATE_MACRO_KEY					CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_INDEX+100, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_SIMULATE_MODIFIER_KEY	                CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_INDEX+101, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                 
#define IOCTL_SIMULATE_MOUSE		                CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_INDEX+102, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                    
#define IOCTL_SIMULATE_CONSUMER_CONTROL_KEY         CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_INDEX+104, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                    
#define IOCTL_SIMULATE_SYSTEM_CONTROL_KEY           CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_INDEX+106, METHOD_BUFFERED, FILE_ANY_ACCESS) 
#define IOCTL_SIMULATE_PRESS_KEY					CTL_CODE(FILE_DEVICE_UNKNOWN,IOCTL_INDEX+107, METHOD_BUFFERED, FILE_ANY_ACCESS) 

//////////////////////////////////////////////////////////////////////
#define VIRTUAL_DEVICE_TYPE_KEYBOARD            0x06090105      //  usage page + usage id  
#define VIRTUAL_DEVICE_TYPE_MOUSE               0x02090105      //
//#define VIRTUAL_DEVICE_TYPE_TABLET              0x02090D05      //
//#define VIRTUAL_DEVICE_TYPE_CONSUMER_CONTROL    0x01090c05      //
//#define VIRTUAL_DEVICE_TYPE_SYSTEM_CONTROL      0x80090105      //

//----------------------------------------------
// Modifier Key
//----------------------------------------------
#define VIRTUAL_KEY_MODIFY_NULL		0
#define VIRTUAL_CONTROL_KEY			0x1
#define VIRTUAL_SHIFT_KEY			0x2
#define VIRTUAL_ALT_KEY				0x4
#define VIRTUAL_WIN_KEY				0x8

//----------------------------------------------
// Button Click
//----------------------------------------------
#define VIRTUAL_BUTTON_NULL		    0
#define VIRTUAL_LEFT_BUTTON			0x1
#define VIRTUAL_RIGHT_BUTTON		0x2
#define VIRTUAL_MIDDLE_BUTTON		0x4

typedef struct _SIMULATE_REQ_PACKET
{
    ULONG   Type;               // VIRTUAL_DEVICE_TYPE_KEYBOARD           
                                // VIRTUAL_DEVICE_TYPE_MOUSE               
                                // VIRTUAL_DEVICE_TYPE_CONSUMER_CONTROL    
                                // VIRTUAL_DEVICE_TYPE_SYSTEM_CONTROL      

    ULONG   KeyNumber;			// item no of Data                            
    UCHAR	Data[256][3];       // data send to system
}SIMULATE_REQ, *PSIMULATE_PACKET;

typedef struct _SIMULATE_KEYPRESS_PACKET
{
    ULONG   Type;               // VIRTUAL_DEVICE_TYPE_KEYBOARD                                                  
    UCHAR	isMakeKey;				
	UCHAR	KeyBuffer[8];
}SIMULATE_KEYPRESS_PACKET, *PSIMULATE_KEYPRESS_PACKET;

#endif //_GENIUS_DRIVER_H 