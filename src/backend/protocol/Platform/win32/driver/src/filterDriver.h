#ifndef _DRIVER_H
#define _DRIVER_H 1        

                 
                   //
//  Mouse event paket
//

typedef struct _KB_EVENT
{
        USHORT  UnitId;
        USHORT  Dummy;
        HANDLE  hEvent;
} KB_EVENT, *PKB_EVENT;

typedef struct _EVENTPACK
{
        USHORT  UnitId;
        USHORT  nSwapFlag;
        HANDLE  hEvent;
} EVENTPACK, *PEVENTPACK;


typedef struct _CALLER_EVENT_PACKET 
{
    union
    {    
        struct _KB_EVENT      Device;
        struct _EVENTPACK     Mouse;                
    };
} CALLER_EVENT_PACKET, *PCALLER_EVENT_PACKET;    






//-------------------------------------------------------------------
// Mouse definition
//         
//
//-------------------------------------------------------------------

//lParam
#define     MOUSE_LEFT_BUTTON_DOWN      0x1
#define     MOUSE_LEFT_BUTTON_UP        0x2
#define     MOUSE_RIGHT_BUTTON_DOWN     0x4
#define     MOUSE_RIGHT_BUTTON_UP       0x8
#define     MOUSE_MIDDLE_BUTTON_DOWN    0x10		// 3th button
#define     MOUSE_MIDDLE_BUTTON_UP      0x20		// 3th button
#define     MOUSE_4BUTTON_DOWN			0x40		// 3rd button
#define     MOUSE_4BUTTON_UP			0x80		// 3rd button
#define     MOUSE_5BUTTON_DOWN			0x100
#define     MOUSE_5BUTTON_UP			0x200

#define     MOUSE_Z_MOVEMENT            0x400
#define     MOUSE_XY_MOVEMENT           0x800
#define     MOUSE_RI_MOVEMENT           0x1000	// 3B + Roller
#define     MOUSE_MI_MOVEMENT           0x2000	// 4B + Roller
#define     MOUSE_PI_MOVEMENT           0x4000	// 5B + Roller
#define     MOUSE_LOW_POWER             0x8000


#define MOUSE_R_MOVE				     0x80000000			// for horizontal scroll 


#define     MOUSE_6BUTTON_DOWN			0x10000    
#define     MOUSE_6BUTTON_UP			0x20000    
#define     MOUSE_7BUTTON_DOWN			0x40000    
#define     MOUSE_7BUTTON_UP			0x80000
#define     MOUSE_8BUTTON_DOWN			0x100000
#define		MOUSE_8BUTTON_UP			0x200000
#define		MOUSE_9BUTTON_DOWN			0x400000
#define		MOUSE_9BUTTON_UP			0x800000
#define		MOUSE_10BUTTON_DOWN			0x1000000
#define		MOUSE_10BUTTON_UP			0x2000000

#define     MOUSE_SIXI_MOVEMENT         0x4000000
#define     SMOUSE_EVENI_MOVEMENT       0x8000000

#define		MOUSE_8Z_MOVE         0x10000000   // 8 button + wheel 
#define		MOUSE_9Z_MOVE         0x20000000   // 9 button + wheel
#define		MOUSE_10Z_MOVE        0x40000000   // 10 button + wheel 

#define USB_6_NEWBUTTON_DOWN          0x00000100
#define USB_7_NEWBUTTON_DOWN          0x00000200
#define USB_8_NEWBUTTON_DOWN          0x00000400
#define USB_9_NEWBUTTON_DOWN          0x00000800
#define USB_10_NEWBUTTON_DOWN         0x00001000
#define USB_11_NEWBUTTON_DOWN         0x00002000
#define USB_12_NEWBUTTON_DOWN         0x00004000
#define USB_13_NEWBUTTON_DOWN         0x00008000


#define MOUSE_BUTTON_6_DOWN     0x10000
#define MOUSE_BUTTON_6_UP       0x20000
#define MOUSE_BUTTON_7_DOWN     0x40000
#define MOUSE_BUTTON_7_UP       0x80000

//reserve space for special spec.
#define MOUSE_BUTTON_8_DOWN     0x100000
#define MOUSE_BUTTON_8_UP       0x200000
#define MOUSE_BUTTON_9_DOWN     0x400000
#define MOUSE_BUTTON_9_UP       0x800000
#define MOUSE_BUTTON_10_DOWN    0x1000000
#define MOUSE_BUTTON_10_UP      0x2000000

//lParam1
#define     MOUSE_11BUTTON_DOWN   0x1		
#define     MOUSE_11BUTTON_UP     0x2
#define     MOUSE_12BUTTON_DOWN   0x4
#define     MOUSE_12BUTTON_UP     0x8
#define     MOUSE_13BUTTON_DOWN   0x10
#define     MOUSE_13BUTTON_UP     0x20
#define     MOUSE_14BUTTON_DOWN   0x40
#define     MOUSE_14BUTTON_UP     0x80
#define     MOUSE_15BUTTON_DOWN   0x100
#define     MOUSE_15BUTTON_UP     0x200

#define     MOUSE_11Z_MOVE        0x400
#define     MOUSE_12Z_MOVE        0x800
#define     MOUSE_13Z_MOVE        0x1000
#define     MOUSE_14Z_MOVE        0x2000
#define     MOUSE_15Z_MOVE        0x4000

#define		MOUSE_DPI_SWITCH	  0x80000000

///////////add by Justin Yeh 2008/06/29 //////////////////////////////////////////
//
// ReportID 7 Function
//
#define DPIFUNC			0x0100//0x00000100
#define MODE_FUNC		0x0200//0x00000200

#define BATTERY_FUNC	0x2300//0x00002300
#define WIN8_MODE       0x2200//0x00002200//   	//// Add by Steven  20120307

#define CAM_MOUSE_MODE       0x2a00//0x00002a00//   //add by stven  20130503


////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//by Anderson
typedef struct _SBATTERY
{
	int  	DataType;		
	BOOL	bEnable;	
	int		TotalLevel;
	int		WarningLevel;
	int		FullLevel;
	BOOL	bShowIcon;
	BOOL    bShowUI;
	BOOL	bShowUIValue;
	BOOL	bShowUIStatus;
	BOOL	bShowTrayValue;
	BOOL	bDeviceExist;

} SBATTERY, *PSBATTERY;  

//
//Mouse button action paket
//
typedef struct _MOUSE_EVENT_PACKET
{
    ULONG   wParam;
    ULONG   lParam;
    LONG    xMovement;
    LONG    yMovement;
	ULONG   lParam1;
	ULONG   nDPIValue;	// DpiValue, add by ricky 2005/6/16
	SBATTERY	BatteryInfo; // Battery ,add by 20110706
} MOUSE_EVENT_PACKET, *PMOUSE_EVENT_PACKET;  



 
 ///////////////////////////////////////////////////////////////////////////////////////////////////////
//  new UI support                       
//
//Mouse button action paket
//

                 
typedef struct _REPORT_DATA
{
    ULONG       MsgType;                // 0 - Driver Consumer or SYS key been 
                                        // 1 - Driver report message to UI    
										
    USHORT      VendorID;
    USHORT      ProductID;   
    ULONG       DUMMY[6];               // total 32 bytes    
}REPORT_DATA, *PREPORT_DATA;

typedef struct _MOUSE_DATA
{
    ULONG       MsgType;                // 0 - Driver Consumer or SYS key been 
                                        // 1 - Driver report message to UI    
										// 0x80000000	- PS2,  0 - USB
    USHORT      VendorID;
    USHORT      ProductID;   
    MOUSE_EVENT_PACKET  MouseDataPacket;
}MOUSE_DATA, *PMOUSE_DATA;
           
               
                 
typedef struct _CONSUMER_SYS_EVENT_PACKET 
{
    union
    {    
        struct _MOUSE_DATA      Mouse;
        struct _REPORT_DATA     Report;                
    };
} CONSUMER_SYS_EVENT_PACKET, *PCONSUMER_SYS_EVENT_PACKET;                                 
// end mouse info
              
typedef struct _MOUSESETTINGS 
{   
	//union
    //{
	//	USHORT   UnitId[2];       
    //};
    USHORT   VendorID;
    USHORT   ProductID;   

    ULONG    nSwapFlag;
    ULONG    nLeftRightButtonFunc;
    ULONG    nMiddleButtonFunc;   
    ULONG    nFourButtonFunc;
    ULONG    nFiveButtonFunc;
	ULONG    nSixButtonFunc;
	ULONG    nSevenButtonFunc;
	ULONG    nEightButtonFunc;
	ULONG    nNineButtonFunc;
	ULONG    nTenButtonFunc;
	ULONG    nElevenButtonFunc;

    LONG     nOrientationSin;
    LONG     nOrientationCos;    
    ULONG    bLoop;
	ULONG	 bScroll;
    
} MOUSESETTINGS, *PMOUSESETTINGS;

typedef struct _COMMON_REQUEST
{
    USHORT  VendorID;
    USHORT  ProductID;
    USHORT  Function;
    UCHAR   Request;
    USHORT  Value;
    USHORT  Index;
    ULONG   TransferBufferLength;
    ULONG   RetLength;
	USHORT  InOut;
    UCHAR   TransferBuffer[1];

} COMMON_REQUEST, *PCOMMON_REQUEST;

//MsgType
#define  MSGTYPE_HARDWARE_DATA       0
#define  MSGTYYPE_SYSTEM_REPORT      1
                  
// messge                      
//#define MESSAGE_ADD_DEVICE          1
//#define MESSAGE_REMOVE_DEVICE       2

                      
#define DEVICE_TYPE_MOUSE        0x00000001
#define DEVICE_TYPE_KEYBOARD     0x00000002
#define DEVICE_TYPE_JOYSTICK     0x00000004

#define DEVICE_TYPE_USB          0x10000000             
#define DEVICE_TYPE_PS2          0x20000000     


//default mouse button funciton
#define     DEFAULT_LOOP               FALSE
#define     DEFAULT_BROWSING_DIR       1
#define     DEFAULT_BROWSING_SPEED    -1
#define     DEFAULT_BROWSING_HSPEED   -1 
#define     DEFAULT_ORIENTATION_Y     -1
#define     DEFAULT_ORIENTATION_X      0
#define     DEFAULT_BROWSINGLINE       3
#define     DEFAULT_HBROWSINGLINE      3
#define     DEFAULT_NONEBUTTON		   0
#define     DEFAULT_LEFTRIGHTBUTTON    2
#define     DEFAULT_MIDDLEBUTTON       3
#define     DEFAULT_FOURBUTTON         4 
#define     DEFAULT_FIVEBUTTON         5
#define     DEFAULT_SIXBUTTON          6   
#define     DEFAULT_SEVENBUTTON        7   
#define     DEFAULT_EIGHTBUTTON        8   
#define     DEFAULT_NINEBUTTON         9   
#define     DEFAULT_TENBUTTON          10
#define     DEFAULT_ELEVENBUTTON	   11
#define     DEFAULT_TWELVEBUTTON	   12
#define     DEFAULT_THIRTEENBUTTON	   13
#define     DEFAULT_FOURTEENBUTTON	   14
#define     DEFAULT_FIFTEENBUTTON	   15

#define URB_FUNCTION_CLASS_INTERFACE    0x001B           
#define FILE_ANY_ACCESS                 0         
#define FILE_DEVICE_UNKNOWN             0x00000022              
#define METHOD_BUFFERED                 0
                    
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define URB_FUNCTION_VENDOR_DEVICE            0x0017

#define USBD_TRANSFER_DIRECTION_OUT           0
#define USBD_TRANSFER_DIRECTION_IN            1

#define BULKUSB_IOCTL_INDEX				300  //KYE SmartGenius Mouse driver by 20150707 Anderson modify.
     
// for PS2 device control
#define IOCTL_READ_REGISTRY				        CTL_CODE(FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_REGISTRY			        CTL_CODE(FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VENDOR_CONTROL_WRITE              CTL_CODE(FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+3, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                   
#define IOCTL_VENDOR_CONTROL_READ               CTL_CODE(FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+4, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                                                                    
#define IOCTL_GET_DESCRIPTOR                    CTL_CODE(FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+5, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                                                             
#define IOCTL_COMMON_REQUEST                    CTL_CODE(FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+6, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                                                                                 

///////////////////////////////////////                                                 
// for device PNP                                                    
#define IOCTL_DEVICE_CHANGE_REGISTER_EVENT	    CTL_CODE(FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+31, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                   
#define IOCTL_DEVICE_CHANGE_UNREGISTER_EVENT	CTL_CODE(FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+32, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                                                                   
#define IOCTL_GET_DEVICE				        CTL_CODE(FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+33, METHOD_BUFFERED, FILE_ANY_ACCESS)                                                                           

///////////////////////////////////////////////////////////////////////////////////////////////////////
// for mouse data             
#define IOCTL_MOUSE_GETINFO                     CTL_CODE( FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+100, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_MOUSE_SETFUNCTION                 CTL_CODE( FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+101, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_MOUSE_REGISTER_EVENT              CTL_CODE( FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+102, METHOD_BUFFERED, FILE_ANY_ACCESS )
//#define IOCTL_MOUSE_UNREGISTER_EVENT            CTL_CODE( FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+103, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_MOUSE_PEEK                        CTL_CODE( FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+104, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_MOUSE_RECORD_APP                  CTL_CODE( FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+105, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_MOUSE_DERECORD_APP                CTL_CODE( FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+106, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_EMOUSE_SETFUNCTION                CTL_CODE( FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+107, METHOD_BUFFERED, FILE_ANY_ACCESS )      // Andy Nien, support Web Scroll
#define	IOCTL_MOUSE_SETDATA                     CTL_CODE( FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+108, METHOD_BUFFERED, FILE_ANY_ACCESS )      
#define	IOCTL_MOUSE_GETDATA                     CTL_CODE( FILE_DEVICE_UNKNOWN, BULKUSB_IOCTL_INDEX+109, METHOD_BUFFERED, FILE_ANY_ACCESS )      


#endif





