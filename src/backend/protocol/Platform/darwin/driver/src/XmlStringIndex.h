/*
 *  XmlStringIndex.h
 *  FunctionXmlRealWriteTest
 *
 *  Created by System Administrator on 5/6/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
 
 
//-------------------------------------------------------------//






//-------------------------------------------------------------// 
#pragma mark -
#pragma mark button

#define BT_TYPE_DATA_INFO	-1
#define BT_DATA_INFO_INDEX_END	0


#define BT_TYPE_INDEX_COMMAND	-1

enum{
	BT_COMMAND_INDEX_CLOSE_APP = -1,
	BT_COMMAND_INDEX_SCROLL_ZOOM_MODE = 1,
    BT_COMMAND_INDEX_REPORTID_5 = 2,

};



enum{

	SINDEX_BT_TYPE_NONE = 0,
		
	SINDEX_BT_TYPE_MOUSE = 100,
	SINDEX_BT_TYPE_MOUSE_LED,
	SINDEX_BT_TYPE_MOUSE_EXTENT_T1,
	SINDEX_BT_TYPE_MOUSE_DPI,
	SINDEX_BT_TYPE_MOUSE_CONSUMER,
	SINDEX_BT_TYPE_MOUSE_V_SCROLL_UP,
	SINDEX_BT_TYPE_MOUSE_V_SCROLL_DOWN,
	SINDEX_BT_TYPE_MOUSE_BATTERY,
	
	SINDEX_BT_TYPE_KEYBOARD_CONSUMER = 200,
	SINDEX_BT_TYPE_KEYBOARD_SYSTEMKEY,
	SINDEX_BT_TYPE_KEYBOARD_FUNCTIONKEY,

};

/*
enum{
	SINDEX_BT_TYPE_SUBTYPE_NONE = 0,
	SINDEX_BT_TYPE_SUBTYPE_MOUSE_TURBO_SCROLL,
	SINDEX_BT_TYPE_SUBTYPE_MOUSE_QUICK_SCROLL,
	SINDEX_BT_TYPE_SUBTYPE_MOUSE_FLYING_SCROLL,
	SINDEX_BT_TYPE_SUBTYPE_MOUSE_HARDWARE_MACRO_KEY,
};
*/


//-------------------------------------------------------------//
#pragma mark -
#pragma mark Function Group
enum{
	SINDEX_FUNC_GP_NONE = 0,
	SINDEX_FUNC_GP_GCOMPANY,
	SINDEX_FUNC_GP_DEFAULT,
	SINDEX_FUNC_GP_CLICK,
	SINDEX_FUNC_GP_SCROLL,
	//SINDEX_FUNC_GP_SYSTEM,
	//SINDEX_FUNC_GP_OFFICE,
	//SINDEX_FUNC_GP_MEDIA,
	//SINDEX_FUNC_GP_BROWSER,
	//SINDEX_FUNC_GP_PAINT,
	//SINDEX_FUNC_GP_PEN,
	//SINDEX_FUNC_GP_MOUSE,
};


/*
//-------------------------------------------------------------//
#pragma mark -
#pragma mark Function Name

#pragma mark -
#pragma mark Function 

enum{
	SINDEX_FUNC_NM_ALLNONE = 0,
	SINDEX_FUNC_NM_POST_BT_MSG,
	SINDEX_FUNC_NM_SHOW_MOUSE_DPI_TO_UI,
	SINDEX_FUNC_NM_SHOW_MOUSE_BATTERY_TO_UI,
};


//------------------------------------------------------------//

#pragma mark -
#pragma mark Function  Default
enum{
	SINDEX_FUNC_NM_DEFAULT=1,
	SINDEX_FUNC_NM_NONE,
};
*/



//-------------------------------------------------------------//
#pragma mark -
#pragma mark Function Click Group

enum{
	SINDEX_FUNC_NM_BUTTON_1 = 1,
	SINDEX_FUNC_NM_BUTTON_2,
	SINDEX_FUNC_NM_BUTTON_3,
	SINDEX_FUNC_NM_BUTTON_4,
	SINDEX_FUNC_NM_BUTTON_5,
	SINDEX_FUNC_NM_DOUBLE_CLICK,
	SINDEX_FUNC_NM_TRIPLE_CLICK,
	SINDEX_FUNC_NM_CLICK_LOCK,
    SINDEX_FUNC_NM_SCROLL_NONE,
};


//-------------------------------------------------------------//
#pragma mark -
#pragma mark Function Click Scroll

enum{
	SINDEX_FUNC_NM_SCROLL_UP = 1,
	SINDEX_FUNC_NM_SCROLL_DOWN,
	SINDEX_FUNC_NM_SCROLL_LEFT,
	SINDEX_FUNC_NM_SCROLL_RIGHT,
	SINDEX_FUNC_NM_SCROLL_UP_DOWN,
	SINDEX_FUNC_NM_SCROLL_LEFT_RIGHT,
	SINDEX_FUNC_NM_SCROLL_8WAY,
	SINDEX_FUNC_NM_VOLUME_UP_DOWN,
	SINDEX_FUNC_NM_LAYER_SWITCH_UP_DOWN,
	SINDEX_FUNC_NM_BRUSH_SIZE_LARGE_SMALL,
	SINDEX_FUNC_NM_CANVAS_ROTATION_LEFT_RIGHT,
	SINDEX_FUNC_NM_ZOOM_IN_OUT,
	
};


/*
//-------------------------------------------------------------//
#pragma mark -
#pragma mark Function  System
enum{
	SINDEX_FUNC_NM_PAGE_UP = 1,
	SINDEX_FUNC_NM_PAGE_DOWN,
	SINDEX_FUNC_NM_BACK,
	SINDEX_FUNC_NM_FORWARD,
	SINDEX_FUNC_NM_QUIT,
	SINDEX_FUNC_NM_FILE_NEW,
	SINDEX_FUNC_NM_FILE_OPEN,
	SINDEX_FUNC_NM_MOVE_TRASH,
	SINDEX_FUNC_NM_EJECT_IMG,
	SINDEX_FUNC_NM_CLOSE_WINDOW,
	SINDEX_FUNC_NM_CAPTURE_SCREEN,
	SINDEX_FUNC_NM_CAPTURE_RECT_SECTION,
	SINDEX_FUNC_NM_SWITCH_APP,
	SINDEX_FUNC_NM_SWITCH_APP_DOWN,
	SINDEX_FUNC_NM_SWITCH_APP_UP,
	SINDEX_FUNC_NM_BRIGHT_UP,
	SINDEX_FUNC_NM_BRIGHT_DOWN,
	SINDEX_FUNC_NM_SHUTDOWN,
	SINDEX_FUNC_NM_RESTART,
	SINDEX_FUNC_NM_SLEEP,
	SINDEX_FUNC_NM_OPEN_ITEM,
	SINDEX_FUNC_NM_KEY_STROKE,
	SINDEX_FUNC_NM_ZOOM_IN,
	SINDEX_FUNC_NM_ZOOM_OUT,
	SINDEX_FUNC_NM_FINDER,
};
//-------------------------------------------------------------//
#pragma mark -
#pragma mark Function  Office
enum{
	SINDEX_FUNC_NM_UNDO = 1,
	SINDEX_FUNC_NM_CUT,
	SINDEX_FUNC_NM_COPY,
	SINDEX_FUNC_NM_PASTE,
	SINDEX_FUNC_NM_SAVE,
	SINDEX_FUNC_NM_PRINT,
};
//-------------------------------------------------------------//
#pragma mark -
#pragma mark Function  Media
enum{
	SINDEX_FUNC_NM_VOLUME_UP = 1,
	SINDEX_FUNC_NM_VOLUME_DOWN,
	SINDEX_FUNC_NM_MUTE,
	SINDEX_FUNC_NM_EJECT_CD,
	SINDEX_FUNC_NM_PREVIOUS_TRACK,
	SINDEX_FUNC_NM_REWIND,
	SINDEX_FUNC_NM_PLAY,
	SINDEX_FUNC_NM_STOP,
	SINDEX_FUNC_NM_FAST_FORWARD,
	SINDEX_FUNC_NM_NEXT_TRACK,
};

//-------------------------------------------------------------//
#pragma mark -
#pragma mark Function  Browser
enum{
	SINDEX_FUNC_NM_SMART_SEARCH=1,
	SINDEX_FUNC_NM_ADD_FAVORITE,
	SINDEX_FUNC_NM_LAUNCH_URL,
};


//-------------------------------------------------------------//
#pragma mark -
#pragma mark Function  paint

enum{
	SINDEX_FUNC_NM_LAYER_SWITCH_UP=1,
	SINDEX_FUNC_NM_LAYER_SWITCH_DOWN,
	SINDEX_FUNC_NM_BRUSH_SIZE_LARGE,
	SINDEX_FUNC_NM_BRUSH_SIZE_SMALL,
	SINDEX_FUNC_NM_CANVAS_ROTATION_LEFT,
	SINDEX_FUNC_NM_CANVAS_ROTATION_RIGHT,
};

//-------------------------------------------------------------//
#pragma mark -
#pragma mark Function  pen
enum{
	SINDEX_FUNC_NM_PRESSURE_HOLD=1,
	SINDEX_FUNC_NM_PEN_ERASER,
	SINDEX_FUNC_NM_MODE_SWITCH,
};
//-------------------------------------------------------------//
#pragma mark -
#pragma mark Function  Mouse
enum{

	SINDEX_FUNC_NM_SHOW_DPI=1,
	SINDEX_FUNC_NM_HARDWARE_KEYSTROKE,
	SINDEX_FUNC_NM_SCROLL_ZOOM_MODE,
};


enum{
	
	BATT_CHARGING = 1,
	BATT_NOCHARGE,
	BATT_NOSEGMENT,
	BATT_OFFLINE,
	BATT_BATTERYLOW,
	
	BATT_MIDDLE_VALUE = 29,
	BATT_HIGH_VALUE	= 69,
};
 
 */
