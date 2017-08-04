#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <node_object_wrap.h>

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/ev_keymap.h>

#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#include <pthread.h>
#include <IOKit/IODataQueueShared.h>
#include <IOKit/IODataQueueClient.h>
#include <uv.h>

#include "MouseKeyCommon.h"
#include "MouseDriverShareData.h"
#include "XmlStringIndex.h"
#include "CGKeyCodeTable.h"

//Keyboard
#include "KeyboardDriverShareData.h"
#include "KeyboardKeyCommon.h"


using namespace v8;


//Mouse driver function
void OpenDriver(const v8::FunctionCallbackInfo<Value>& args);
void CloseDriver(const v8::FunctionCallbackInfo<Value>& args);
void SetMacroKey(const v8::FunctionCallbackInfo<Value>& args);


//Keyboard function
void OpenKbDriver(const v8::FunctionCallbackInfo<Value>& args);
void CloseKbDriver(const v8::FunctionCallbackInfo<Value>& args);
void SetKbFunctionAction(const v8::FunctionCallbackInfo<Value>& args);
//

void SetAllMacroKey(const v8::FunctionCallbackInfo<Value>& args);

//output message to log file
void RegisterLogMessage(const v8::FunctionCallbackInfo<Value>& args);

#define DEBUG_HEADER fprintf(stderr, "DarwinDriver [%s:%s() %d]: ", __FILE__, __FUNCTION__, __LINE__); 
#define DEBUG_FOOTER fprintf(stderr, "\n");
#define DEBUG_LOG(...) DEBUG_HEADER fprintf(stderr, __VA_ARGS__); DEBUG_FOOTER

//Mouse
#define MAX_MOUSE_QTY 20
#define FILTER_DRV_NAME "com_geniusnet_driver_DeviceFilter" //driver name

//Keyboard
#define MAX_KEYBOARD_QTY 20
#define FILTER_KBDRV_NAME "com_geniusnet_driver_KBFilter" //driver name



typedef struct _CMD_PACKET
{
	UInt16 DownUp;
	ULONG  Delay;
	int KeyCode;
}CMD_PACKET, *PCMD_PACKET;


typedef struct _KEY_MACRO_PACKET
{
	UInt16 KeyNum;
	UInt16 MacroType;
	UInt16 ButtonAction;
	UInt16 MacroMode;
	UInt16 MacroRepeatTimes;
	PCMD_PACKET Cmds[255];
}KEY_MACRO_PACKET,*PKEY_MACRO_PACKET;


typedef struct _MOUSE_MACRO_PACKET
{
	UInt16 VID;
	UInt16 PID;
	char SN[30];
	PKEY_MACRO_PACKET Keys[20];
}MOUSE_MACRO_PACKET,*PMOUSE_MACRO_PACKET;


//Keyboard
typedef struct _KEY_FUNCTION_PACKET
{
	char KeyName[16];
	UInt16 KeyIndex;
	UInt16 KeyType;
	UInt16 ProgramAction;
	UInt16 MacroMode;
	UInt16 MacroRepeatTimes;
	UInt16 Function;
	char Param1[255];

}KEY_FUNCTION_PACKET,*PKEY_FUNCTION_PACKET;

typedef struct _KEYBOARD_MACRO_PACKET
{
	UInt16 VID;
	UInt16 PID;
	char SN[30];

	PKEY_FUNCTION_PACKET Keyfunc[12];
	PKEY_MACRO_PACKET Keys[50];

}KEYBOARD_MACRO_PACKET,*PKEYBOARD_MACRO_PACKET;

typedef struct
{
    pthread_t thread;
    bool bRun;
}sThreadInfo;

typedef struct _Mouse_DeviceData
{
	io_connect_t connection;
	sThreadInfo myThread;
	PMOUSE_MACRO_PACKET mouse;
	char locPath[255];
	Persistent<Value> callback;
	bool isTransfer;
}Mouse_DeviceData,*PMouse_DeviceData;


//Keyboard
typedef struct _Keyboard_DeviceData
{
	io_connect_t connection;
	sThreadInfo myThread;
	PKEYBOARD_MACRO_PACKET keyboard;
	char locPath[255];
	Persistent<Value> callback;
	bool isTransfer;
}Keyboard_DeviceData,*PKeyboard_DeviceData;

typedef struct
{
	char buf[64];
	int len;
	PMouse_DeviceData dev;
}sTransferData,*PTransferData;

//Keyboard
typedef struct
{
	char buf[64];
	int len;
	PKeyboard_DeviceData dev;
}sKbTransferData,*PKbTransferData;


PMOUSE_MACRO_PACKET mouseMacros[MAX_MOUSE_QTY];
PMouse_DeviceData mouseDeviceData[MAX_MOUSE_QTY];
PKEY_MACRO_PACKET currRunCmds = NULL;


//Keyboard
PKEYBOARD_MACRO_PACKET keyboardMacros[MAX_KEYBOARD_QTY];
PKeyboard_DeviceData keyboardDeviceData[MAX_KEYBOARD_QTY];
PKEY_MACRO_PACKET currKbRunCmds = NULL;

//F1 - F12 keycode
static UInt8   FilterKeyCode[12] = {0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45};


static IONotificationPortRef	gNotifyPort = NULL;
static io_iterator_t			gAddedIter = 0;
Persistent<Value> LogMessageCallback;
bool isLogMessageRegistered = false;
void NotifyLogMessage(char * msg);

bool isInitDriver = false;
bool isKbInitDriver = false;

pthread_t thread_macro = NULL;
pthread_attr_t attr_macro;
bool isDrvWaitingCmd = false, isDrvWritingCmd = false, isMacroBtnBreak = true;
pthread_t kb_thread_macro = NULL;
pthread_attr_t kb_attr_macro;
bool isKbDrvWaitingCmd = false, isKbDrvWritingCmd = false, isBtnbreak = true;
UInt16 currMacroMode = 1, currMacroRepeatTimes = 1;

//Mouse
kern_return_t SetDriverInfo(io_connect_t connect, const uint32_t scalarI, const size_t structISize, const sIDDriverInfo* structI);
kern_return_t SetDeviceData(io_connect_t connect, const uint32_t scalarI, const size_t structISize, const sIDDeviceInfo* structI);
kern_return_t SetUsbCommand(io_connect_t connect, const uint32_t scalarI, const size_t structISize, const sUSBCommandInfo* structI);
kern_return_t GetUsbCommand(io_connect_t connect, const size_t structISize, const sUSBCommandInfo* structI, size_t* structOSize, sUSBCommandInfo* structO);

//keyboard
kern_return_t SetKbDriverInfo(io_connect_t connect, const uint32_t scalarI, const size_t structISize, const sIDKbDriverInfo* structI);
kern_return_t SetKbDeviceData(io_connect_t connect, const uint32_t scalarI, const size_t structISize, const sIDKbDeviceInfo* structI);


//Function
static io_connect_t get_event_driver();
static void HIDPostAuxKey(const UInt8 auxKeyCode);
static OSStatus LowRunAppleScript(const void* text, long textLength, AEDesc *resultData);
static OSStatus SimpleRunAppleScript(const char* theScript);
OSStatus SendAppleEventToSystemProcess(AEEventID EventToSend);


PMouse_DeviceData GetNewDeviceData()
{
	for (int i = 0; i < MAX_MOUSE_QTY; i++)
	{
		if(mouseDeviceData[i] == NULL)
		{
			mouseDeviceData[i] = (PMouse_DeviceData)calloc(1, sizeof(Mouse_DeviceData));
			mouseDeviceData[i]->connection = 0;
			mouseDeviceData[i]->isTransfer = false;
			mouseDeviceData[i]->mouse = NULL;
			return mouseDeviceData[i];
		}
		if (mouseDeviceData[i]->connection == 0)
			return mouseDeviceData[i];
	}
	return NULL;
}

//Keyboard
PKeyboard_DeviceData GetNewKBDeviceData()
{
	for (int i = 0; i < MAX_KEYBOARD_QTY; i++)
	{
		if(keyboardDeviceData[i] == NULL)
		{
			keyboardDeviceData[i] = (PKeyboard_DeviceData)calloc(1, sizeof(Keyboard_DeviceData));
			keyboardDeviceData[i]->connection = 0;
			keyboardDeviceData[i]->isTransfer = false;
			keyboardDeviceData[i]->keyboard = NULL;
			return keyboardDeviceData[i];
		}
		if (keyboardDeviceData[i]->connection == 0)
			return keyboardDeviceData[i];
	}
	return NULL;
}

PMOUSE_MACRO_PACKET GetNewMouse(UInt16 vid, UInt16 pid)
{
	for (int i = 0; i < MAX_MOUSE_QTY; i++)
	{
		if(mouseMacros[i] == NULL)
		{
			mouseMacros[i] = (PMOUSE_MACRO_PACKET)calloc(1, sizeof(MOUSE_MACRO_PACKET));
			mouseMacros[i]->VID = vid;
			mouseMacros[i]->PID = pid;
			return mouseMacros[i];
		}
	}
	return NULL;
}

//Keyboard
PKEYBOARD_MACRO_PACKET GetNewKeyboard(UInt16 vid, UInt16 pid)
{
	for (int i = 0; i < MAX_KEYBOARD_QTY; i++)
	{
		if(keyboardMacros[i] == NULL)
		{
			keyboardMacros[i] = (PKEYBOARD_MACRO_PACKET)calloc(1, sizeof(KEYBOARD_MACRO_PACKET));
			keyboardMacros[i]->VID = vid;
			keyboardMacros[i]->PID = pid;
			return keyboardMacros[i];
		}
	}
	return NULL;
}

PMOUSE_MACRO_PACKET GetMouseFromDevs(UInt16 vid, UInt16 pid, char * locPath)
{
	for (int i = 0; i < MAX_MOUSE_QTY; i++)
	{
		if (mouseDeviceData[i] != NULL && mouseDeviceData[i]->mouse != NULL && mouseDeviceData[i]->mouse->VID == vid && mouseDeviceData[i]->mouse->PID == pid &&
			strcmp(mouseDeviceData[i]->locPath, locPath) == 0)
			return mouseDeviceData[i]->mouse;
	}
	return NULL;
}

PKEYBOARD_MACRO_PACKET GetKeyboardFromDevs(UInt16 vid, UInt16 pid, char * locPath)
{
	for (int i = 0; i < MAX_KEYBOARD_QTY; i++)
	{
		if (keyboardDeviceData[i] != NULL && keyboardDeviceData[i]->keyboard != NULL && keyboardDeviceData[i]->keyboard->VID == vid && keyboardDeviceData[i]->keyboard->PID == pid &&
			strcmp(keyboardDeviceData[i]->locPath, locPath) == 0)
			return keyboardDeviceData[i]->keyboard;
	}
	return NULL;
}

PMouse_DeviceData GetDeviceData(UInt16 vid, UInt16 pid, char * locPath)
{
	for (int i = 0; i < MAX_MOUSE_QTY; i++)
	{
		if (mouseDeviceData[i] != NULL && mouseDeviceData[i]->mouse != NULL && mouseDeviceData[i]->mouse->VID == vid && mouseDeviceData[i]->mouse->PID == pid &&
			strcmp(mouseDeviceData[i]->locPath, locPath) == 0)
			return mouseDeviceData[i];
	}
	return NULL;
}

PKeyboard_DeviceData GetKbDeviceData(UInt16 vid, UInt16 pid, char * locPath)
{
	for (int i = 0; i < MAX_KEYBOARD_QTY; i++)
	{
		if (keyboardDeviceData[i] != NULL && keyboardDeviceData[i]->keyboard != NULL && keyboardDeviceData[i]->keyboard->VID == vid && keyboardDeviceData[i]->keyboard->PID == pid &&
			strcmp(keyboardDeviceData[i]->locPath, locPath) == 0)
			return keyboardDeviceData[i];
	}
	return NULL;
}

PKEY_MACRO_PACKET GetMouseKey(PMOUSE_MACRO_PACKET mouse, UInt16 key)
{
	if (mouse == NULL)
		return NULL;
	for (int i = 0; i < 20; i++)
	{
		if(mouse->Keys[i] == NULL)
		{
			mouse->Keys[i] = (PKEY_MACRO_PACKET)calloc(1, sizeof(KEY_MACRO_PACKET));
			mouse->Keys[i]->KeyNum = key;
			return mouse->Keys[i];
		}
		if (mouse->Keys[i]->KeyNum == key)
			return mouse->Keys[i];
	}
	return NULL;
}

PKEY_MACRO_PACKET GetKeyboardKey(PKEYBOARD_MACRO_PACKET keyboard, UInt16 key)
{
	if (keyboard == NULL)
		return NULL;
	for (int i = 0; i < 50; i++)
	{
		if(keyboard->Keys[i] == NULL)
		{
			keyboard->Keys[i] = (PKEY_MACRO_PACKET)calloc(1, sizeof(KEY_MACRO_PACKET));
			keyboard->Keys[i]->KeyNum = key;
			return keyboard->Keys[i];
		}
		if (keyboard->Keys[i]->KeyNum == key)
			return keyboard->Keys[i];
	}
	return NULL;
}


PKEY_FUNCTION_PACKET GetKeyboardFuncKey(PKEYBOARD_MACRO_PACKET Keyboard, UInt16 Index)
{
	if (Keyboard == NULL)
		return NULL;
	for (int i = 0; i < 12; i++) //12: F1~F12
	{
		if(Keyboard->Keyfunc[i] == NULL)
		{
			Keyboard->Keyfunc[i] = (PKEY_FUNCTION_PACKET)calloc(1, sizeof(KEY_FUNCTION_PACKET));
			Keyboard->Keyfunc[i]->KeyIndex = Index;
			DEBUG_LOG("GetKeyboard>> KeyIndex == NULL, return Keyboard->Keyfunc[%d]",i);
			return Keyboard->Keyfunc[i];
		}
		if (Keyboard->Keyfunc[i]->KeyIndex == Index)
		{
			DEBUG_LOG("GetKeyboard>> KeyIndex == Index, return Keyboard->Keyfunc[%d]",i);
			return Keyboard->Keyfunc[i];
		}
	}
	return NULL;
}

void FreeMouseKeyMacro(PKEY_MACRO_PACKET key)
{
	for (int i = 0; i < 255; i++)
	{
		if (key->Cmds[i] != NULL){
			free(key->Cmds[i]);
			key->Cmds[i] = NULL;
		}else{
			break;
		}
	}
}


void FreeKeyMacro(PKEY_MACRO_PACKET key)
{
	for (int i = 0; i < 255; i++)
	{
		if (key->Cmds[i] != NULL){
			free(key->Cmds[i]);
			key->Cmds[i] = NULL;
		}else{
			break;
		}
	}
}

void * MacroThreadProc(void * args)
{
	ULONG iStep = 0;
	while(true)
	{
		while(isDrvWaitingCmd)
			usleep(1000);
		isDrvWritingCmd = true;
		if (currRunCmds != NULL){
			if (currRunCmds->MacroType == 2){//Keyboard
				int iStep = 0;
				int keyboardCodes[255] = { -1 };
				
				if (currRunCmds->Cmds[0]->KeyCode & 1){
					keyboardCodes[iStep] = 0x3b;
					iStep++;
				}
				if (currRunCmds->Cmds[0]->KeyCode & 2){
					keyboardCodes[iStep] = 0x38;
					iStep++;
				}
				if (currRunCmds->Cmds[0]->KeyCode & 4){
					keyboardCodes[iStep] = 0x3a;
					iStep++;
				}
				if (currRunCmds->Cmds[0]->KeyCode & 8){
					keyboardCodes[iStep] = 0x37;
					iStep++;
				}
				for(int i = 1; i < 255; i++){
					if (currRunCmds->Cmds[i] == NULL)
						break;
					keyboardCodes[iStep] = HidToCGKeyCode[currRunCmds->Cmds[i]->KeyCode];
					iStep++;
				}
				for(int i = 0; i < iStep; i++){
					if (keyboardCodes[i] == -1)
						break;
					CGPostKeyboardEvent(0, keyboardCodes[i], true);
				}
				for(int i = 0; i < iStep; i++){
					if (keyboardCodes[i] == -1)
						break;
					CGPostKeyboardEvent(0, keyboardCodes[i], false);
				}
			}else if (currRunCmds->MacroType == 3 || currRunCmds->MacroType == 4){//Macro
				if (currRunCmds->MacroType == 4){

					if(currRunCmds->MacroMode == 1)
					{
						NotifyLogMessage("[MSG]   [darwinDriver]   [MacroMouseThreadProc]   [Start currMacroMode = 1]");

						for(int i = 0; i < 255; i++){
							if (currRunCmds->Cmds[i] == NULL)
								break;
							iStep = 0;
							while(isDrvWritingCmd){
								iStep ++;
								if(iStep >= currRunCmds->Cmds[i]->Delay)
									break;
								usleep(1000);
							}
							if (!isDrvWritingCmd)
								break;
							bool downUp = (currRunCmds->Cmds[i]->DownUp == 0x80);
							CGPostKeyboardEvent(0, currRunCmds->Cmds[i]->KeyCode, downUp);
						}
					
					}
					else if(currRunCmds->MacroMode == 2)
					{
						NotifyLogMessage("[MSG]   [darwinDriver]   [MacroMouseThreadProc]   [Start currMacroMode = 2]");

						for(int j = 0; j < currRunCmds->MacroRepeatTimes; j++)
						{
							for(int z = 0; z < 255; z++){
								if (currRunCmds->Cmds[z] == NULL)
									break;
								iStep = 0;
								while(isDrvWritingCmd){
									iStep ++;
									if(iStep >= currRunCmds->Cmds[z]->Delay)
										break;
									usleep(1000);
								}
								if (!isDrvWritingCmd)
									break;
								bool downUp = (currRunCmds->Cmds[z]->DownUp == 0x80);
								CGPostKeyboardEvent(0, currRunCmds->Cmds[z]->KeyCode, downUp);
							}
						}
					}
					else if(currRunCmds->MacroMode == 3)
					{
						NotifyLogMessage("[MSG]   [darwinDriver]   [MacroMouseThreadProc]   [Start currMacroMode = 3]");

 						while(1)
 						{
 							for(int x = 0; x < 255; x++){
								if (currRunCmds->Cmds[x] == NULL)
									break;
								iStep = 0;
								while(isDrvWritingCmd){
									iStep ++;
									if(iStep >= currRunCmds->Cmds[x]->Delay)
										break;
									usleep(1000);
								}
								if (!isDrvWritingCmd)
									break;
								bool downUp = (currRunCmds->Cmds[x]->DownUp == 0x80);
								CGPostKeyboardEvent(0, currRunCmds->Cmds[x]->KeyCode, downUp);
							}

							if(isMacroBtnBreak)
								break;
 						}
					}
				}else{//3
					int iStep = 0;
					int keyboardCodes[255] = { -1 };
					if (currRunCmds->Cmds[0]->KeyCode & 1){
						keyboardCodes[iStep] = 0x3b;
						iStep++;
					}
					if (currRunCmds->Cmds[0]->KeyCode & 2){
						keyboardCodes[iStep] = 0x38;
						iStep++;
					}
					if (currRunCmds->Cmds[0]->KeyCode & 4){
						keyboardCodes[iStep] = 0x3a;
						iStep++;
					}
					if (currRunCmds->Cmds[0]->KeyCode & 8){
						keyboardCodes[iStep] = 0x37;
						iStep++;
					}
					for(int i = 1; i < 255; i++){
						if (currRunCmds->Cmds[i] == NULL)
							break;
						keyboardCodes[iStep] = HidToCGKeyCode[currRunCmds->Cmds[i]->KeyCode];
						iStep++;
					}
					bool downUp = (currRunCmds->Cmds[0]->DownUp == 0x80);
					for(int i = 0; i < iStep; i++){
						if (keyboardCodes[i] == -1)
							break;
						CGPostKeyboardEvent(0, keyboardCodes[i], downUp);
					}
				}
			}
		}
		isDrvWritingCmd = false;
		isDrvWaitingCmd = true;
	}
}

void * MacroKbThreadProc(void * args)
{
	ULONG iStep = 0;
	while(true)
	{
		while(isKbDrvWaitingCmd)
			usleep(1000);

		if(currKbRunCmds == NULL)
			NotifyLogMessage("[MSG]   [darwinDriver]   [MacroKbThreadProc]   [currKbRunCmds = NULL]");
		// else
		// 	NotifyLogMessage("MacroKbThreadProc currKbRunCmds.");

		isKbDrvWritingCmd = true;
		if (currKbRunCmds != NULL){
			if (currKbRunCmds->MacroType == 2){//Keyboard
				int iStep = 0;
				int keyboardCodes[255] = { -1 };
				
				if (currKbRunCmds->Cmds[0]->KeyCode & 1){
					keyboardCodes[iStep] = 0x3b;
					iStep++;
				}
				if (currKbRunCmds->Cmds[0]->KeyCode & 2){
					keyboardCodes[iStep] = 0x38;
					iStep++;
				}
				if (currKbRunCmds->Cmds[0]->KeyCode & 4){
					keyboardCodes[iStep] = 0x3a;
					iStep++;
				}
				if (currKbRunCmds->Cmds[0]->KeyCode & 8){
					keyboardCodes[iStep] = 0x37;
					iStep++;
				}
				for(int i = 1; i < 255; i++){
					if (currKbRunCmds->Cmds[i] == NULL)
						break;
					keyboardCodes[iStep] = HidToCGKeyCode[currKbRunCmds->Cmds[i]->KeyCode];
					iStep++;
				}
				for(int i = 0; i < iStep; i++){
					if (keyboardCodes[i] == -1)
						break;
					CGPostKeyboardEvent(0, keyboardCodes[i], true);
				}
				for(int i = 0; i < iStep; i++){
					if (keyboardCodes[i] == -1)
						break;
					CGPostKeyboardEvent(0, keyboardCodes[i], false);
				}
			}else if (currKbRunCmds->MacroType == 3 || currKbRunCmds->MacroType == 4){//Macro
				if (currKbRunCmds->MacroType == 4){

					if(currMacroMode == 1)
					{
						NotifyLogMessage("[MSG]   [darwinDriver]   [MacroKbThreadProc]   [Start currMacroMode = 1]");

						for(int i = 0; i < 255; i++){
							if (currKbRunCmds->Cmds[i] == NULL)
								break;
							iStep = 0;
							while(isKbDrvWritingCmd){
								iStep ++;
								if(iStep >= currKbRunCmds->Cmds[i]->Delay)
									break;
								usleep(1000);
							}
							if (!isKbDrvWritingCmd)
								break;
							bool downUp = (currKbRunCmds->Cmds[i]->DownUp == 0x80);
							CGPostKeyboardEvent(0, currKbRunCmds->Cmds[i]->KeyCode, downUp);
						}
					}
					else if(currMacroMode == 2)
					{
						NotifyLogMessage("[MSG]   [darwinDriver]   [MacroKbThreadProc]   [Start currMacroMode = 2");

						for(int j = 0; j < currMacroRepeatTimes; j++)
						{
							for(int z = 0; z < 255; z++){
								if (currKbRunCmds->Cmds[z] == NULL)
									break;
								iStep = 0;
								while(isKbDrvWritingCmd){
									iStep ++;
									if(iStep >= currKbRunCmds->Cmds[z]->Delay)
										break;
									usleep(1000);
								}
								if (!isKbDrvWritingCmd)
									break;
								bool downUp = (currKbRunCmds->Cmds[z]->DownUp == 0x80);
								CGPostKeyboardEvent(0, currKbRunCmds->Cmds[z]->KeyCode, downUp);
							}
						}
					}
					else if(currMacroMode == 3)
 					{
 						NotifyLogMessage("[MSG]   [darwinDriver]   [MacroKbThreadProc]   [Start currMacroMode = 3");

 						while(!isBtnbreak)
 						{
 							for(int x = 0; x < 255; x++){
								if (currKbRunCmds->Cmds[x] == NULL)
									break;
								iStep = 0;
								while(isKbDrvWritingCmd){
									iStep ++;
									if(iStep >= currKbRunCmds->Cmds[x]->Delay)
										break;
									usleep(1000);
								}
								if (!isKbDrvWritingCmd)
									break;
								bool downUp = (currKbRunCmds->Cmds[x]->DownUp == 0x80);
								CGPostKeyboardEvent(0, currKbRunCmds->Cmds[x]->KeyCode, downUp);
							}
 						}
					}
				}else{//3
					int iStep = 0;
					int keyboardCodes[255] = { -1 };
					if (currKbRunCmds->Cmds[0]->KeyCode & 1){
						keyboardCodes[iStep] = 0x3b;
						iStep++;
					}
					if (currKbRunCmds->Cmds[0]->KeyCode & 2){
						keyboardCodes[iStep] = 0x38;
						iStep++;
					}
					if (currKbRunCmds->Cmds[0]->KeyCode & 4){
						keyboardCodes[iStep] = 0x3a;
						iStep++;
					}
					if (currKbRunCmds->Cmds[0]->KeyCode & 8){
						keyboardCodes[iStep] = 0x37;
						iStep++;
					}
					for(int i = 1; i < 255; i++){
						if (currKbRunCmds->Cmds[i] == NULL)
							break;
						keyboardCodes[iStep] = HidToCGKeyCode[currKbRunCmds->Cmds[i]->KeyCode];
						iStep++;
					}
					bool downUp = (currKbRunCmds->Cmds[0]->DownUp == 0x80);
					for(int i = 0; i < iStep; i++){
						if (keyboardCodes[i] == -1)
							break;
						CGPostKeyboardEvent(0, keyboardCodes[i], downUp);
					}
				}
			}
		}

		isKbDrvWritingCmd = false;
		isKbDrvWaitingCmd = true;
	}

}

void RunMouseKeyMacro(PMOUSE_MACRO_PACKET mouse, UInt16 keyNum)
{
	NotifyLogMessage("[MSG]   [darwinDriver]   [RunMouseKeyMacro]   [RunMouseKeyMacro");

	if(isMacroBtnBreak == false)
	{
		isMacroBtnBreak = true;
		usleep(1000);
		return;
	}

	if (thread_macro == NULL){
        pthread_attr_init(&attr_macro);
        pthread_attr_setdetachstate(&attr_macro,PTHREAD_CREATE_DETACHED);
		isDrvWaitingCmd = true;
        if(pthread_create(&thread_macro, &attr_macro, MacroThreadProc, NULL) != 0){
        	NotifyLogMessage("[ERR]   [darwinDriver]   [RunMouseKeyMacro]   [Create thread failed]");
			thread_macro = NULL;
        }
	}
	isDrvWritingCmd = false;
	while(!isDrvWaitingCmd)
		usleep(1000);
	currRunCmds = NULL;

	for(int i = 0;i < 20; i++){
		if (mouse->Keys[i]->KeyNum == keyNum){
			currRunCmds = mouse->Keys[i];
			break;
		}
	}
	isDrvWaitingCmd = false;
}

void RunKeyboardKeyMacro(PKEYBOARD_MACRO_PACKET keyboard, UInt16 keyNum, UInt16 Mode, UInt16 Repeats)
{	
	NotifyLogMessage("[MSG]   [darwinDriver]   [RunKeyboardKeyMacro]   [Start RunKeyboardKeyMacro]");

	// DEBUG_LOG("RunKeyboardKeyMacro>> Keyboard->Keyfunc[%d], Mode:%d, Repeats:%d",keyNum, Mode, Repeats);

	if (kb_thread_macro == NULL){

		NotifyLogMessage("[ERR]   [darwinDriver]   [RunKeyboardKeyMacro]   [thread kb_thread_macro == NULL]");

        pthread_attr_init(&kb_attr_macro);
        pthread_attr_setdetachstate(&kb_attr_macro,PTHREAD_CREATE_DETACHED);
		isKbDrvWaitingCmd = true;
        if(pthread_create(&kb_thread_macro, &kb_attr_macro, MacroKbThreadProc, NULL) != 0){
            NotifyLogMessage("[ERR]   [darwinDriver]   [RunKeyboardKeyMacro]   [Create thread failed]");
			thread_macro = NULL;
        }
	}

	if(isBtnbreak == false)
	{
		isBtnbreak = true;
		usleep(1000);
	}

	if(Mode == 1 || Mode == 2 || Mode == 3)
	{
		currMacroMode = Mode;
		isBtnbreak = false;
	}
	else
		currMacroMode = 1;

	currMacroRepeatTimes = Repeats;

	isKbDrvWritingCmd = false;
	while(!isKbDrvWaitingCmd)
		usleep(1000);
	currKbRunCmds = NULL;
	int i = 0;
	for(i = 0;i < 50; i++){
	 	if (keyboard->Keys[i]->KeyNum == keyNum){
	 		currKbRunCmds = keyboard->Keys[i];
	 		break;
	 	}
	}
	
	isKbDrvWaitingCmd = false;
}

//Filter Driver.
UInt16 GetButtonMacroType(PMOUSE_MACRO_PACKET mouse, UInt16 keyNum)
{
	for(int i = 0;i < 20; i++)
	{
		if (mouse->Keys[i] == NULL)
			return 0;
		if (mouse->Keys[i]->KeyNum == keyNum)
			return mouse->Keys[i]->MacroType;
	}
	return 0;
}

UInt16 GetButtonFunction(PMOUSE_MACRO_PACKET mouse, UInt16 keyNum)
{
	for(int i = 0;i < 20; i++)
	{
		if (mouse->Keys[i] == NULL)
			return 0;
		if (mouse->Keys[i]->KeyNum == keyNum)
		{
			if(mouse->Keys[i]->ButtonAction == 1)
				return 1;
			else if(mouse->Keys[i]->ButtonAction == 2)
				return 2;
			else if(mouse->Keys[i]->ButtonAction == 4)//Middle button
				return 3;
			else if(mouse->Keys[i]->ButtonAction == 8) //four button
				return 4;
			else if(mouse->Keys[i]->ButtonAction == 16) //five button
				return 5;
			else
				return -1;
		}
	}
	return -1;
}

bool CheckButtonHasMacro(PMOUSE_MACRO_PACKET mouse, UInt16 keyNum)
{
	UInt16 tp = GetButtonMacroType(mouse, keyNum);
	return (tp == 2 || tp == 3 || tp == 4);
}
void SetMouseKeyMacroDownUp(PMOUSE_MACRO_PACKET mouse, UInt16 keyNum, UInt16 downup)
{
	for(int i = 0;i < 20; i++)
	{
		if (mouse->Keys[i] == NULL)
			break;
		if (mouse->Keys[i]->KeyNum == keyNum){
			for(int n = 0;n < 255; n++){
				PCMD_PACKET c = mouse->Keys[i]->Cmds[n];
				if (c == NULL)
					break;
				c->DownUp = downup;
			}
			break;
		}
	}
}
void SetMouseFunc(UInt16 vid, UInt16 pid, char * locPath)
{
	kern_return_t   kernResult;
	
	for (int i = 0; i < MAX_MOUSE_QTY; i++)
	{
		if(mouseDeviceData[i] == NULL)
			break;
		if (mouseDeviceData[i]->connection == 0 || mouseDeviceData[i]->mouse == NULL)
			continue;
		if (mouseDeviceData[i]->mouse->VID == vid && mouseDeviceData[i]->mouse->PID == pid && strcmp(mouseDeviceData[i]->locPath, locPath) == 0)
		{
			PMOUSE_MACRO_PACKET mouse = mouseDeviceData[i]->mouse;
			if (mouse == NULL)
			{
				NotifyLogMessage("[ERR]   [darwinDriver]   [SetMouseFunc]   [mouseDeviceData->mouse NULL]");
				return;
			}
			sIDDeviceInfo DeviceInfo;
			DeviceInfo.CoursorSpeedLevel = 0;
			bool isDefMouse = true;

			DEBUG_LOG("DeviceInfo SetMouseFunc..............................");
			for(int n = 0; n < 5; n++)
			{
				UInt16 keyNum = n+1;
				UInt16 tp = GetButtonMacroType(mouse, keyNum);
				bool isDefBtn = true;//(tp == 1 || tp == 3 || tp == 5);
		        DeviceInfo.ButtonData[n].ButtonIndex = keyNum;
		        DeviceInfo.ButtonData[n].ButtonType = SINDEX_BT_TYPE_MOUSE;
				if (tp != 1 && isDefMouse)
					isDefMouse = false;
				if (isDefBtn){
					DeviceInfo.ButtonData[n].FunctionGroup = SINDEX_FUNC_GP_CLICK;
					switch(keyNum)
					{
						case 1:
							DeviceInfo.ButtonData[n].FunctionName = SINDEX_FUNC_NM_BUTTON_1;
						break;
						case 2:
						case 3:
						case 4:
						case 5:
							if(tp == 2 || tp == 3 || tp == 4)
							{
								DEBUG_LOG("DeviceInfo.ButtonData[%d].FunctionName 234 = %d", n, SINDEX_FUNC_NM_SCROLL_NONE);
								DeviceInfo.ButtonData[n].FunctionName = SINDEX_FUNC_NM_SCROLL_NONE;
							}
							else if(GetButtonFunction(mouse, keyNum) != 0xFFFF)
							{
								DEBUG_LOG("DeviceInfo.ButtonData[%d].FunctionName 15 = %d", n, GetButtonFunction(mouse, keyNum));
								DeviceInfo.ButtonData[n].FunctionName = GetButtonFunction(mouse, keyNum);
							}
							else 
							{
								DEBUG_LOG("DeviceInfo.ButtonData[%d].FunctionName default = %d", n, keyNum);
								DeviceInfo.ButtonData[n].FunctionName = keyNum;
							}
						break;
					}
				}else{
					DeviceInfo.ButtonData[n].FunctionGroup = SINDEX_FUNC_GP_NONE;
					DeviceInfo.ButtonData[n].FunctionName = 0;
				}
		        DeviceInfo.ButtonData[n].Param1 = 0;
		        DeviceInfo.ButtonData[n].Param2 = 0;
			}
			sIDDriverInfo DriverInfo;
	        DriverInfo.bAppActive = !isDefMouse;
	        DriverInfo.nDebugLevel = 0;
			kernResult = SetDriverInfo(mouseDeviceData[i]->connection, 0, sizeof(sIDDriverInfo), &DriverInfo);
	        if (kernResult != KERN_SUCCESS) {
	        	NotifyLogMessage("[ERR]   [darwinDriver]   [SetMouseFunc]   [SetDriverInfo error]");
	        }
			kernResult = SetDeviceData(mouseDeviceData[i]->connection, 0, sizeof(sIDDeviceInfo), &DeviceInfo);
    		if (kernResult == KERN_SUCCESS) {
    			NotifyLogMessage("[MSG]   [darwinDriver]   [SetMouseFunc]   [SetDeviceData was successful]");
		    }
		    else {
		    	NotifyLogMessage("[ERR]   [darwinDriver]   [SetMouseFunc]   [SetDeviceData error]");
		    }
		}
	}
}

//Set function filter keyboard key to driver
void SetKeyboardFunc(UInt16 vid, UInt16 pid, char * locPath)
{
	kern_return_t   kernResult;
	
	for (int i = 0; i < MAX_KEYBOARD_QTY; i++)
	{
		if(keyboardDeviceData[i] == NULL)
			break;

		if (keyboardDeviceData[i]->connection == 0 || keyboardDeviceData[i]->keyboard == NULL)
			continue;

		if (keyboardDeviceData[i]->keyboard->VID == vid && keyboardDeviceData[i]->keyboard->PID == pid && strcmp(keyboardDeviceData[i]->locPath, locPath) == 0)
		{
			PKEYBOARD_MACRO_PACKET keyboard = keyboardDeviceData[i]->keyboard;
			if (keyboard == NULL)
			{
				NotifyLogMessage("[ERR]   [darwinDriver]   [SetKeyboardFunc]   [keyboardDeviceData->keyboard NULL]");
				return;
			}

			sIDKbDeviceInfo DeviceInfo;
			DeviceInfo.KeyboardRepeatLevel = 5;
			bool isDefKeyboard = true;

			//F1-F12 Key function
			for(int i = 0; i < 12; i++)
			{
				PKEY_FUNCTION_PACKET key = GetKeyboardFuncKey(keyboard, i);

				if(key->ProgramAction != 0)
					DeviceInfo.KeyData[i].FilterKeyCode = FilterKeyCode[i];	
				else
					DeviceInfo.KeyData[i].FilterKeyCode = 0;	

    			DeviceInfo.KeyData[i].FilterKeyIndex = i+1;
				DeviceInfo.KeyData[i].FilterModifyKey = 0;
			}

			sIDKbDriverInfo DriverInfo;
	        DriverInfo.bAppActive = 1;
	        DriverInfo.nDebugLevel = 0;


			kernResult = SetKbDriverInfo(keyboardDeviceData[i]->connection, 0, sizeof(sIDKbDriverInfo), &DriverInfo);
	        if (kernResult != KERN_SUCCESS) {
	        	NotifyLogMessage("[ERR]   [darwinDriver]   [SetKeyboardFunc]   [kSetDriverInfo error]");
	        }
			kernResult = SetKbDeviceData(keyboardDeviceData[i]->connection, 0, sizeof(sIDKbDeviceInfo), &DeviceInfo);
    		if (kernResult == KERN_SUCCESS) {
    			NotifyLogMessage("[MSG]   [darwinDriver]   [SetKeyboardFunc]   [SetKbDeviceData success]");
		    }
		    else {
		    	NotifyLogMessage("[ERR]   [darwinDriver]   [SetKeyboardFunc]   [SetDeviceData error]");
		    }
		}
	}
}

void ZoomIn() {
  	CGPostKeyboardEvent( 0, 55, TRUE ); //command
    CGPostKeyboardEvent( 0, 24, TRUE );  //+
    CGPostKeyboardEvent( 0, 24, FALSE );
    CGPostKeyboardEvent( 0, 55, FALSE );
}
void ZoomOut() {
  	CGPostKeyboardEvent( 0, 55, TRUE ); //command
    CGPostKeyboardEvent( 0, 27, TRUE );  // -
    CGPostKeyboardEvent( 0, 27, FALSE );
    CGPostKeyboardEvent( 0, 55, FALSE );
}

//Media Function
void PreviousTrack_iTunes () {

	SimpleRunAppleScript("tell application \"iTunes\"\nprevious track\nend tell");
}

void NextTrack_iTunes () {

	SimpleRunAppleScript("tell application \"iTunes\"\nnext track\nend tell");
}

void Rewind_iTunes () {

 	SimpleRunAppleScript("tell application \"iTunes\"\nset curPos to (get player position)\nset player position to curPos - 5\nend tell");
}

void FastForward_iTunes () {

 	SimpleRunAppleScript("tell application \"iTunes\"\nset curPos to (get player position)\nset player position to curPos + 5\nend tell");
}

void Play_iTunes () {
	SimpleRunAppleScript("tell application \"iTunes\"\nif(player state is paused) or (player state is stopped) then\nplay\nelse\npause\nend if\nend tell");
}

void Stop_iTunes () {
	SimpleRunAppleScript("tell application \"iTunes\"\nstop\nend tell");
}

//Volume Control
void VolumeUp () {

	HIDPostAuxKey(NX_KEYTYPE_SOUND_UP);

}

void VolumeDown () {

	HIDPostAuxKey(NX_KEYTYPE_SOUND_DOWN);

}

void Mute () {

	HIDPostAuxKey(NX_KEYTYPE_MUTE);

}

//Open software
void OpenBrowser () {

	SimpleRunAppleScript("on DefaultBrowserID()\nset _scpt to \"export VERSIONER_PERL_PREFER_32_BIT=yes;\" & \"perl -MMac::InternetConfig -le \" & \"'print +(GetICHelper \\\"http\\\")[1]\'\"\nreturn (do shell script _scpt)\nend DefaultBrowserID\ntell application DefaultBrowserID()\nactivate\nreopen\nend tell");

}

void OpenMedia () {

	SimpleRunAppleScript("tell application \"iTunes\"\n activate\n reopen\nend tell");
}

void OpenMail () {

	SimpleRunAppleScript("tell application \"Mail\"\n activate\n reopen\nend tell");
}

void Find () {

	CGPostKeyboardEvent( 0, 55, TRUE );				
	CGPostKeyboardEvent( 0, 3, TRUE );
	CGPostKeyboardEvent( 0, 3, FALSE ); 
	CGPostKeyboardEvent( 0, 55, FALSE );
}

void Sleep () {
	SendAppleEventToSystemProcess(kAESleep);
}

void ShutDown () {
	SendAppleEventToSystemProcess(kAEShutDown);
}

void Restart () {
	SendAppleEventToSystemProcess(kAERestart);
}

//Control function
void ShowDesktop () {

	CGPostKeyboardEvent( 0, 103, TRUE );
    CGPostKeyboardEvent( 0, 103, FALSE );
}

void Minimize () {

 	CGPostKeyboardEvent( 0, 55, TRUE ); //command
    CGPostKeyboardEvent( 0, 46, TRUE );  // m
    CGPostKeyboardEvent( 0, 46, FALSE );
    CGPostKeyboardEvent( 0, 55, FALSE );
}


//Favorite

void OpenFinder () {

	SimpleRunAppleScript("tell application \"Finder\"\n activate\n reopen\nend tell");
}

void LogOut () {

 	CGPostKeyboardEvent( 0, 55, TRUE ); //command
    CGPostKeyboardEvent( 0,	56, TRUE );  // shift
    CGPostKeyboardEvent( 0, 12, TRUE );  // q
    CGPostKeyboardEvent( 0, 12, FALSE );  // q
    CGPostKeyboardEvent( 0, 56, FALSE );
    CGPostKeyboardEvent( 0, 55, FALSE );
}


void LaunchApp (char * appName)
{

	char url[256];// = "on DefaultBrowserID()\nset _scpt to \"export VERSIONER_PERL_PREFER_32_BIT=yes;\" & \"perl -MMac::InternetConfig -le \" & \"'print +(GetICHelper \\\"http\\\")[1]\'\"\nreturn (do shell script _scpt)\nend DefaultBrowserID\ntell application DefaultBrowserID()\nopen location \"http://www.yahoo.com\"\nend tell";

	snprintf(url, sizeof(url), "tell application \"%s\"\n activate\n reopen\nend tell", appName);

	//DEBUG_LOG(url);
	SimpleRunAppleScript(url);

}

void LaunchURL (char * website) {

	char url[512];// = "on DefaultBrowserID()\nset _scpt to \"export VERSIONER_PERL_PREFER_32_BIT=yes;\" & \"perl -MMac::InternetConfig -le \" & \"'print +(GetICHelper \\\"http\\\")[1]\'\"\nreturn (do shell script _scpt)\nend DefaultBrowserID\ntell application DefaultBrowserID()\nopen location \"http://www.yahoo.com\"\nend tell";

	snprintf(url, sizeof(url), "on DefaultBrowserID()\nset _scpt to \"export VERSIONER_PERL_PREFER_32_BIT=yes;\" & \"perl -MMac::InternetConfig -le \" & \"'print +(GetICHelper \\\"http\\\")[1]\'\"\nreturn (do shell script _scpt)\nend DefaultBrowserID\ntell application DefaultBrowserID()\nopen location \"%s\"\nend tell", website);
	SimpleRunAppleScript(url);

	//DEBUG_LOG(website);

}

void Facebook (char * website) {

	SimpleRunAppleScript("on DefaultBrowserID()\nset _scpt to \"export VERSIONER_PERL_PREFER_32_BIT=yes;\" & \"perl -MMac::InternetConfig -le \" & \"'print +(GetICHelper \\\"http\\\")[1]\'\"\nreturn (do shell script _scpt)\nend DefaultBrowserID\ntell application DefaultBrowserID()\nopen location \"http://www.facebook.com\"\nend tell");

}

void Youtube (char * website) {

	SimpleRunAppleScript("on DefaultBrowserID()\nset _scpt to \"export VERSIONER_PERL_PREFER_32_BIT=yes;\" & \"perl -MMac::InternetConfig -le \" & \"'print +(GetICHelper \\\"http\\\")[1]\'\"\nreturn (do shell script _scpt)\nend DefaultBrowserID\ntell application DefaultBrowserID()\nopen location \"http://www.youtube.com\"\nend tell");


}

void GoogleMap () {
	SimpleRunAppleScript("on DefaultBrowserID()\nset _scpt to \"export VERSIONER_PERL_PREFER_32_BIT=yes;\" & \"perl -MMac::InternetConfig -le \" & \"'print +(GetICHelper \\\"http\\\")[1]\'\"\nreturn (do shell script _scpt)\nend DefaultBrowserID\ntell application DefaultBrowserID()\nopen location \"http://www.googlemaps.com\"\nend tell");
}

void Amazon () {

	SimpleRunAppleScript("on DefaultBrowserID()\nset _scpt to \"export VERSIONER_PERL_PREFER_32_BIT=yes;\" & \"perl -MMac::InternetConfig -le \" & \"'print +(GetICHelper \\\"http\\\")[1]\'\"\nreturn (do shell script _scpt)\nend DefaultBrowserID\ntell application DefaultBrowserID()\nopen location \"http://www.amazon.com\"\nend tell");
}


void Twitter () {

	SimpleRunAppleScript("on DefaultBrowserID()\nset _scpt to \"export VERSIONER_PERL_PREFER_32_BIT=yes;\" & \"perl -MMac::InternetConfig -le \" & \"'print +(GetICHelper \\\"http\\\")[1]\'\"\nreturn (do shell script _scpt)\nend DefaultBrowserID\ntell application DefaultBrowserID()\nopen location \"http://www.twitter.com\"\nend tell");
}

void LinkeDin () {

	SimpleRunAppleScript("on DefaultBrowserID()\nset _scpt to \"export VERSIONER_PERL_PREFER_32_BIT=yes;\" & \"perl -MMac::InternetConfig -le \" & \"'print +(GetICHelper \\\"http\\\")[1]\'\"\nreturn (do shell script _scpt)\nend DefaultBrowserID\ntell application DefaultBrowserID()\nopen location \"http://www.linkedin.com\"\nend tell");
}



void ParserKeyboardData(KEYBOARD_MACRO_PACKET *keyboard, sIDKeyboardKeyMsg *pKeyMsgData) {

	PKEY_FUNCTION_PACKET funcKey = keyboard->Keyfunc[pKeyMsgData->FilterKeyIndex-1];

	switch (funcKey->ProgramAction)
	{
		//Launch app
		case 1:
			NotifyLogMessage("[MSG]   [darwinDriver]   [ParserKeyboardData]   [LaunchApp]");
			LaunchApp(funcKey->Param1);
			break;

		//Lunch URL
		case 2:
			NotifyLogMessage("[MSG]   [darwinDriver]   [ParserKeyboardData]   [Launch URL]");
			LaunchURL(funcKey->Param1);
			break;

		//System action
		case 3:
		{
			if(funcKey->Function == 1)
				VolumeUp();
			else if(funcKey->Function == 2)
				VolumeDown();
			else if(funcKey->Function == 3)
				Mute();
			else if(funcKey->Function == 4)
				ShutDown();
			else if(funcKey->Function == 5)
				Restart();
		
			NotifyLogMessage("[MSG]   [darwinDriver]   [ParserKeyboardData]   [System Action]");
			break;

		}

		//Run Macro key
		case 4:
		{
			//DEBUG_LOG("funcKey->MacroMode:0x%04x --- button:%4x, funcKey->MacroRepeatTimes:0x%04x",funcKey->MacroMode,pKeyMsgData->FilterKeyIndex-1, funcKey->MacroRepeatTimes);
			RunKeyboardKeyMacro(keyboard, funcKey->Function, funcKey->MacroMode, funcKey->MacroRepeatTimes);
			NotifyLogMessage("[MSG]   [darwinDriver]   [ParserKeyboardData]   [Run Macro Key]");
			break;
		}

		//Special Website
		case 5:
		{
			NotifyLogMessage("[MSG]   [darwinDriver]   [ParserKeyboardData]   [Launch Special Website]");
			LaunchURL(funcKey->Param1);
			break;
		}
	}
}


void NotifyTransferAsync(uv_work_t* req)
{
}
void NotifyTransferFinished(uv_work_t* req)
{
	PTransferData trans = static_cast<PTransferData>(req->data);
  	
	Isolate* isolate = Isolate::GetCurrent();
  	HandleScope scope(isolate);

  	Local<Value> cbVal=Local<Value>::New(isolate, trans->dev->callback);
	Local<Function> cb = Local<Function>::Cast(cbVal);
	const unsigned argc = 2;
	Local<Object> buf = node::Buffer::Copy(isolate, trans->buf, trans->len).ToLocalChecked();
	Local<Value> argv[argc] = { 
		String::NewFromUtf8(isolate,trans->dev->mouse->SN),
		buf
	};
	free(trans->dev);
	trans->dev = NULL;
	free(trans);
	trans = NULL;
	cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
}

void * MyListenForData(void * args)
{
    kern_return_t		kernResult;
    sIDButtonMsg 		ButtonMsgData;
    UInt32 				dataSize = sizeof(sIDButtonMsg);
    IODataQueueMemory * queueMappedMemory;
    mach_vm_size_t 		queueMappedMemorySize;
    mach_vm_address_t	address = 0;
    mach_vm_size_t		size = 0;
    unsigned int		msgType = 1;
    mach_port_t			recvPort;
	PMouse_DeviceData devData = (PMouse_DeviceData)args;
	
    // Allocates and returns a new mach port able to receive data available notifications from an IODataQueue.
    recvPort = IODataQueueAllocateNotificationPort();
    if (!recvPort)
    {
    	NotifyLogMessage("[ERR]   [darwinDriver]   [MyListenForData]   [returned a NULL mach_port_t]");
        kernResult= kIOReturnError;
        goto freeDeviceData;
    }
    devData->myThread.bRun = true;
    
    // IOConnectSetNotificationPort will call registerNotificationPort() inside your user client class.
    kernResult = IOConnectSetNotificationPort(devData->connection, msgType, recvPort, 0);
    if (kernResult != kIOReturnSuccess)
    {
    	NotifyLogMessage("[ERR]   [darwinDriver]   [MyListenForData]   [IOConnectSetNotificationPort returned error]");
        mach_port_destroy(mach_task_self(), recvPort);
        goto freeDeviceData;
    }
    
    // IOConnectMapMemory will call clientMemoryForType() inside your user client class.
    kernResult = IOConnectMapMemory(devData->connection, kIODefaultMemoryType, mach_task_self(), &address, &size, kIOMapAnywhere);
    if (kernResult != kIOReturnSuccess)
    {
    	NotifyLogMessage("[ERR]   [darwinDriver]   [MyListenForData]   [IOConnectMapMemory returned error]");
        mach_port_destroy(mach_task_self(), recvPort);
        goto freeDeviceData;
    }
    
    queueMappedMemory = (IODataQueueMemory *) address;
    queueMappedMemorySize = size;
    
    // IODataQueueWaitForAvailableData doesn't return until there's data on the recvPort.
    while (IODataQueueWaitForAvailableData(queueMappedMemory, recvPort) == kIOReturnSuccess)
    {
		// IODataQueueDataAvailable returns true when the shared memory contains data to dequeue.
        while (IODataQueueDataAvailable(queueMappedMemory))
        {
            // Dequeues the next available entry on the queue and copies it into the given data pointer.
            kernResult = IODataQueueDequeue(queueMappedMemory, &ButtonMsgData, &dataSize);
            if (kernResult == kIOReturnSuccess)
            {
                // The KEXT is telling us to stop listening, so exit.
                if ((ButtonMsgData.ButtonType == BT_TYPE_INDEX_COMMAND) && (ButtonMsgData.ButtonIndex == BT_COMMAND_INDEX_CLOSE_APP) && (ButtonMsgData.ButtonValue1 == -1)) {
	            	NotifyLogMessage("[MSG]   [darwinDriver]   [MyListenForData]   [IODataQueueDequeue success]");
                    goto exit;
                }
                //thread to leave
                if(!devData->myThread.bRun){
                	NotifyLogMessage("[MSG]   [darwinDriver]   [MyListenForData]   [thread run null,User Exit]");
                    goto exit;
                }
                //for spec ScrollZoomMode
                if ((ButtonMsgData.ButtonType == BT_TYPE_INDEX_COMMAND) && (ButtonMsgData.ButtonIndex == BT_COMMAND_INDEX_SCROLL_ZOOM_MODE) ) {
                    DEBUG_LOG("scrollZoom mode data");
                    continue;
                }
				PMOUSE_MACRO_PACKET mouse = devData->mouse;
				//for report id
                if ((ButtonMsgData.ButtonType == BT_TYPE_INDEX_COMMAND) && (ButtonMsgData.ButtonIndex == BT_COMMAND_INDEX_REPORTID_5) )
                {
					if (devData->isTransfer){
	                    PTransferData data = (PTransferData)calloc(1, sizeof(sTransferData));
						data->len = 8;
	                    bcopy(ButtonMsgData.ReportIDData, data->buf, data->len);
						data->dev = (PMouse_DeviceData)calloc(1, sizeof(Mouse_DeviceData));
						memcpy(data->dev, devData, sizeof(Mouse_DeviceData));
						uv_work_t* reqTranChg = new uv_work_t();
						reqTranChg->data = data;
				        uv_queue_work(uv_default_loop(), reqTranChg, NotifyTransferAsync, (uv_after_work_cb)NotifyTransferFinished);
					}
                    continue;
                }

                

                
				if(mouse != NULL && CheckButtonHasMacro(mouse, ButtonMsgData.ButtonIndex)){
					UInt16 macroType = GetButtonMacroType(mouse, ButtonMsgData.ButtonIndex);

					DEBUG_LOG("darwinDriver - MouseBtn Index=%d, BtnValue1:%d, BtnValue2:%d, macroType:%d",ButtonMsgData.ButtonIndex, ButtonMsgData.ButtonValue1, ButtonMsgData.ButtonValue2,macroType);
					if ((macroType == 4 || macroType == 2)){
						if(ButtonMsgData.ButtonValue1 == 1)
						{
							DEBUG_LOG("darwinDriver - ButtonMsgData.ButtonValue1 == 1");

							RunMouseKeyMacro(mouse, ButtonMsgData.ButtonIndex);

							isMacroBtnBreak = false;
						}
						else
							isMacroBtnBreak = true;

					}else if (macroType == 3){
						if (ButtonMsgData.ButtonValue1 == 0){
							if (ButtonMsgData.ButtonValue2 == -1){
								ZoomOut();
							}else if (ButtonMsgData.ButtonValue2 == 1){
								ZoomIn();
							}
						}
						// }else{
						// 	if (ButtonMsgData.ButtonValue1 == 1){
						// 		SetMouseKeyMacroDownUp(mouse, ButtonMsgData.ButtonIndex, 0x80);
						// 	}else{
						// 		SetMouseKeyMacroDownUp(mouse, ButtonMsgData.ButtonIndex, 0x00);
						// 	}
						// 	RunMouseKeyMacro(mouse, ButtonMsgData.ButtonIndex);
						// }
					}
				}
			}
            else
            {
            	NotifyLogMessage("[ERR]   [darwinDriver]   [MyListenForData]   [IODataQueueDequeue error returned]");
                goto exit;
            }
         }
    }
    
exit:
    // IOConnectUnmapMemory will call clientMemoryForType() inside your user client class.
    kernResult = IOConnectUnmapMemory(devData->connection, kIODefaultMemoryType, mach_task_self(), address);
    if (kernResult != kIOReturnSuccess)
    {
    	NotifyLogMessage("[ERR]   [darwinDriver]   [MyListenForData]   [IOConnectUnmapMemory returned error]");
    }
    mach_port_destroy(mach_task_self(), recvPort);
        
freeDeviceData:
    devData->myThread.bRun = false;
    devData->myThread.thread = NULL;	
    if(devData->connection){
        IOServiceClose(devData->connection);
        devData->connection = 0;
    }
 	//return kernResult;
	return NULL;
}


//Keyboard
void * MyKeyboardListenForData(void * args)
{
    kern_return_t		kernResult;
    sIDKeyboardKeyMsg   KeyboardMsgData;
    UInt32 			dataSize = sizeof(sIDKeyboardKeyMsg);

    IODataQueueMemory * queueMappedMemory;
    mach_vm_size_t 		queueMappedMemorySize;
    mach_vm_address_t	address = 0;
    mach_vm_size_t		size = 0;
    unsigned int		msgType = 1;
    mach_port_t			recvPort;
	PKeyboard_DeviceData devData = (PKeyboard_DeviceData)args;
	
    // Allocates and returns a new mach port able to receive data available notifications from an IODataQueue.
    recvPort = IODataQueueAllocateNotificationPort();
    if (!recvPort)
    {
    	NotifyLogMessage("[ERR]   [darwinDriver]   [MyKeyboardListenForData]   [IODataQueueAllocateNotificationPort returned a NULL mach_port_t]");
        kernResult= kIOReturnError;
        goto freeDeviceData;
    }
    devData->myThread.bRun = true;
    
    // IOConnectSetNotificationPort will call registerNotificationPort() inside your user client class.
    kernResult = IOConnectSetNotificationPort(devData->connection, msgType, recvPort, 0);
    if (kernResult != kIOReturnSuccess)
    {
    	NotifyLogMessage("[ERR]   [darwinDriver]   [MyKeyboardListenForData]   [IOConnectSetNotificationPort returned error]");
        mach_port_destroy(mach_task_self(), recvPort);
        goto freeDeviceData;
    }
    
    // IOConnectMapMemory will call clientMemoryForType() inside your user client class.
    kernResult = IOConnectMapMemory(devData->connection, kIODefaultMemoryType, mach_task_self(), &address, &size, kIOMapAnywhere);
    if (kernResult != kIOReturnSuccess)
    {
    	NotifyLogMessage("[ERR]   [darwinDriver]   [MyKeyboardListenForData]   [IOConnectMapMemory returned error]");
        mach_port_destroy(mach_task_self(), recvPort);
        goto freeDeviceData;
    }
    
    queueMappedMemory = (IODataQueueMemory *) address;
    queueMappedMemorySize = size;
    
    // IODataQueueWaitForAvailableData doesn't return until there's data on the recvPort.
    while (IODataQueueWaitForAvailableData(queueMappedMemory, recvPort) == kIOReturnSuccess)
    {
		// IODataQueueDataAvailable returns true when the shared memory contains data to dequeue.
        while (IODataQueueDataAvailable(queueMappedMemory))
        {
            // Dequeues the next available entry on the queue and copies it into the given data pointer.
            kernResult = IODataQueueDequeue(queueMappedMemory, &KeyboardMsgData, &dataSize);
            if (kernResult == kIOReturnSuccess)
            {
                // The KEXT is telling us to stop listening, so exit.
                if ((KeyboardMsgData.FilterKeyIndex == KB_BT_COMMAND_INDEX_CLOSE_APP) && (KeyboardMsgData.KeyValue1 == -1)) {
                	NotifyLogMessage("[MSG]   [darwinDriver]   [MyKeyboardListenForData]   [Get exit code, Keyboard Device Removed]");
                    goto exit;
                }
                //thread to leave
                if(!devData->myThread.bRun){
                	NotifyLogMessage("[MSG]   [darwinDriver]   [MyKeyboardListenForData]   [thread run null,User Exit]");
                    goto exit;
                }
                //for spec ScrollZoomMode
                // if ((KeyboardMsgData.ButtonType == BT_TYPE_INDEX_COMMAND) && (KeyboardMsgData.ButtonIndex == BT_COMMAND_INDEX_SCROLL_ZOOM_MODE) ) {
                    // DEBUG_LOG("scrollZoom mode data");
                    // continue;
                // }

				PKEYBOARD_MACRO_PACKET keyboard = devData->keyboard;

				//Keyboard key down
				if(KeyboardMsgData.KeyValue1 == 1) 
				{
					// NotifyLogMessage("KeyboardMsgData.KeyValue1 == 1....................");
					
					//if Macro runing, stop it.
					if(isKbDrvWritingCmd)
					{
						if(isBtnbreak == false)
							isBtnbreak = true;						
						
						isKbDrvWritingCmd = false;

							while(!isKbDrvWaitingCmd)
								usleep(1000);
							currKbRunCmds = NULL;
					}

                	ParserKeyboardData(keyboard, &KeyboardMsgData);
				}
                else if(KeyboardMsgData.KeyValue1 == 0)
                {
                	// NotifyLogMessage("KeyboardMsgData.KeyValue1 == 0....................");
                	isBtnbreak = true;
                }
			}
            else
            {
            	NotifyLogMessage("[ERR]   [darwinDriver]   [MyKeyboardListenForData]   [IODataQueueDequeue error returned]");
                goto exit;
            }
         }
    }
    
exit:
    // IOConnectUnmapMemory will call clientMemoryForType() inside your user client class.
    kernResult = IOConnectUnmapMemory(devData->connection, kIODefaultMemoryType, mach_task_self(), address);
    if (kernResult != kIOReturnSuccess)
    	NotifyLogMessage("[ERR]   [darwinDriver]   [MyKeyboardListenForData]   [IOConnectUnmapMemory returned error]");

    mach_port_destroy(mach_task_self(), recvPort);
        
freeDeviceData:
    devData->myThread.bRun = false;
    devData->myThread.thread = NULL;	
    if(devData->connection){
        IOServiceClose(devData->connection);
        devData->connection = 0;
    }
 	//return kernResult;
	return NULL;
}


void MyDeviceAdd(void *refCon, io_iterator_t iterator)
{
    kern_return_t       kr;
    io_service_t        obj;
    int					res;
    pthread_attr_t		attr;
    sIDUsbInterfaceInfo	sUsbDeviceInfo;
    size_t				structSize = sizeof(sIDUsbInterfaceInfo);
    
    while ((obj = IOIteratorNext(iterator)))
    {
    	NotifyLogMessage("[MSG]   [darwinDriver]   [MyDeviceAdd]   [Begin MyDeviceAdd]");
		isInitDriver = true;
		PMouse_DeviceData devData = GetNewDeviceData();
		if (devData == NULL)
            break;
        kr = IOServiceOpen(obj, mach_task_self(), 0, &(devData->connection));
        IOObjectRelease(obj);
        if (kr != kIOReturnSuccess)
        {
        	NotifyLogMessage("[ERR]   [darwinDriver]   [MyDeviceAdd]   [IOServiceOpen failed returned error]");
            continue;
        }

		kr = IOConnectCallMethod(devData->connection,						// an io_connect_t returned from IOServiceOpen().
                                         kGetUsbDeviceInfo,		// selector of the function to be called via the user client.
                                         NULL,					// array of scalar (64-bit) input values.
                                         0,								// the number of scalar input values.
                                         NULL,							// a pointer to the struct input parameter.
                                         0,								// the size of the input structure parameter.
                                         NULL,							// array of scalar (64-bit) output values.
                                         NULL,							// pointer to the number of scalar output values.
                                         &sUsbDeviceInfo,						// pointer to the struct output parameter.
                                         &structSize					// pointer to the size of the output structure parameter.
                                         );
       	if (kr != KERN_SUCCESS)
        {
        	NotifyLogMessage("[ERR]   [darwinDriver]   [MyDeviceAdd]   [IOConnectCallScalarMethod failed returned]");
			continue;
        }
		devData->mouse = GetNewMouse((sUsbDeviceInfo.nDeviceID & 0xFFFF0000) >> 16, sUsbDeviceInfo.nDeviceID & 0xFFFF);
		snprintf(devData->locPath, sizeof(devData->locPath), "HID_%04hx_%04hx_%x", devData->mouse->VID, devData->mouse->PID, sUsbDeviceInfo.nLocationID);
        
        //set thread start
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
        res = pthread_create(&(devData->myThread.thread),&attr,MyListenForData,devData);
        if(res!=0){
        	NotifyLogMessage("[ERR]   [darwinDriver]   [MyDeviceAdd]   [Create thread failed]");
        }
    }
}

void InitFilterDriver(){
	kern_return_t	kernResult;
    mach_port_t		masterPort;
    CFDictionaryRef	matchingDict;
	
	isInitDriver = false;
	kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
    
    if (kernResult != kIOReturnSuccess)
    {
    	NotifyLogMessage("[ERR]   [darwinDriver]   [InitFilterDriver]   [IOMasterPort returned error]");
        return ;
    }
	matchingDict = IOServiceMatching(FILTER_DRV_NAME);
    if (matchingDict == NULL)
    {
    	NotifyLogMessage("[ERR]   [darwinDriver]   [InitFilterDriver]   [Can't create a USB matching dictionary]");
        mach_port_deallocate(mach_task_self(), masterPort);
        return;
    }
	
	gNotifyPort = IONotificationPortCreate(masterPort);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), IONotificationPortGetRunLoopSource(gNotifyPort), kCFRunLoopDefaultMode);
	kernResult = IOServiceAddMatchingNotification(gNotifyPort,			// notifyPort
                                                  kIOFirstMatchNotification,	// notificationType
                                                  matchingDict,			// matching
                                                  MyDeviceAdd,		// callback
                                                  NULL,				// refCon
                                                  &gAddedIter			// notification
                                                  );
    
    if (kernResult != kIOReturnSuccess )
	{
		NotifyLogMessage("[ERR]   [darwinDriver]   [InitFilterDriver]   [IOServiceAddMatchingNotification failed]");
        mach_port_deallocate(mach_task_self(), masterPort);
        return;
	}
    
    MyDeviceAdd(NULL, gAddedIter);
 
    //all done
    mach_port_deallocate(mach_task_self(), masterPort);
    masterPort = 0;
}


void MyKbDeviceAdd(void *refCon, io_iterator_t iterator)
{
    kern_return_t       kr;
    io_service_t        obj;
    int					res;
    pthread_attr_t		attr;
    sIDKbUsbInterfaceInfo	sUsbDeviceInfo;
    size_t				structSize = sizeof(sIDUsbInterfaceInfo);
    
    while ((obj = IOIteratorNext(iterator)))
    {
    	NotifyLogMessage("[MSG]   [darwinDriver]   [MyKbDeviceAdd]   [Begin MyKbDeviceAdd]");

		isKbInitDriver = true;
		PKeyboard_DeviceData devData = GetNewKBDeviceData();
		if (devData == NULL)
            break;
        kr = IOServiceOpen(obj, mach_task_self(), 0, &(devData->connection));
        IOObjectRelease(obj);
        if (kr != kIOReturnSuccess)
        {
        	NotifyLogMessage("[ERR]   [darwinDriver]   [MyKbDeviceAdd]   [IOServiceOpen failed returned error]");
            continue;
        }

		kr = IOConnectCallMethod(devData->connection,						// an io_connect_t returned from IOServiceOpen().
                                         kGetKbUsbDeviceInfo,		// selector of the function to be called via the user client.
                                         NULL,					// array of scalar (64-bit) input values.
                                         0,								// the number of scalar input values.
                                         NULL,							// a pointer to the struct input parameter.
                                         0,								// the size of the input structure parameter.
                                         NULL,							// array of scalar (64-bit) output values.
                                         NULL,							// pointer to the number of scalar output values.
                                         &sUsbDeviceInfo,						// pointer to the struct output parameter.
                                         &structSize					// pointer to the size of the output structure parameter.
                                         );
       	if (kr != KERN_SUCCESS)
        {
        	NotifyLogMessage("[ERR]   [darwinDriver]   [MyKbDeviceAdd]   [IOConnectCallScalarMethod failed returned]");
			continue;
        }
		devData->keyboard = GetNewKeyboard((sUsbDeviceInfo.nDeviceID & 0xFFFF0000) >> 16, sUsbDeviceInfo.nDeviceID & 0xFFFF);
		snprintf(devData->locPath, sizeof(devData->locPath), "HID_%04hx_%04hx_%x", devData->keyboard->VID, devData->keyboard->PID, sUsbDeviceInfo.nLocationID);
        
        //set thread start
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
        res = pthread_create(&(devData->myThread.thread),&attr,MyKeyboardListenForData,devData);
        if(res!=0){
        	NotifyLogMessage("[ERR]   [darwinDriver]   [MyKbDeviceAdd]   [create Keyboard thread failed]");
        }
    }
}


void InitKBFilterDriver(){
	kern_return_t	kernResult;
    mach_port_t		masterPort;
    CFDictionaryRef	matchingDict;
	
	NotifyLogMessage("[MSG]   [darwinDriver]   [InitKBFilterDriver]   [Begin InitKBFilterDriver]");

	isKbInitDriver = false;
	kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
    
    if (kernResult != kIOReturnSuccess)
    {
    	NotifyLogMessage("[ERR]   [darwinDriver]   [InitKBFilterDriver]   [IOMasterPort returned error]");
        return ;
    }

	matchingDict = IOServiceMatching(FILTER_KBDRV_NAME);
    if (matchingDict == NULL)
    {
        NotifyLogMessage("darwinDriver - Can't create a USB Keyboard matching dictionary.");
        mach_port_deallocate(mach_task_self(), masterPort);
        return;
    }
	
	gNotifyPort = IONotificationPortCreate(masterPort);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), IONotificationPortGetRunLoopSource(gNotifyPort), kCFRunLoopDefaultMode);
	kernResult = IOServiceAddMatchingNotification(gNotifyPort,			// notifyPort
                                                  kIOFirstMatchNotification,	// notificationType
                                                  matchingDict,			// matching
                                                  MyKbDeviceAdd,		// callback
                                                  NULL,				// refCon
                                                  &gAddedIter			// notification
                                                  );
    
    if (kernResult != kIOReturnSuccess )
	{
		NotifyLogMessage("[ERR]   [darwinDriver]   [InitKBFilterDriver]   [IOServiceAddMatchingNotification Keyboard failed]");
        mach_port_deallocate(mach_task_self(), masterPort);
        return;
	}
    
    MyKbDeviceAdd(NULL, gAddedIter);
 
    //all done
    mach_port_deallocate(mach_task_self(), masterPort);
    masterPort = 0;
}


void CheckDriverInstall(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	bool r = (0 == system("/usr/sbin/kextstat | /usr/bin/grep -qF com.geniusnet.driver.DeviceFilter"));
	
	args.GetReturnValue().Set(r);
}

void CheckKBDriverInstall(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	bool r = (0 == system("/usr/sbin/kextstat | /usr/bin/grep -qF com.geniusnet.driver.KBFilter"));
	
	args.GetReturnValue().Set(r);
}


void WriteUSBCmd(UInt16 vid, UInt16 pid, UInt8 req, UInt16 val, UInt16 idx, char * locPath){
	kern_return_t   kernResult;
	sUSBCommandInfo data;
    
    data.bmRequestType = 0x40;
    data.bRequest = req;
    data.wIndex = idx;
    data.wValue = val;
    data.wLength = 0;
    
    data.intData[0] = 0;
    data.intData[1] = 0;
    data.intData[2] = 0;
    data.intData[3] = 0;
    data.intData[4] = 0;
    data.intData[5] = 0;
    data.intData[6] = 0;
    data.intData[7] = 0;
	
	for (int i = 0; i < MAX_MOUSE_QTY; i++)
	{
		if(mouseDeviceData[i] == NULL)
			break;
		if (mouseDeviceData[i]->connection == 0 || mouseDeviceData[i]->mouse == NULL)
			continue;
		if (mouseDeviceData[i]->mouse->VID == vid && mouseDeviceData[i]->mouse->PID == pid && strcmp(mouseDeviceData[i]->locPath, locPath) == 0)
		{
			kernResult = SetUsbCommand(mouseDeviceData[i]->connection, 0, sizeof(sUSBCommandInfo), &data);
		    if (kernResult != KERN_SUCCESS) {
		    	NotifyLogMessage("[ERR]   [darwinDriver]   [WriteUSBCmd]   [SetUsbCommand fail]");
		    }
		}
	}
}
UInt8 ReadUSBCmd(UInt16 vid, UInt16 pid, UInt8 req, UInt16 val, UInt16 idx, char * locPath){
	kern_return_t   kernResult;
    sUSBCommandInfo data;
    sUSBCommandInfo	sampleStruct2;
    size_t structSize1 = sizeof(sUSBCommandInfo);
    size_t structSize2 = sizeof(sUSBCommandInfo);

    data.bmRequestType = 0xc0;
    data.bRequest = req;
    data.wIndex = idx;
    data.wValue = val;
    data.wLength = 1;
    
	data.intData[0] = 0;
    data.intData[1] = 0;
    data.intData[2] = 0;
    data.intData[3] = 0;
    data.intData[4] = 0;
    data.intData[5] = 0;
    data.intData[6] = 0;
    data.intData[7] = 0;
	
	for (int i = 0; i < MAX_MOUSE_QTY; i++)
	{
		if(mouseDeviceData[i] == NULL)
			break;
		if (mouseDeviceData[i]->connection == 0 || mouseDeviceData[i]->mouse == NULL)
			continue;
		if (mouseDeviceData[i]->mouse->VID == vid && mouseDeviceData[i]->mouse->PID == pid && strcmp(mouseDeviceData[i]->locPath, locPath) == 0)
		{
			kernResult = GetUsbCommand(mouseDeviceData[i]->connection, structSize1, &data, &structSize2, &sampleStruct2);
		    if (kernResult != KERN_SUCCESS) {
		    	NotifyLogMessage("[ERR]   [darwinDriver]   [ReadUSBCmd]   [ReadUSBCmd fail]");
				continue;
		    }
			return sampleStruct2.intData[0];
		}
	}
}

void AmmoxSetDPI(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	if(!isInitDriver)
	{
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"Please install driver & plugin Ammox Device first.")));
		return;
	}
	if (args.Length() != 5) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 5 arguments:[vid, pid, dpi1, dpi2, Location]")));
		return;
	}
	if (!args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsInt32() || !args[3]->IsInt32() || !args[4]->IsString()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"argument type can only be Int32,Int32,Int32,Int32,String.")));
		return;
	}
	
	uint32_t vid = args[0]->Uint32Value();
	uint32_t pid = args[1]->Uint32Value();
	uint32_t dpi1 = args[2]->Uint32Value();
	uint32_t dpi2 = args[3]->Uint32Value();
	
	char sLocation[255] = {0};
	String::Utf8Value valLocation(args[4]->ToString());
	int pathLen = valLocation.length();
	strncpy(sLocation, *valLocation, pathLen);
	
	WriteUSBCmd(vid, pid, 0x01, 0x0000, 0x0000, sLocation);
	WriteUSBCmd(vid, pid, 0x01, 0x0100, 0x5a09, sLocation);
	WriteUSBCmd(vid, pid, 0x01, 0x0100, 0x017f, sLocation);
	//Set 1 & 2 DPI.
	WriteUSBCmd(vid, pid, 0x01, 0x0100, dpi1, sLocation);
	//Set 3 & 4 DPI.
	WriteUSBCmd(vid, pid, 0x01, 0x0100, dpi2, sLocation);
}

void AmmoxGetDPI(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	if(!isInitDriver)
	{
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"Please install driver & plugin Ammox Device first.")));
		return;
	}
	if (args.Length() != 3) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 3 arguments:[vid, pid, Location]")));
		return;
	}
	if (!args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsString()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"argument type can only be Int32,Int32,String.")));
		return;
	}
	uint32_t vid = args[0]->Uint32Value();
	uint32_t pid = args[1]->Uint32Value();
	
	char sLocation[255] = {0};
	String::Utf8Value valLocation(args[2]->ToString());
	int pathLen = valLocation.length();
	strncpy(sLocation, *valLocation, pathLen);
	
	WriteUSBCmd(vid, pid, 0x01, 0x0000, 0x0000, sLocation);
	WriteUSBCmd(vid, pid, 0x01, 0x0100, 0x5a09, sLocation);
	WriteUSBCmd(vid, pid, 0x01, 0x0100, 0x017f, sLocation);
	//Get 1 & 2 DPI.
	UInt8 dpi1 = ReadUSBCmd(vid, pid, 0x01, 0x0100, 0x0010, sLocation);
	//Get 3 & 4 DPI.
	UInt8 dpi2 = ReadUSBCmd(vid, pid, 0x01, 0x0100, 0x0011, sLocation);
	Handle<Object> result = Object::New(isolate);
    result->Set(String::NewFromUtf8(isolate,"DPI1"), Integer::New(isolate, dpi1));
    result->Set(String::NewFromUtf8(isolate,"DPI2"), Integer::New(isolate, dpi2));
    args.GetReturnValue().Set(result);
}

void RegisterTransfer(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	if (args.Length() != 5) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 5 arguments:[vid, pid, sn, Location, function]")));
		return;
	}
	if (!args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsString() || !args[3]->IsString() || !args[4]->IsFunction()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"argument type can only be Int32,Int32,String,String,Function.")));
		return;
	}
	uint32_t vid = args[0]->Uint32Value();
	uint32_t pid = args[1]->Uint32Value();
	
	char sLocation[255] = {0};
	String::Utf8Value valLocation(args[3]->ToString());
	int pathLen = valLocation.length();
	strncpy(sLocation, *valLocation, pathLen);
	
	PMouse_DeviceData dev = GetDeviceData(vid, pid, sLocation);
	if (dev == NULL){
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"GetDeviceData is NULL.")));
		return;
	}
	String::Utf8Value valSN(args[2]->ToString());
	pathLen = valSN.length();
	strncpy(dev->mouse->SN, *valSN, pathLen);
	dev->callback.Reset(isolate, args[4]);
	dev->isTransfer = true;
}


void RegisterKBTransfer(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	if (args.Length() != 5) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"KB function can only be 5 arguments:[vid, pid, sn, Location, function]")));
		return;
	}
	if (!args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsString() || !args[3]->IsString() || !args[4]->IsFunction()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"KB argument type can only be Int32,Int32,String,String,Function.")));
		return;
	}
	uint32_t vid = args[0]->Uint32Value();
	uint32_t pid = args[1]->Uint32Value();
	
	char sLocation[255] = {0};
	String::Utf8Value valLocation(args[3]->ToString());
	int pathLen = valLocation.length();
	strncpy(sLocation, *valLocation, pathLen);
	
	PKeyboard_DeviceData dev = GetKbDeviceData(vid, pid, sLocation);
	if (dev == NULL){
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"KB GetDeviceData is NULL.")));
		return;
	}
	String::Utf8Value valSN(args[2]->ToString());
	pathLen = valSN.length();
	strncpy(dev->keyboard->SN, *valSN, pathLen);
	dev->callback.Reset(isolate, args[4]);
	dev->isTransfer = true;
}




extern "C" {
  void Init(Handle<Object> exports) {
    NODE_SET_METHOD(exports, "OpenDriver", OpenDriver);
    NODE_SET_METHOD(exports, "CloseDriver", CloseDriver);
    NODE_SET_METHOD(exports, "SetMacroKey", SetMacroKey);
    NODE_SET_METHOD(exports, "SetAllMacroKey", SetAllMacroKey);
	NODE_SET_METHOD(exports, "CheckDriverInstall", CheckDriverInstall);
	NODE_SET_METHOD(exports, "AmmoxSetDPI", AmmoxSetDPI);
	NODE_SET_METHOD(exports, "AmmoxGetDPI", AmmoxGetDPI);
	
	NODE_SET_METHOD(exports, "RegisterTransfer", RegisterTransfer);
	
	NODE_SET_METHOD(exports, "RegisterLogMessage", RegisterLogMessage);
	
	InitFilterDriver();


	//keyboard

	NODE_SET_METHOD(exports, "OpenKbDriver", OpenKbDriver);
    NODE_SET_METHOD(exports, "CloseKbDriver", CloseKbDriver);
    NODE_SET_METHOD(exports, "SetKbFunctionAction", SetKbFunctionAction);
    NODE_SET_METHOD(exports, "CheckKBDriverInstall", CheckKBDriverInstall);

	NODE_SET_METHOD(exports, "RegisterKBTransfer", RegisterKBTransfer);

	InitKBFilterDriver();

  }

  NODE_MODULE(driverDarwin, Init)
}

void OpenDriver(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	
	if (!isInitDriver)
	{
		InitFilterDriver();
	}
	else
	{
		//MessageBox(NULL,"Init Driver fail"," info",MB_OK);
	}
}
void CloseDriver(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	  
	//free device
	for(int i = 0; i < MAX_MOUSE_QTY; i++)
	{
		if (mouseDeviceData[i] == NULL)
			break;
		mouseDeviceData[i]->myThread.bRun = false;
	    mouseDeviceData[i]->myThread.thread = NULL;	
		mouseDeviceData[i]->mouse = NULL;
	    if(mouseDeviceData[i]->connection){
			sIDDriverInfo DriverInfo;
		    DriverInfo.bAppActive = false;
		    DriverInfo.nDebugLevel = 0;
		    if (SetDriverInfo(mouseDeviceData[i]->connection, 0, sizeof(sIDDriverInfo), &DriverInfo) != KERN_SUCCESS) {
		    	NotifyLogMessage("[ERR]   [darwinDriver]   [CloseDriver]   [SetDriverInfo error]");
		    }
	        IOServiceClose(mouseDeviceData[i]->connection);
	        mouseDeviceData[i]->connection = 0;
	    }
	}
	//free macro
	for(int i = 0; i < MAX_MOUSE_QTY; i++)
	{
		if (mouseMacros[i] == NULL)
			break;
		for (int n = 0; n < 20; n++)
		{
			if (mouseMacros[i]->Keys[n] == NULL)
				break;
			FreeMouseKeyMacro(mouseMacros[i]->Keys[n]);
			free(mouseMacros[i]->Keys[n]);
			mouseMacros[i]->Keys[n] = NULL;
		}
		free(mouseMacros[i]);
		mouseMacros[i] = NULL;
	}

	for(int i = 0; i < MAX_KEYBOARD_QTY; i++)
	{
		if (keyboardMacros[i] == NULL)
			break;
		for (int n = 0; n < 50; n++)
		{
			if (keyboardMacros[i]->Keys[n] == NULL)
				break;
			FreeKeyMacro(keyboardMacros[i]->Keys[n]);
			free(keyboardMacros[i]->Keys[n]);
			keyboardMacros[i]->Keys[n] = NULL;
		}
		free(keyboardMacros[i]);
		keyboardMacros[i] = NULL;
	}
}

void RegisterLogMessage(const v8::FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	  
	if (args.Length() < 1 || !args[0]->IsFunction()) 
	{
		isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"First argument must be a function")));
		return;
	}
	LogMessageCallback.Reset(isolate, args[0]);
	isLogMessageRegistered = true;
}

void NotifyLogMessageCallback(char * msg){
	Isolate* isolate = Isolate::GetCurrent();
  	HandleScope scope(isolate);
    Handle<Value> argv[1];
    argv[0] = String::NewFromUtf8(isolate, msg);
    Local<Value> cbVal=Local<Value>::New(isolate, LogMessageCallback);
    Local<Function> cb = Local<Function>::Cast(cbVal);
    cb->Call(isolate->GetCurrentContext()->Global(), 1, argv);
} 
void NotifyLogMessageAsync(uv_work_t* req)
{
}
void NotifyLogMessageFinished(uv_work_t* req)
{
	char * msg = static_cast<char *>(req->data);
  	NotifyLogMessageCallback(msg);
}

void NotifyLogMessage(char * msg){
  	if (isLogMessageRegistered) {
	    uv_work_t* reqLogChg = new uv_work_t();
		reqLogChg->data = msg;
        uv_queue_work(uv_default_loop(), reqLogChg, NotifyLogMessageAsync, (uv_after_work_cb)NotifyLogMessageFinished);
  	}else{
	  	DEBUG_LOG(msg);
  	}
}

kern_return_t SetDriverInfo(io_connect_t connect, const uint32_t scalarI, const size_t structISize, const sIDDriverInfo* structI)
{
	kern_return_t	kernResult;
	uint64_t scalarI_64 = scalarI;
        
    kernResult = IOConnectCallMethod(connect,						// an io_connect_t returned from IOServiceOpen().
                                         kSetDriverInfo,		// selector of the function to be called via the user client.
                                         &scalarI_64,					// array of scalar (64-bit) input values.
                                         0,								// the number of scalar input values.
                                         structI,						// a pointer to the struct input parameter.
                                         structISize,					// the size of the input structure parameter.
                                         NULL,							// array of scalar (64-bit) output values.
                                         NULL,							// pointer to the number of scalar output values.
                                         NULL,							// pointer to the struct output parameter.
                                         NULL							// pointer to the size of the output structure parameter.
                                         );
	return kernResult;
}
kern_return_t SetDeviceData(io_connect_t connect, const uint32_t scalarI, const size_t structISize, const sIDDeviceInfo* structI)
{
	kern_return_t	kernResult;
	uint64_t scalarI_64 = scalarI;
        
    kernResult = IOConnectCallMethod(connect,						// an io_connect_t returned from IOServiceOpen().
                                     kSetDeviceData,		// selector of the function to be called via the user client.
                                     &scalarI_64,					// array of scalar (64-bit) input values.
                                     0,								// the number of scalar input values.
                                     structI,						// a pointer to the struct input parameter.
                                     structISize,					// the size of the input structure parameter.
                                     NULL,							// array of scalar (64-bit) output values.
                                     NULL,							// pointer to the number of scalar output values.
                                     NULL,							// pointer to the struct output parameter.
                                     NULL							// pointer to the size of the output structure parameter.
                                     );
	return kernResult;
}


//Keyboard
kern_return_t SetKbDriverInfo(io_connect_t connect, const uint32_t scalarI, const size_t structISize, const sIDKbDriverInfo* structI)
{
	kern_return_t	kernResult;
	uint64_t scalarI_64 = scalarI;
        
    kernResult = IOConnectCallMethod(connect,						// an io_connect_t returned from IOServiceOpen().
                                         kSetKbDriverInfo,		// selector of the function to be called via the user client.
                                         &scalarI_64,					// array of scalar (64-bit) input values.
                                         0,								// the number of scalar input values.
                                         structI,						// a pointer to the struct input parameter.
                                         structISize,					// the size of the input structure parameter.
                                         NULL,							// array of scalar (64-bit) output values.
                                         NULL,							// pointer to the number of scalar output values.
                                         NULL,							// pointer to the struct output parameter.
                                         NULL							// pointer to the size of the output structure parameter.
                                         );
	return kernResult;
}
kern_return_t SetKbDeviceData(io_connect_t connect, const uint32_t scalarI, const size_t structISize, const sIDKbDeviceInfo* structI)
{
	kern_return_t	kernResult;
	uint64_t scalarI_64 = scalarI;
        
    kernResult = IOConnectCallMethod(connect,						// an io_connect_t returned from IOServiceOpen().
                                     kSetKbDeviceData,		// selector of the function to be called via the user client.
                                     &scalarI_64,					// array of scalar (64-bit) input values.
                                     0,								// the number of scalar input values.
                                     structI,						// a pointer to the struct input parameter.
                                     structISize,					// the size of the input structure parameter.
                                     NULL,							// array of scalar (64-bit) output values.
                                     NULL,							// pointer to the number of scalar output values.
                                     NULL,							// pointer to the struct output parameter.
                                     NULL							// pointer to the size of the output structure parameter.
                                     );
	return kernResult;
}

kern_return_t SetUsbCommand(io_connect_t connect, const uint32_t scalarI, const size_t structISize, const sUSBCommandInfo* structI)
{
	kern_return_t	kernResult;
	uint64_t scalarI_64 = scalarI;
        
    kernResult = IOConnectCallMethod(connect,						// an io_connect_t returned from IOServiceOpen().
                                     kSetUSBCommand,		// selector of the function to be called via the user client.
                                     &scalarI_64,					// array of scalar (64-bit) input values.
                                     0,								// the number of scalar input values.
                                     structI,						// a pointer to the struct input parameter.
                                     structISize,					// the size of the input structure parameter.
                                     NULL,							// array of scalar (64-bit) output values.
                                     NULL,							// pointer to the number of scalar output values.
                                     NULL,							// pointer to the struct output parameter.
                                     NULL							// pointer to the size of the output structure parameter.
                                     );
	return kernResult;
}

kern_return_t GetUsbCommand(io_connect_t connect, const size_t structISize, const sUSBCommandInfo* structI, size_t* structOSize, sUSBCommandInfo* structO)
{
	kern_return_t	kernResult;
	kernResult = IOConnectCallStructMethod(connect,						// an io_connect_t returned from IOServiceOpen().
                                               kGetUSBCommand,		// selector of the function to be called via the user client.
                                               structI,						// pointer to the input struct parameter.
                                               structISize,					// the size of the input structure parameter.
                                               structO,						// pointer to the output struct parameter.
                                               structOSize					// pointer to the size of the output structure parameter.
                                               );
	return kernResult;
}

void SetMacroKey(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	
	if (args.Length() != 5) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 5 arguments:[vid, pid, macro, sn, Location]")));
		return;
	}
	if (!args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsObject() || !args[3]->IsString() || !args[4]->IsString()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"argument type can only be Int32,Int32,Object,String,String.")));
		return;
	}
	uint32_t vid = args[0]->Uint32Value();
	uint32_t pid = args[1]->Uint32Value();
	
	char sLocation[255] = {0};
	String::Utf8Value valLocation(args[4]->ToString());
	int pathLen = valLocation.length();
	strncpy(sLocation, *valLocation, pathLen);
	
	PMOUSE_MACRO_PACKET mouse = GetMouseFromDevs(vid, pid, sLocation);
	if (mouse == NULL){
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"GetMouse is NULL.")));
		return;
	}
	String::Utf8Value valSN(args[3]->ToString());
	pathLen = valSN.length();
	strncpy(mouse->SN, *valSN, pathLen);
	
	Local<Object> obj = args[2]->ToObject();
	Local<Array> arr = Local<Array>::Cast(obj);
	for (int i = 0; i < 20; i++)
	{
		if(arr->Get(i)->IsUndefined())
			break;		
		Local<Object> item = arr->Get(i)->ToObject();
		uint32_t keyNum = item->Get(String::NewFromUtf8(isolate,"KeyNum"))->Uint32Value();
		PKEY_MACRO_PACKET key = GetMouseKey(mouse, keyNum);
		key->MacroType = item->Get(String::NewFromUtf8(isolate,"MacroType"))->Uint32Value();
		key->ButtonAction = item->Get(String::NewFromUtf8(isolate,"ButtonAction"))->Uint32Value();
		FreeMouseKeyMacro(key);		
		Local<Value> arrMacroVal = item->Get(String::NewFromUtf8(isolate,"MacroContent"));
		if (!arrMacroVal->IsUndefined()){
			Local<Array> arrMacro = Local<Array>::Cast(arrMacroVal);
			if (key->MacroType == 4){

				key->MacroMode = arrMacro->Get(String::NewFromUtf8(isolate,"MacroMode"))->Uint32Value();
				key->MacroRepeatTimes = arrMacro->Get(String::NewFromUtf8(isolate,"MacroRepeatTimes"))->Uint32Value();
			
				DEBUG_LOG("darwinDriver - [Debug]>>SetMacroKey()>>  VID:0x%04x, PID:0x%04x LocationPath:%s, MacroMode=%d, MacroRepeatTimes=%d",vid,pid,sLocation,key->MacroMode,key->MacroRepeatTimes);

				for(int n = 0; n < 255; n++)
				{
					if(arrMacro->Get(n)->IsUndefined())
						break;
					key->Cmds[n] = (PCMD_PACKET)calloc(1, sizeof(CMD_PACKET));
					Local<Object> itemMacro = arrMacro->Get(n)->ToObject();				
					key->Cmds[n]->DownUp = itemMacro->Get(String::NewFromUtf8(isolate,"DownUp"))->Uint32Value();
					key->Cmds[n]->Delay = itemMacro->Get(String::NewFromUtf8(isolate,"Delay"))->Uint32Value();
					key->Cmds[n]->KeyCode = HidToCGKeyCode[itemMacro->Get(String::NewFromUtf8(isolate,"KeyCode"))->Uint32Value()];
				}
			}else if(key->MacroType == 2 || key->MacroType == 3){
				for(int n = 0; n < 8; n++)
				{
					if(arrMacro->Get(n)->IsUndefined())
						break;
					key->Cmds[n] = (PCMD_PACKET)calloc(1, sizeof(CMD_PACKET));			
					key->Cmds[n]->DownUp = 0;
					key->Cmds[n]->Delay = 0;
					key->Cmds[n]->KeyCode = arrMacro->Get(n)->Uint32Value();
				}
			}
		}
	}
	SetMouseFunc(vid, pid, sLocation);
	NotifyLogMessage("[MSG]   [darwinDriver]   [SetMacroKey]   [SetMacroKey Complete]");
}

void SetAllMacroKey(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	
	if (args.Length() != 5) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 5 arguments:[vid, pid, macro, sn, Location]")));
		return;
	}
	if (!args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsObject() || !args[3]->IsString() || !args[4]->IsString()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"argument type can only be Int32,Int32,Object,String,String.")));
		return;
	}
	uint32_t vid = args[0]->Uint32Value();
	uint32_t pid = args[1]->Uint32Value();
	
	char sLocation[255] = {0};
	String::Utf8Value valLocation(args[4]->ToString());
	int pathLen = valLocation.length();
	strncpy(sLocation, *valLocation, pathLen);
	
	PKEYBOARD_MACRO_PACKET keyboard = GetKeyboardFromDevs(vid, pid, sLocation);
	if (keyboard == NULL){
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"GetKeyboard is NULL.")));
		return;
	}
	String::Utf8Value valSN(args[3]->ToString());
	pathLen = valSN.length();
	strncpy(keyboard->SN, *valSN, pathLen);


	DEBUG_LOG("darwinDriver - [Debug]>>SetAllMacroKey()>>  VID:0x%04x, PID:0x%04x LocationPath:%s",vid,pid,sLocation);
	
	Local<Object> obj = args[2]->ToObject();
	Local<Array> arr = Local<Array>::Cast(obj);
	for (int i = 0; i < 50; i++)
	{
		if(arr->Get(i)->IsUndefined())
			break;		
		Local<Object> item = arr->Get(i)->ToObject();
		uint32_t keyNum = item->Get(String::NewFromUtf8(isolate,"KeyNum"))->Uint32Value();
		PKEY_MACRO_PACKET key = GetKeyboardKey(keyboard, keyNum);
		key->MacroType = item->Get(String::NewFromUtf8(isolate,"MacroType"))->Uint32Value();
		FreeKeyMacro(key);		
		Local<Value> arrMacroVal = item->Get(String::NewFromUtf8(isolate,"MacroContent"));

		// DEBUG_LOG("GetKeyboard>> KeyNum[%d]", keyNum);
		// DEBUG_LOG("GetKeyboard>> MacroType[%d]", key->MacroType);
		
		if (!arrMacroVal->IsUndefined()){
		
			Local<Array> arrMacro = Local<Array>::Cast(arrMacroVal);
			if (key->MacroType == 4){
				for(int n = 0; n < 255; n++)
				{
					if(arrMacro->Get(n)->IsUndefined())
						break;
					key->Cmds[n] = (PCMD_PACKET)calloc(1, sizeof(CMD_PACKET));
					Local<Object> itemMacro = arrMacro->Get(n)->ToObject();				
					key->Cmds[n]->DownUp = itemMacro->Get(String::NewFromUtf8(isolate,"DownUp"))->Uint32Value();
					key->Cmds[n]->Delay = itemMacro->Get(String::NewFromUtf8(isolate,"Delay"))->Uint32Value();
					key->Cmds[n]->KeyCode = HidToCGKeyCode[itemMacro->Get(String::NewFromUtf8(isolate,"KeyCode"))->Uint32Value()];
				}
			}
		}
	}
	NotifyLogMessage("[MSG]   [darwinDriver]   [SetAllMacroKey]   [SetAllMacroKey Complete]");
}

//Keybaord
void OpenKbDriver(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	
	if (!isKbInitDriver)
	{
		NotifyLogMessage("[MSG]   [darwinDriver]   [OpenKbDriver]   [Begin initKBFilterDriver]");
		NotifyLogMessage("OpenKbDriver --- initKBFilterDriver.");
		InitKBFilterDriver();
	}
	else
	{
		NotifyLogMessage("[MSG]   [darwinDriver]   [OpenKbDriver]   [Already initKBFilterDriver]");
	}
}
void CloseKbDriver(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	  
	//free device
	for(int i = 0; i < MAX_KEYBOARD_QTY; i++)
	{
		if (keyboardDeviceData[i] == NULL)
			break;
		keyboardDeviceData[i]->myThread.bRun = false;
	    keyboardDeviceData[i]->myThread.thread = NULL;	
		keyboardDeviceData[i]->keyboard = NULL;
	    if(keyboardDeviceData[i]->connection){
			sIDKbDriverInfo DriverInfo;
		    DriverInfo.bAppActive = false;
		    DriverInfo.nDebugLevel = 0;
		    if (SetKbDriverInfo(keyboardDeviceData[i]->connection, 0, sizeof(sIDKbDriverInfo), &DriverInfo) != KERN_SUCCESS) {
		    	NotifyLogMessage("[ERR]   [darwinDriver]   [CloseKbDriver]   [SetKbDriverInfo error]");
		    }
	        IOServiceClose(keyboardDeviceData[i]->connection);
	        keyboardDeviceData[i]->connection = 0;
	    }
	}
	//free macro
	/*
	for(int i = 0; i < MAX_KEYBOARD_QTY; i++)
	{
		if (mouseMacros[i] == NULL)
			break;
		for (int n = 0; n < 20; n++)
		{
			if (mouseMacros[i]->Keys[n] == NULL)
				break;
			FreeMouseKeyMacro(mouseMacros[i]->Keys[n]);
			free(mouseMacros[i]->Keys[n]);
			mouseMacros[i]->Keys[n] = NULL;
		}
		free(mouseMacros[i]);
		mouseMacros[i] = NULL;
	}
	*/
}

void SetKbFunctionAction(const v8::FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
		
	//char       buf[200]={0};
	NotifyLogMessage("[MSG]   [darwinDriver]   [SetKbFunctionAction]   [Begin SetKbFunctionAction]");

	if (args.Length() != 5) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"KbFunctionList can only be 5 arguments:[vid, pid, macro, sn, Location]")));
		NotifyLogMessage("[ERR]   [darwinDriver]   [SetKbFunctionAction]   [args.Length() != 5, return]");
		return;
	}

	if (!args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsObject() || !args[3]->IsString() || !args[4]->IsString()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"KbFunctionList ,argument type can only be Int32,Int32,Object,String,String.")));
		NotifyLogMessage("[ERR]   [darwinDriver]   [SetKbFunctionAction]   [args[2]->IsObject() return]");
		return;
	}

	uint32_t vid = args[0]->Uint32Value();	
	uint32_t pid = args[1]->Uint32Value();

	char sLocation[255] = {0};
	String::Utf8Value valLocation(args[4]->ToString());
	
	int DataLen = valLocation.length();
	strncpy(sLocation, *valLocation, DataLen);


	DEBUG_LOG("darwinDriver - [Debug]>>SetKbFunctionAction()>>  VID:0x%04x, PID:0x%04x LocationPath:%s",vid,pid,sLocation);

	//PKEYBOARD_PACKET keyboard = GetKeyboard(vid, pid, sLocation);
	PKEYBOARD_MACRO_PACKET keyboard = GetKeyboardFromDevs(vid, pid, sLocation);

	if (keyboard == NULL){
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"darwinDriver - GetKeyboardFromDevs is NULL.")));
		return;
	}

	//String::Utf8Value valSN(args[3]->ToString());
	//pathLen = valSN.length();
	//strncpy(keyboard->SN, *valSN, pathLen);


	Local<Object> obj = args[2]->ToObject();
	Local<Array> ActionFunc = Local<Array>::Cast(obj);

	for (int i = 0; i < 12; i++) //12: F1~F12
	{
		if(ActionFunc->Get(i)->IsUndefined())
			break;

		Local<Object> item = ActionFunc->Get(i)->ToObject();

		UInt16 Index = i;

		PKEY_FUNCTION_PACKET key = GetKeyboardFuncKey(keyboard, Index);
		String::Utf8Value val(item->Get(String::NewFromUtf8(isolate,"keyName"))->ToString());
		DataLen = val.length();
		strncpy(key->KeyName, *val, DataLen);
		
		key->ProgramAction = item->Get(String::NewFromUtf8(isolate,"ProgramAction"))->Uint32Value();
		key->MacroMode = item->Get(String::NewFromUtf8(isolate,"MacroMode"))->Uint32Value();
		key->MacroRepeatTimes = item->Get(String::NewFromUtf8(isolate,"MacroRepeatTimes"))->Uint32Value();

		key->Function = item->Get(String::NewFromUtf8(isolate,"FuncTypeID"))->Uint32Value();

		strncpy(key->Param1, " ", 255);

		String::Utf8Value param1(item->Get(String::NewFromUtf8(isolate,"Param1"))->ToString());
		DataLen = param1.length();
		strncpy(key->Param1, *param1, DataLen);

		DEBUG_LOG("darwinDriver - [Debug]>>SetKbFunctionAction()>>  ProgramAction=%04x, MacroMode=%04x, MacroRepeatTimes=%04x, Function=%04x, LocationPath:%s",key->ProgramAction,key->MacroMode, key->MacroRepeatTimes, key->Function,key->Param1);

	}

	SetKeyboardFunc(vid, pid, sLocation);
	NotifyLogMessage("[MSG]   [darwinDriver]   [SetKbFunctionAction]   [SetKeyboard Function Complete]");
}

static io_connect_t get_event_driver() {

	static  mach_port_t sEventDrvrRef = 0;
    mach_port_t masterPort, service, iter;
    kern_return_t    kr;

    if (!sEventDrvrRef)
    {
         // Get master device port
         kr = IOMasterPort( bootstrap_port, &masterPort );
         check( KERN_SUCCESS == kr);

         kr = IOServiceGetMatchingServices( masterPort, IOServiceMatching( kIOHIDSystemClass ), &iter );
         check( KERN_SUCCESS == kr);

         service = IOIteratorNext( iter );
         check( service );

         kr = IOServiceOpen( service, mach_task_self(), kIOHIDParamConnectType, &sEventDrvrRef );
         check( KERN_SUCCESS == kr );

         IOObjectRelease( service );
         IOObjectRelease( iter );
     }

     return sEventDrvrRef;

}

static void HIDPostAuxKey(const UInt8 auxKeyCode )
{
	NXEventData   event;
  	kern_return_t kr;
  	IOGPoint      loc = { 0, 0 };

	// Key press event
  	UInt32      evtInfo = auxKeyCode << 16 | NX_KEYDOWN << 8;
  	bzero(&event, sizeof(NXEventData));
  	event.compound.subType = NX_SUBTYPE_AUX_CONTROL_BUTTONS;
  	event.compound.misc.L[0] = evtInfo;
  	kr = IOHIDPostEvent( get_event_driver(), NX_SYSDEFINED, loc, &event, kNXEventDataVersion, 0, FALSE );
  	check( KERN_SUCCESS == kr );

  	// Key release event
  	evtInfo = auxKeyCode << 16 | NX_KEYUP << 8;
  	bzero(&event, sizeof(NXEventData));
  	event.compound.subType = NX_SUBTYPE_AUX_CONTROL_BUTTONS;
  	event.compound.misc.L[0] = evtInfo;
  	kr = IOHIDPostEvent( get_event_driver(), NX_SYSDEFINED, loc, &event, kNXEventDataVersion, 0, FALSE );
  	check( KERN_SUCCESS == kr );
	 
}


static OSStatus LowRunAppleScript(const void* text, long textLength,AEDesc *resultData) 
{
	ComponentInstance theComponent;
	AEDesc scriptTextDesc;
	OSStatus err;
	OSAID scriptID, resultID;
	// set up locals to a known state 
	theComponent = NULL;
	AECreateDesc(typeNull, NULL, 0, &scriptTextDesc);
	scriptID = kOSANullScript;
	resultID = kOSANullScript;
		// open the scripting component 
       
	theComponent = OpenDefaultComponent(kOSAComponentType,
					typeAppleScript);
	if (theComponent == NULL) 
	{ 
		err = paramErr; 	 	
		NotifyLogMessage("[ERR]   [darwinDriver]   [LowRunAppleScript]   [OpenDefaultComponent]");
		goto bail; 
	}
        	/* put the script text into an aedesc */
	err = AECreateDesc(typeChar, text, textLength, &scriptTextDesc);
	if (err != noErr)
	{
		NotifyLogMessage("[ERR]   [darwinDriver]   [LowRunAppleScript]   [AECreateDesc]");
	 	goto bail;
    }   
		/* compile the script */
	err = OSACompile(theComponent, &scriptTextDesc, 
					kOSAModeNull, &scriptID);
	if (err != noErr)
	{ 
		NotifyLogMessage("[ERR]   [darwinDriver]   [LowRunAppleScript]   [OSACompile]");
		goto bail;
	}
        	/* run the script/get the result */
	err = OSAExecute(theComponent, scriptID, kOSANullScript,
					kOSAModeNull, &resultID);
        if (resultData != NULL) 
        {
			AECreateDesc(typeNull, NULL, 0, resultData);
			if (err == errOSAScriptError) 
			{
				OSAScriptError(theComponent, kOSAErrorMessage,
						typeChar, resultData);

				NotifyLogMessage("[ERR]   [darwinDriver]   [LowRunAppleScript]   [errOSAScriptError]");
			}
			else if (err == noErr && resultID != kOSANullScript)
			{
				OSADisplay(theComponent, resultID, typeChar,
							kOSAModeNull, resultData);

				NotifyLogMessage("[ERR]   [darwinDriver]   [LowRunAppleScript]   [kOSANullScript]");
			}
	}
bail:
	AEDisposeDesc(&scriptTextDesc);
	if (scriptID != kOSANullScript) OSADispose(theComponent,  scriptID);
	if (resultID != kOSANullScript) OSADispose(theComponent,  resultID);
	if (theComponent != NULL) CloseComponent(theComponent);

	return err;
}



static OSStatus SimpleRunAppleScript(const char* theScript) {
	return LowRunAppleScript(theScript, strlen(theScript), NULL);
}


OSStatus SendAppleEventToSystemProcess(AEEventID EventToSend)
{
    AEAddressDesc targetDesc;
    static const ProcessSerialNumber
         kPSNOfSystemProcess = { 0, kSystemProcess };
    AppleEvent eventReply = {typeNull, NULL};
    AppleEvent appleEventToSend = {typeNull, NULL};

    OSStatus error = noErr;

    error = AECreateDesc(typeProcessSerialNumber,
        &kPSNOfSystemProcess, sizeof(kPSNOfSystemProcess),
        &targetDesc);

    if (error != noErr)
    {
        return(error);
    }

    error = AECreateAppleEvent(kCoreEventClass, EventToSend,
                     &targetDesc, kAutoGenerateReturnID,
                     kAnyTransactionID, &appleEventToSend);

    AEDisposeDesc(&targetDesc);

    if (error != noErr)
    {
        return(error);
    }

    error = AESend(&appleEventToSend, &eventReply, kAENoReply,
             kAENormalPriority, kAEDefaultTimeout,
             NULL, NULL);

    AEDisposeDesc(&appleEventToSend);

    if (error != noErr)
    {
        return(error);
    }

    AEDisposeDesc(&eventReply);

    return(error); //if this is noErr then we are successful
}



