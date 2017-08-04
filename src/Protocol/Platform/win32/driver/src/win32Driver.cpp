#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <uv.h>

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <virtualDriver.h>
#include <filterDriver.h>

#include <string.h>
#include <tchar.h>
#include <winreg.h>


using namespace v8;

HANDLE    hVirtualDrvHandle = INVALID_HANDLE_VALUE, hVirtualDrvWaitEvent = NULL, hVirtualDrvWriteEvent = NULL, hOpenDriverWaitEvent = NULL;
HANDLE    hFilterDrvHandle = INVALID_HANDLE_VALUE, hFilterDrvEvent = NULL, hFilterDrvCloseEvent = NULL;
HANDLE	  hUsbCmdHandle = INVALID_HANDLE_VALUE;

DWORD filterThreadId, virtualThreadId;
HANDLE filterThreadHandle = NULL, virtualThreadHandle = NULL;
bool isFilterDrvExit = false, isVirtualDrvWritingCmd = false, isVirtualDrvWaitingCmd = false;

Persistent<Value> LogMessageCallback;
bool isLogMessageRegistered = false;
void NotifyLogMessage(TCHAR * msg);
void StopMouseKeyMacro();

DWORD WINAPI FilterThreadProc(LPVOID lpParam);
void OpenDriver(const v8::FunctionCallbackInfo<Value>& args);
void CloseDriver(const v8::FunctionCallbackInfo<Value>& args);
void SetMacroKey(const v8::FunctionCallbackInfo<Value>& args);
void SetAllMacroKey(const v8::FunctionCallbackInfo<Value>& args);
void RegisterLogMessage(const v8::FunctionCallbackInfo<Value>& args);

#define DEBUG_HEADER fprintf(stderr, "Win32Driver [%s:%s() %d]: ", __FILE__, __FUNCTION__, __LINE__);
#define DEBUG_FOOTER fprintf(stderr, "\n");
#define DEBUG_LOG(...) DEBUG_HEADER fprintf(stderr, __VA_ARGS__); DEBUG_FOOTER
#define MAX_MOUSE_QTY 10
#define MAX_KEYBOARD_QTY 10
#define VIRTUAL_DRV_NAME "\\\\.\\ioFakDrv"
#define FILTER_DRV_NAME "\\\\.\\gFilterMouUsb"
#define USB_CMD_DRV_NAME "\\\\.\\gHidCommand"
#define KEYBOARD_FILTER_DRV_NAME "\\\\.\\XgKbdUpper"


/////////////Win32 Kb Driver//////////////////////////////
#define MAX_LOADSTRING 100
#define MAX_PATH 255

#define		IOCTL_REGISTER_EVENT	CTL_CODE(FILE_DEVICE_UNKNOWN,2011,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define		IOCTL_UNREGISTER_EVENT	CTL_CODE(FILE_DEVICE_UNKNOWN,2012,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define		IOCTL_PEEK_DATA			CTL_CODE(FILE_DEVICE_UNKNOWN,6000,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define		WM_DATA_IN      WM_USER+2001

#define KeyDown(VKCode)	keybd_event(VKCode,(BYTE)MapVirtualKey(VKCode,0) ,0 ,0 )
#define KeyUp(VKCode)	keybd_event(VKCode,(BYTE)MapVirtualKey(VKCode,0) ,KEYEVENTF_KEYUP ,0 )

HANDLE		hHandle = INVALID_HANDLE_VALUE;
BOOL		ThreadAlive = FALSE;


typedef struct _KB_EVENTPACK
{
	HANDLE		hEvent;
	ULONG		hDevExist;
} KB_EVENTPACK, *PKB_EVENTPACK;

KB_EVENTPACK		pKbDevEvent;

typedef struct _DEVICE_DATA
{
	WCHAR		drvName[128];
	WCHAR		hwID[128];
	LONG		devVID;
	LONG		devPID;
	LONG		drvID;
	LONG		keyData[8];
} DEVICE_DATA, *PDEVICE_DATA;
DEVICE_DATA		DeviceData;

typedef struct _KEY_FUNCTION_PACKET
{
	char KeyName[16];
	USHORT KeyIndex;
	USHORT KeyType;
	USHORT Group;
	USHORT Function;

}KEY_FUNCTION_PACKET,*PKEY_FUNCTION_PACKET;

typedef struct _KEYBOARD_PACKET
{
	USHORT VID;
	USHORT PID;
	char SN[30];
	char locPath[255];
	PKEY_FUNCTION_PACKET Keys[12];
}KEYBOARD_PACKET,*PKEYBOARD_PACKET;

PKEYBOARD_PACKET KeyboardInfo[MAX_KEYBOARD_QTY];

DWORD WINAPI KbPeekData(LPVOID arg);
void OpenKbDriver(const v8::FunctionCallbackInfo<Value>& args);
void CloseKbDriver(const v8::FunctionCallbackInfo<Value>& args);
void SetKbFunctionAction(const v8::FunctionCallbackInfo<Value>& args);



////////////////////////////////////////////////


typedef struct _CMD_PACKET
{
	USHORT DownUp;
	ULONG  Delay;
	int KeyCode;
}CMD_PACKET, *PCMD_PACKET;

typedef struct _KEY_MACRO_PACKET
{
	USHORT KeyNum;
	USHORT MacroType;
	USHORT ButtonAction;//Anderson add
	USHORT MacroMode;//Anderson add
	USHORT MacroRepeatTimes;//Anderson add
	PCMD_PACKET Cmds[255];
}KEY_MACRO_PACKET,*PKEY_MACRO_PACKET;

typedef struct _MOUSE_MACRO_PACKET
{
	USHORT VID;
	USHORT PID;
	char SN[30];
	char locPath[255];
	PKEY_MACRO_PACKET Keys[20];
}MOUSE_MACRO_PACKET,*PMOUSE_MACRO_PACKET;

PMOUSE_MACRO_PACKET mouseMacros[MAX_MOUSE_QTY];
PKEY_MACRO_PACKET currRunCmds = NULL;

PMOUSE_MACRO_PACKET GetMouse(USHORT vid, USHORT pid, char * locPath)
{
	for (int i = 0; i < MAX_MOUSE_QTY; i++)
	{
		if(mouseMacros[i] == NULL)
		{
			mouseMacros[i] = (PMOUSE_MACRO_PACKET)calloc(1, sizeof(MOUSE_MACRO_PACKET));
			mouseMacros[i]->VID = vid;
			mouseMacros[i]->PID = pid;
			strcpy(mouseMacros[i]->locPath, locPath);
			return mouseMacros[i];
		}
		if (mouseMacros[i]->VID == vid && mouseMacros[i]->PID == pid/* && stricmp(mouseMacros[i]->locPath, locPath) == 0*/)
			return mouseMacros[i];
	}
	return NULL;
}
PKEY_MACRO_PACKET GetMouseKey(PMOUSE_MACRO_PACKET mouse, USHORT key)
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

//Virtual Driver.
bool OpenVirtualDriver() {
	hVirtualDrvHandle = CreateFile(VIRTUAL_DRV_NAME, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  	if (hVirtualDrvHandle != INVALID_HANDLE_VALUE)
    	return true;
  	return false;
}

void CloseVirtualDriver() {
	if (hVirtualDrvWaitEvent){
		CloseHandle(hVirtualDrvWaitEvent);
		hVirtualDrvWaitEvent = NULL;
	}
	if (hVirtualDrvWriteEvent){
		CloseHandle(hVirtualDrvWriteEvent);
		hVirtualDrvWriteEvent = NULL;
	}
	if (virtualThreadHandle != NULL){
			TerminateThread(virtualThreadHandle, 0);
			virtualThreadHandle = NULL;
		}
	if (hVirtualDrvHandle != INVALID_HANDLE_VALUE){
		CloseHandle(hVirtualDrvHandle);
		hVirtualDrvHandle = INVALID_HANDLE_VALUE;
	}
}

USHORT GetMouseIsMakeKeyStatus(PSIMULATE_KEYPRESS_PACKET keyPressCmd)
{
	return (keyPressCmd->KeyBuffer[0] == 0 && keyPressCmd->KeyBuffer[2] == 0 && keyPressCmd->KeyBuffer[3] == 0 && keyPressCmd->KeyBuffer[4] == 0 && keyPressCmd->KeyBuffer[5] == 0 && keyPressCmd->KeyBuffer[6] == 0 && keyPressCmd->KeyBuffer[7] == 0) ? 0 : 1;
}

USHORT GetHidCodeIsModifyCode(USHORT hidCode)
{
	if (hidCode == 0xe0 || hidCode == 0xe4)
		return VIRTUAL_CONTROL_KEY;
	else if (hidCode == 0xe1 || hidCode == 0xe5)
		return VIRTUAL_SHIFT_KEY;
	else if (hidCode == 0xe2 || hidCode == 0xe6)
		return VIRTUAL_ALT_KEY;
	else if (hidCode == 0xe3 || hidCode == 0xe7)
		return VIRTUAL_WIN_KEY;
	return VIRTUAL_KEY_MODIFY_NULL;
}

DWORD WINAPI VitualThreadProc(LPVOID lpParam)
{
	PSIMULATE_PACKET macroKeyCmd = NULL;
	PSIMULATE_KEYPRESS_PACKET keyPressCmd = NULL;
	DWORD dwRet;
	
	while(true)
	{
		isVirtualDrvWaitingCmd = true;
		WaitForSingleObject(hVirtualDrvWaitEvent, INFINITE);
		isVirtualDrvWritingCmd = true;
		isVirtualDrvWaitingCmd = false;
		if (currRunCmds != NULL){
			if (currRunCmds->MacroType == 2){//Keyboard
				macroKeyCmd = (PSIMULATE_PACKET)calloc(1, sizeof(SIMULATE_REQ));
				macroKeyCmd->Type = VIRTUAL_DEVICE_TYPE_KEYBOARD;
				macroKeyCmd->KeyNumber = 0;
				for(int i = 1; i < 255; i++){
					if (currRunCmds->Cmds[i] == NULL)
						break;
					macroKeyCmd->Data[i-1][0] = currRunCmds->Cmds[0]->KeyCode;
					macroKeyCmd->Data[i-1][1] = currRunCmds->Cmds[i]->KeyCode;
					macroKeyCmd->KeyNumber++;
				}
				DeviceIoControl(hVirtualDrvHandle, IOCTL_SIMULATE_MACRO_KEY, macroKeyCmd, sizeof(SIMULATE_REQ), NULL, 0, &dwRet, NULL);
				free(macroKeyCmd);
				macroKeyCmd = NULL;
			}else if (currRunCmds->MacroType == 3 || currRunCmds->MacroType == 4){//Macro
				keyPressCmd = (PSIMULATE_KEYPRESS_PACKET)calloc(1, sizeof(SIMULATE_KEYPRESS_PACKET));
				keyPressCmd->Type = VIRTUAL_DEVICE_TYPE_KEYBOARD;
				if (currRunCmds->MacroType == 4){
					//Anderson modify
					//MacroMode == 1 ,No repeat，times=1;
					//MacroMode == 2 ,have repeat，times=N;
					if((currRunCmds->MacroMode == 1)||(currRunCmds->MacroMode == 2))
					{
						for(int times = 0; times < currRunCmds->MacroRepeatTimes; times++)
						{
							DEBUG_LOG("([Debug]>>>> MacroRepeatTimes = %d", times+1);
							for(int i = 0; i < 255; i++)
							{
								if (currRunCmds->Cmds[i] == NULL)
									break;
								WaitForSingleObject(hVirtualDrvWriteEvent, currRunCmds->Cmds[i]->Delay);
								if (!isVirtualDrvWritingCmd)
									break;
								USHORT modifyCode = GetHidCodeIsModifyCode(currRunCmds->Cmds[i]->KeyCode);
								if (modifyCode != VIRTUAL_KEY_MODIFY_NULL)
								{//Modify Key
									if (currRunCmds->Cmds[i]->DownUp)
										keyPressCmd->KeyBuffer[0] = keyPressCmd->KeyBuffer[0] | modifyCode;
									else
										keyPressCmd->KeyBuffer[0] = keyPressCmd->KeyBuffer[0] & (0xFF - modifyCode);
								}
								else
								{//Other
									if (currRunCmds->Cmds[i]->DownUp)
									{//Down	
										for(int n = 2; n < 8; n++)
										{
											if (keyPressCmd->KeyBuffer[n] == currRunCmds->Cmds[i]->KeyCode)
											{
												DEBUG_LOG("[Debug]>>KeyBuffer[n] == currRunCmds->Cmds[i]->KeyCode");
												break;
											}
											if (keyPressCmd->KeyBuffer[n] == 0)
											{
												keyPressCmd->KeyBuffer[n] = (UCHAR)currRunCmds->Cmds[i]->KeyCode;											
												break;
											}
										}
									}
									else
									{//Up							
										for(int n = 2; n < 8; n++)
										{
											
											if (keyPressCmd->KeyBuffer[n] == currRunCmds->Cmds[i]->KeyCode)
											{
												for(int m = n; m < 8; m++)
												{
													if (m == 7 || keyPressCmd->KeyBuffer[m + 1] == 0)
													{
														
														keyPressCmd->KeyBuffer[m] = 0;
														break;
													}
													keyPressCmd->KeyBuffer[m] = keyPressCmd->KeyBuffer[m + 1];
													
												}
												DEBUG_LOG("It's Key Up,KeyBuffer==KeyCode, KeyBuffer[%d] = %d", n,keyPressCmd->KeyBuffer[n]);
												break;
											}
											
										}
									}

								}

								//"Macro Repeat" is too fast and requires Sleep
								if(times >= 1)
									Sleep(50);

								keyPressCmd->isMakeKey = (UCHAR)GetMouseIsMakeKeyStatus(keyPressCmd);
								DEBUG_LOG("DeviceIoControl, isMakeKey=%d", keyPressCmd->isMakeKey);
								DeviceIoControl(hVirtualDrvHandle, IOCTL_SIMULATE_PRESS_KEY, keyPressCmd, sizeof(SIMULATE_KEYPRESS_PACKET), NULL, 0, &dwRet, NULL);
							}

						}
					}
					else if(currRunCmds->MacroMode == 3)//Press and hold the button
					{
						for(int times = 0; times < 300 ; times++)//Press and hold the preset 300 times
						{
							for(int i = 0; i < 255; i++)
							{
								if (currRunCmds->Cmds[i] == NULL)
									break;
								WaitForSingleObject(hVirtualDrvWriteEvent, currRunCmds->Cmds[i]->Delay);
								if (!isVirtualDrvWritingCmd)
									break;
								USHORT modifyCode = GetHidCodeIsModifyCode(currRunCmds->Cmds[i]->KeyCode);
								if (modifyCode != VIRTUAL_KEY_MODIFY_NULL)
								{//Modify Key
									if (currRunCmds->Cmds[i]->DownUp)
										keyPressCmd->KeyBuffer[0] = keyPressCmd->KeyBuffer[0] | modifyCode;
									else
										keyPressCmd->KeyBuffer[0] = keyPressCmd->KeyBuffer[0] & (0xFF - modifyCode);
								}
								else
								{//Other
									if (currRunCmds->Cmds[i]->DownUp)
									{//Down
										for(int n = 2; n < 8; n++)
										{
											if (keyPressCmd->KeyBuffer[n] == currRunCmds->Cmds[i]->KeyCode)
												break;
											if (keyPressCmd->KeyBuffer[n] == 0)
											{
												keyPressCmd->KeyBuffer[n] = (UCHAR)currRunCmds->Cmds[i]->KeyCode;
												break;
											}
										}
									}
									else
									{//Up
										for(int n = 2; n < 8; n++)
										{
											if (keyPressCmd->KeyBuffer[n] == currRunCmds->Cmds[i]->KeyCode)
											{
												for(int m = n; m < 8; m++)
												{
													if (m == 7 || keyPressCmd->KeyBuffer[m + 1] == 0)
													{
														keyPressCmd->KeyBuffer[m] = 0;
														break;
													}
													keyPressCmd->KeyBuffer[m] = keyPressCmd->KeyBuffer[m + 1];
												}
												break;
											}
										}
									}
								}


								//"Macro Repeat" is too fast and requires Sleep
								if(times >= 1)
									Sleep(50);

								keyPressCmd->isMakeKey = (UCHAR)GetMouseIsMakeKeyStatus(keyPressCmd);
								DeviceIoControl(hVirtualDrvHandle, IOCTL_SIMULATE_PRESS_KEY, keyPressCmd, sizeof(SIMULATE_KEYPRESS_PACKET), NULL, 0, &dwRet, NULL);
							}

						}

					}



					if (GetMouseIsMakeKeyStatus(keyPressCmd) != 0)
					{
						for(int i = 0; i < 8; i++)
						{
							keyPressCmd->KeyBuffer[i] = 0;
						}
						keyPressCmd->isMakeKey = 0;
						DeviceIoControl(hVirtualDrvHandle, IOCTL_SIMULATE_PRESS_KEY, keyPressCmd, sizeof(SIMULATE_KEYPRESS_PACKET), NULL, 0, &dwRet, NULL);
					}
				}
				else
				{
					if (currRunCmds->Cmds[0]->DownUp)//Modify Key.
						keyPressCmd->KeyBuffer[0] = (UCHAR)currRunCmds->Cmds[0]->KeyCode;
					else
						keyPressCmd->KeyBuffer[0] = VIRTUAL_KEY_MODIFY_NULL;
					for(int i = 2; i < 8; i++){
						if (currRunCmds->Cmds[i - 1] == NULL)
							break;
						if (currRunCmds->Cmds[0]->DownUp)
							keyPressCmd->KeyBuffer[i] = (UCHAR)currRunCmds->Cmds[i - 1]->KeyCode;
						else
							keyPressCmd->KeyBuffer[i] = 0;
					}
					keyPressCmd->isMakeKey = (UCHAR)GetMouseIsMakeKeyStatus(keyPressCmd);
					DeviceIoControl(hVirtualDrvHandle, IOCTL_SIMULATE_PRESS_KEY, keyPressCmd, sizeof(SIMULATE_KEYPRESS_PACKET), NULL, 0, &dwRet, NULL);
				}
			}
		}
		if (keyPressCmd != NULL){
			free(keyPressCmd);
			keyPressCmd = NULL;
		}
		isVirtualDrvWritingCmd = false;
	}
}

void RunMouseKeyMacro(PMOUSE_MACRO_PACKET mouse, USHORT keyNum)
{
	NotifyLogMessage("Start RunMouseKeyMacro.");
	if (hVirtualDrvHandle == INVALID_HANDLE_VALUE){
		NotifyLogMessage("RunMouseKeyMacro hVirtualDrvHandle is error.");
		return;
	}
	if (hVirtualDrvWaitEvent == NULL)
		hVirtualDrvWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hVirtualDrvWriteEvent == NULL)
		hVirtualDrvWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!hVirtualDrvWaitEvent || !hVirtualDrvWriteEvent){
		NotifyLogMessage("RunMouseKeyMacro hVirtualDrvWaitEvent/hVirtualDrvWriteEvent create fail.");
		return;
	}
	if (virtualThreadHandle == NULL){
		virtualThreadHandle = CreateThread(NULL, 1024, VitualThreadProc, NULL, 0, &virtualThreadId);
	}
	if (isVirtualDrvWritingCmd){
		isVirtualDrvWritingCmd = false;
		SetEvent(hVirtualDrvWriteEvent);
	}
	currRunCmds = NULL;
	for(int i = 0;i < 20; i++){
		if (mouse->Keys[i]->KeyNum == keyNum){
			currRunCmds = mouse->Keys[i];
			break;
		}
	}
	while(!isVirtualDrvWaitingCmd)
		Sleep(1);
	SetEvent(hVirtualDrvWaitEvent);
}

//Anderson add
void StopMouseKeyMacro()
{
	DWORD dwRet;
	PSIMULATE_KEYPRESS_PACKET keyPressCmd = NULL;

	keyPressCmd = (PSIMULATE_KEYPRESS_PACKET)calloc(1, sizeof(SIMULATE_KEYPRESS_PACKET));
	keyPressCmd->Type = VIRTUAL_DEVICE_TYPE_KEYBOARD;

	if (virtualThreadHandle != NULL)
	{
		
		TerminateThread(virtualThreadHandle, 0);
		virtualThreadHandle = NULL;
		//DEBUG_LOG("[Debug]>>End the VitualThreadProc Thread, StopMouseKeyMacro.");
		NotifyLogMessage("[Debug]>>End the VitualThreadProc Thread, StopMouseKeyMacro.");
	
	}

	//Set all key press up   
	for(int i = 0; i < 8; i++)
	{
		keyPressCmd->KeyBuffer[i] = 0;
	}
	keyPressCmd->isMakeKey = 0;
	DeviceIoControl(hVirtualDrvHandle, IOCTL_SIMULATE_PRESS_KEY, keyPressCmd, sizeof(SIMULATE_KEYPRESS_PACKET), NULL, 0, &dwRet, NULL);

	if (keyPressCmd != NULL)
	{
		free(keyPressCmd);
		keyPressCmd = NULL;
	}
}

//Filter Driver.
USHORT GetButtonMacroType(PMOUSE_MACRO_PACKET mouse, USHORT keyNum)
{
	for(int i = 0;i < 20; i++)
	{
		if (mouse->Keys[i] == NULL)
			return 1;
		if (mouse->Keys[i]->KeyNum == keyNum)
			return mouse->Keys[i]->MacroType;
	}
	return 1;
}
bool CheckIsDefaultButton(PMOUSE_MACRO_PACKET mouse, USHORT keyNum)
{
	USHORT tp = GetButtonMacroType(mouse, keyNum);
	return (tp == 1);
}
bool CheckButtonHasMacro(PMOUSE_MACRO_PACKET mouse, USHORT keyNum)
{
	USHORT tp = GetButtonMacroType(mouse, keyNum);
	return (tp == 2 || tp == 3 || tp == 4 || tp == 5);
}
void SetMouseKeyMacroDownUp(PMOUSE_MACRO_PACKET mouse, USHORT keyNum, USHORT downup)
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
void SetMouseFuncDefault(USHORT vid, USHORT pid)
{
	DWORD dwByteRet;
	MOUSESETTINGS	InBuffer;
	InBuffer.VendorID = vid;
	InBuffer.ProductID =pid;
	///DEFAULT_NONEBUTTON (Button is No blocking, system do)
  	InBuffer.nSwapFlag = GetSystemMetrics(SM_SWAPBUTTON);
  	InBuffer.nLeftRightButtonFunc = DEFAULT_NONEBUTTON;
  	InBuffer.nMiddleButtonFunc = DEFAULT_NONEBUTTON;
	InBuffer.nFourButtonFunc = DEFAULT_NONEBUTTON;
	InBuffer.nFiveButtonFunc = DEFAULT_NONEBUTTON;
  	InBuffer.nSixButtonFunc = DEFAULT_NONEBUTTON;
  	InBuffer.nSevenButtonFunc = DEFAULT_NONEBUTTON;
  	InBuffer.nEightButtonFunc = DEFAULT_NONEBUTTON;
  	InBuffer.nNineButtonFunc = DEFAULT_NONEBUTTON;
  	InBuffer.nTenButtonFunc = DEFAULT_NONEBUTTON;
  	InBuffer.nElevenButtonFunc = DEFAULT_NONEBUTTON;
	InBuffer.nOrientationSin = 0;
  	InBuffer.nOrientationCos = 0;
	InBuffer.bLoop = DEFAULT_LOOP;
	InBuffer.bScroll = 0;

	if (!DeviceIoControl(hFilterDrvHandle, IOCTL_MOUSE_SETFUNCTION, (PUCHAR)&InBuffer, sizeof(MOUSESETTINGS), NULL, 0, &dwByteRet, NULL)){
		NotifyLogMessage("SetMouseFunc fail.");
	}else{
		NotifyLogMessage("SetMouseFunc complete.");
	}
}
void SetMouseFunc(USHORT vid, USHORT pid, char * locPath)
{
	DWORD dwByteRet;
	PMOUSE_MACRO_PACKET mouse = GetMouse(vid, pid, locPath);
	if (mouse == NULL)
	{
		NotifyLogMessage("SetMouseFunc Error: GetMouse NULL.");
		return;
	}
	//PMOUSESETTINGS InBuffer = (PMOUSESETTINGS)calloc(1, sizeof(MOUSESETTINGS));
	MOUSESETTINGS	InBuffer;
	InBuffer.VendorID = vid;
	InBuffer.ProductID =pid;
	///DEFAULT_NONEBUTTON (Button is No blocking, system do)
  	InBuffer.nSwapFlag = GetSystemMetrics(SM_SWAPBUTTON);
  	InBuffer.nLeftRightButtonFunc = (CheckIsDefaultButton(mouse, 1) && CheckIsDefaultButton(mouse, 2)) ? DEFAULT_NONEBUTTON : DEFAULT_LEFTRIGHTBUTTON;
  	InBuffer.nMiddleButtonFunc = CheckIsDefaultButton(mouse, 3) ? DEFAULT_NONEBUTTON : DEFAULT_MIDDLEBUTTON;
	InBuffer.nFourButtonFunc = CheckIsDefaultButton(mouse, 4) ? DEFAULT_NONEBUTTON : DEFAULT_FOURBUTTON;
	InBuffer.nFiveButtonFunc = CheckIsDefaultButton(mouse, 5) ? DEFAULT_NONEBUTTON : DEFAULT_FIVEBUTTON;
  	InBuffer.nSixButtonFunc = CheckIsDefaultButton(mouse, 6) ? DEFAULT_NONEBUTTON : DEFAULT_SIXBUTTON;
  	InBuffer.nSevenButtonFunc = CheckIsDefaultButton(mouse, 7) ? DEFAULT_NONEBUTTON : DEFAULT_SEVENBUTTON;
  	InBuffer.nEightButtonFunc = CheckIsDefaultButton(mouse, 8) ? DEFAULT_NONEBUTTON : DEFAULT_EIGHTBUTTON;
  	InBuffer.nNineButtonFunc = CheckIsDefaultButton(mouse, 9) ? DEFAULT_NONEBUTTON : DEFAULT_NINEBUTTON;
  	InBuffer.nTenButtonFunc = CheckIsDefaultButton(mouse, 10) ? DEFAULT_NONEBUTTON : DEFAULT_TENBUTTON;
  	InBuffer.nElevenButtonFunc = CheckIsDefaultButton(mouse, 11) ? DEFAULT_NONEBUTTON : DEFAULT_ELEVENBUTTON;
	InBuffer.nOrientationSin = 0;
  	InBuffer.nOrientationCos = 0;
	InBuffer.bLoop = DEFAULT_LOOP;
	InBuffer.bScroll = 0;
	if (!DeviceIoControl(hFilterDrvHandle, IOCTL_MOUSE_SETFUNCTION, (PUCHAR)&InBuffer, sizeof(MOUSESETTINGS), NULL, 0, &dwByteRet, NULL)){
		NotifyLogMessage("SetMouseFunc fail.");
	}else{
		NotifyLogMessage("SetMouseFunc complete.");
	}
}

DWORD WINAPI FilterThreadProc(LPVOID lpParam)
{
	EVENTPACK					kbEventBuf;
   	CONSUMER_SYS_EVENT_PACKET	MouseDataPacket;
   	DWORD						dwByteRet;

	hFilterDrvEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!hFilterDrvEvent){
		NotifyLogMessage("hFilterDrvEvent create fail.");
		return 0;
	}

	hFilterDrvHandle = CreateFile(FILTER_DRV_NAME, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFilterDrvHandle == INVALID_HANDLE_VALUE)
	{
	    CloseHandle(hFilterDrvEvent);
		hFilterDrvEvent = 0;
		NotifyLogMessage("hFilterDrvHandle create fail.");
		return 0;
	}

	if (!DeviceIoControl(hFilterDrvHandle, IOCTL_MOUSE_RECORD_APP, NULL, 0, NULL, 0, &dwByteRet, NULL))
    {
		CloseHandle(hFilterDrvEvent);
		hFilterDrvEvent = 0;
		CloseHandle(hFilterDrvHandle);
		hFilterDrvHandle = INVALID_HANDLE_VALUE;
		NotifyLogMessage("IOCTL_MOUSE_RECORD_APP fail.");
		return 0;
    }

	//first register
	kbEventBuf.nSwapFlag = (USHORT)GetSystemMetrics(SM_SWAPBUTTON);
    kbEventBuf.hEvent    = hFilterDrvEvent;

    if (!DeviceIoControl(hFilterDrvHandle, IOCTL_MOUSE_REGISTER_EVENT, &kbEventBuf, sizeof(EVENTPACK), NULL, 0, &dwByteRet, NULL))
	{
    	CloseHandle(hFilterDrvEvent);
		hFilterDrvEvent = 0;
		DeviceIoControl(hFilterDrvHandle, IOCTL_MOUSE_DERECORD_APP, NULL, 0, NULL, 0, &dwByteRet, NULL);
        CloseHandle(hFilterDrvHandle);
		hFilterDrvHandle = INVALID_HANDLE_VALUE;
		NotifyLogMessage("First IOCTL_MOUSE_REGISTER_EVENT fail.");
		return 0;
	}
	SetEvent(hOpenDriverWaitEvent);
	while(!isFilterDrvExit)
    {
        kbEventBuf.nSwapFlag = (USHORT)GetSystemMetrics(SM_SWAPBUTTON);
        kbEventBuf.hEvent    = hFilterDrvEvent;
		// wait for driver return the event
		WaitForSingleObject(kbEventBuf.hEvent, INFINITE);
		if (isFilterDrvExit)
			break;
		if (!DeviceIoControl(hFilterDrvHandle, IOCTL_MOUSE_PEEK, NULL, 0, &MouseDataPacket, sizeof(CONSUMER_SYS_EVENT_PACKET), &dwByteRet, NULL)){
			NotifyLogMessage("IOCTL_MOUSE_PEEK fail.");
			break;
		}
		if (!isFilterDrvExit){
			ULONG evtType = MouseDataPacket.Mouse.MouseDataPacket.lParam;
			USHORT btnNum = 0, downup = 0;
			if ((evtType & MOUSE_RIGHT_BUTTON_DOWN || evtType & MOUSE_RIGHT_BUTTON_UP) && kbEventBuf.nSwapFlag == 0){
				btnNum = 2;
				downup = evtType & MOUSE_RIGHT_BUTTON_DOWN ? 1 : 2;	
			}
			else if ((evtType & MOUSE_LEFT_BUTTON_DOWN || evtType & MOUSE_LEFT_BUTTON_UP) && kbEventBuf.nSwapFlag != 0){
				btnNum = 1;
				downup = evtType & MOUSE_LEFT_BUTTON_DOWN ? 1 : 2;
			}
			else if (evtType & MOUSE_MIDDLE_BUTTON_DOWN || evtType & MOUSE_MIDDLE_BUTTON_UP){
				btnNum = 3;
				downup = evtType & MOUSE_MIDDLE_BUTTON_DOWN ? 1 : 2;
			}
			else if (evtType & MOUSE_4BUTTON_DOWN || evtType & MOUSE_4BUTTON_UP){
				btnNum = 4;
				downup = evtType & MOUSE_4BUTTON_DOWN ? 1 : 2;
			}
			else if (evtType & MOUSE_5BUTTON_DOWN || evtType & MOUSE_5BUTTON_UP){
				btnNum = 5;
				downup = evtType & MOUSE_5BUTTON_DOWN ? 1 : 2;
			}
			if (btnNum != 0){
				PMOUSE_MACRO_PACKET mouse = GetMouse(MouseDataPacket.Mouse.VendorID, MouseDataPacket.Mouse.ProductID, "");
				if(mouse != NULL && CheckButtonHasMacro(mouse, btnNum))
				{
					USHORT macroType = GetButtonMacroType(mouse, btnNum);

					DEBUG_LOG("[Debug]>>macroType = %d",macroType);
					DEBUG_LOG("[Debug]>> btnNum[%d-1],ButtonAction = %d",btnNum,mouse->Keys[btnNum-1]->ButtonAction);
					//if ((macroType == 4 || macroType == 2) && downup == 1)
					//Anderson add & modify
					if (macroType == 4 || macroType == 2)
					{
						
						if (downup == 1)//Press or hold the button
							RunMouseKeyMacro(mouse, btnNum);
						else//The key is released(Key Up)
						{
							DEBUG_LOG("[Debug]>> MacroMode = %d",mouse->Keys[btnNum-1]->MacroMode);
							
							if(mouse->Keys[btnNum-1]->MacroMode == 3)//The key is Macro & hold mode
							{
								//DEBUG_LOG("[Debug]>>Send the StopMouseKeyMacro();");
								StopMouseKeyMacro();
							}
						}

					}
					else if (macroType == 3)
					{
						if (downup == 1){
							SetMouseKeyMacroDownUp(mouse, btnNum, 0x80);
						}else{
							SetMouseKeyMacroDownUp(mouse, btnNum, 0x00);
						}
						RunMouseKeyMacro(mouse, btnNum);
					}
					else if (macroType == 5)
					{
						//NotifyLogMessage("[Debug]>>macroType == 5 ");
						
						if(btnNum,mouse->Keys[btnNum-1]->ButtonAction == 1)
						{
							if (downup == 1)
								mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
							else
								mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
						}
						else if(btnNum,mouse->Keys[btnNum-1]->ButtonAction == 2)
						{
							if (downup == 1)
								mouse_event(MOUSEEVENTF_RIGHTDOWN,0,0,0,0);
							else
								mouse_event(MOUSEEVENTF_RIGHTUP,0,0,0,0);	
						}
						else if(btnNum,mouse->Keys[btnNum-1]->ButtonAction == 4)
						{
							if (downup == 1)
								mouse_event(MOUSEEVENTF_MIDDLEDOWN,0,0,0,0);
							else
								mouse_event(MOUSEEVENTF_MIDDLEUP,0,0,0,0);	
						}
						else if(btnNum,mouse->Keys[btnNum-1]->ButtonAction == 8)
						{
							if (downup == 1)
								KeyDown(VK_BROWSER_BACK);
							else
								KeyUp(VK_BROWSER_BACK);	
						}
						else if(btnNum,mouse->Keys[btnNum-1]->ButtonAction == 16)
						{
							if (downup == 1)
								KeyDown(VK_BROWSER_FORWARD);
							else
								KeyUp(VK_BROWSER_FORWARD);
						}
						
						
						
						
					}
				}
			}
		}
		if (isFilterDrvExit)
			break;
		if (!DeviceIoControl(hFilterDrvHandle, IOCTL_MOUSE_REGISTER_EVENT, &kbEventBuf, sizeof(EVENTPACK), NULL, 0, &dwByteRet, NULL))
		{
			NotifyLogMessage("While IOCTL_MOUSE_REGISTER_EVENT fail.");
			break;
		}
    }
    CloseHandle(hFilterDrvEvent);
	hFilterDrvEvent = 0;
	DeviceIoControl(hFilterDrvHandle, IOCTL_MOUSE_DERECORD_APP, NULL, 0, NULL, 0, &dwByteRet, NULL);
    CloseHandle(hFilterDrvHandle);
	hFilterDrvHandle = INVALID_HANDLE_VALUE;
	NotifyLogMessage("FilterThreadProc exit.");
	if (hFilterDrvCloseEvent != NULL)
		SetEvent(hFilterDrvCloseEvent);
	return 0;
}

void CheckDriverInstall(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	HANDLE tmpHandle = INVALID_HANDLE_VALUE;
	bool bReturn = false;

	tmpHandle = CreateFile(VIRTUAL_DRV_NAME, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  	if (tmpHandle != INVALID_HANDLE_VALUE){
    	bReturn = true;
		CloseHandle(tmpHandle);
	}
	if (bReturn){
		bReturn = false;
		tmpHandle = CreateFile(FILTER_DRV_NAME, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	  	if (tmpHandle != INVALID_HANDLE_VALUE){
	    	bReturn = true;
			CloseHandle(tmpHandle);
		}
	}
	tmpHandle = INVALID_HANDLE_VALUE;
	args.GetReturnValue().Set(bReturn);
}

UCHAR WriteReadUSBCmd(USHORT vid, USHORT pid, UCHAR req, USHORT val, USHORT idx, ULONG bufLen, USHORT inOut, char* locPath){
	ULONG nBytes = 0;
	COMMON_REQUEST vReq;

	vReq.VendorID = vid;
	vReq.ProductID = pid;
	vReq.Function = URB_FUNCTION_VENDOR_DEVICE;
	vReq.Request = req;
	vReq.Value = val;
	vReq.Index = idx;
	vReq.RetLength = 0;
	vReq.TransferBufferLength = bufLen;
	vReq.InOut = inOut;

	if(!DeviceIoControl(hUsbCmdHandle, IOCTL_COMMON_REQUEST, (PUCHAR)&vReq, sizeof(COMMON_REQUEST), (PUCHAR)&vReq, sizeof(COMMON_REQUEST), &nBytes, NULL))
	{
		NotifyLogMessage("WriteReadUSBCmd fail.");
		return NULL;
	}
	if(inOut == USBD_TRANSFER_DIRECTION_IN){
		return vReq.TransferBuffer[0];
	}else
		return NULL;
}

void AmmoxSetDPI(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	if (args.Length() != 5) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 3 arguments:[vid, pid, dpi1, dpi2, Location]")));
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

	if (hUsbCmdHandle == INVALID_HANDLE_VALUE){
		hUsbCmdHandle = CreateFile(USB_CMD_DRV_NAME, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hUsbCmdHandle == INVALID_HANDLE_VALUE)                   // open driver fail ?
		{
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"CreateFile failed.")));
			return;
		}
	}

	WriteReadUSBCmd(vid, pid, 0x01, 0x0000, 0x0000, 0, USBD_TRANSFER_DIRECTION_OUT, sLocation);
	WriteReadUSBCmd(vid, pid, 0x01, 0x0100, 0x5a09, 0, USBD_TRANSFER_DIRECTION_OUT, sLocation);
	WriteReadUSBCmd(vid, pid, 0x01, 0x0100, 0x017f, 0, USBD_TRANSFER_DIRECTION_OUT, sLocation);
	//Set 1 & 2 DPI.
	WriteReadUSBCmd(vid, pid, 0x01, 0x0100, dpi1, 0, USBD_TRANSFER_DIRECTION_OUT, sLocation);
	//Set 3 & 4 DPI.
	WriteReadUSBCmd(vid, pid, 0x01, 0x0100, dpi2, 0, USBD_TRANSFER_DIRECTION_OUT, sLocation);
}

void AmmoxGetDPI(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
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

	if (hUsbCmdHandle == INVALID_HANDLE_VALUE){
		hUsbCmdHandle = CreateFile(USB_CMD_DRV_NAME, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hUsbCmdHandle == INVALID_HANDLE_VALUE)                   // open driver fail ?
		{
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"CreateFile failed.")));
			return;
		}
	}

	WriteReadUSBCmd(vid, pid, 0x01, 0x0000, 0x0000, 0, USBD_TRANSFER_DIRECTION_OUT, sLocation);
	WriteReadUSBCmd(vid, pid, 0x01, 0x0100, 0x5a09, 0, USBD_TRANSFER_DIRECTION_OUT, sLocation);
	WriteReadUSBCmd(vid, pid, 0x01, 0x0100, 0x017f, 0, USBD_TRANSFER_DIRECTION_OUT, sLocation);
	//Get 1 & 2 DPI.
	UCHAR dpi1 = WriteReadUSBCmd(vid, pid, 0x01, 0x0100, 0x0010, 1, USBD_TRANSFER_DIRECTION_IN, sLocation);
	//Get 3 & 4 DPI.
	UCHAR dpi2 = WriteReadUSBCmd(vid, pid, 0x01, 0x0100, 0x0011, 1, USBD_TRANSFER_DIRECTION_IN, sLocation);
	Handle<Object> result = Object::New(isolate);
    result->Set(String::NewFromUtf8(isolate,"DPI1"), Integer::New(isolate, dpi1));
    result->Set(String::NewFromUtf8(isolate,"DPI2"), Integer::New(isolate, dpi2));
    args.GetReturnValue().Set(result);
}

void GetDriverVersion(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
	DWORD dwSize = 0, dwHandle = 0;
	LPVOID lpVersionData;
	VS_FIXEDFILEINFO *lpvs;
	unsigned int nLen;
	TCHAR szPath[255] = {0};
	char buf[128] = {0};

    GetSystemDirectory(szPath, 255);
	lstrcat(szPath, "\\drivers\\gFilterMouUsb.sys");

	dwSize = GetFileVersionInfoSize(szPath, &dwHandle);
	if (dwSize){
		lpVersionData = GlobalAlloc(GPTR, dwSize + 0x10);
		GetFileVersionInfo(szPath, dwHandle, dwSize, lpVersionData);
		VerQueryValue(lpVersionData, "\\", (LPVOID *)&lpvs, &nLen);
		GlobalFree(lpVersionData);
		sprintf_s(buf, "%d.%d.%d.%d",HIWORD(lpvs->dwFileVersionMS), LOWORD(lpvs->dwFileVersionMS),HIWORD(lpvs->dwFileVersionLS), LOWORD(lpvs->dwFileVersionLS));
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, buf));
	}
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
	NODE_SET_METHOD(exports, "GetDriverVersion", GetDriverVersion);

	NODE_SET_METHOD(exports, "RegisterLogMessage", RegisterLogMessage);

	ZeroMemory(mouseMacros, sizeof(mouseMacros));

	NODE_SET_METHOD(exports, "OpenKbDriver", OpenKbDriver );
  	NODE_SET_METHOD(exports, "CloseKbDriver", CloseKbDriver);
	NODE_SET_METHOD(exports, "SetKbFunctionAction", SetKbFunctionAction);
	ZeroMemory(KeyboardInfo, sizeof(KeyboardInfo));
  }

  NODE_MODULE(driverWin, Init)
}

void OpenDriver(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);

	if (hVirtualDrvHandle == INVALID_HANDLE_VALUE){
		if (!OpenVirtualDriver()) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"OpenVirtualDriver failed.")));
			return;
		}
	}
	isFilterDrvExit = false;
	if (hOpenDriverWaitEvent == NULL)
		hOpenDriverWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hFilterDrvHandle == INVALID_HANDLE_VALUE){
		if (filterThreadHandle != NULL){
			TerminateThread(filterThreadHandle, 0);
			filterThreadHandle = NULL;
		}
		filterThreadHandle = CreateThread(NULL, 1024, FilterThreadProc, NULL, 0, &filterThreadId);
		WaitForSingleObject(hOpenDriverWaitEvent, 2000);
		if (hFilterDrvHandle == INVALID_HANDLE_VALUE){
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"OpenFilterDriver failed.")));
			return;
		}
	}
}
void CloseDriver(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);

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
		SetMouseFuncDefault(mouseMacros[i]->VID, mouseMacros[i]->PID);
		free(mouseMacros[i]);
		mouseMacros[i] = NULL;
	}

	hFilterDrvCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	isFilterDrvExit = true;
	SetEvent(hFilterDrvEvent);
	WaitForSingleObject(hFilterDrvCloseEvent, 1000);
	CloseVirtualDriver();
	if (hFilterDrvCloseEvent){
		CloseHandle(hFilterDrvCloseEvent);
		hFilterDrvCloseEvent = NULL;
	}
	if (hOpenDriverWaitEvent){
		CloseHandle(hOpenDriverWaitEvent);
		hOpenDriverWaitEvent = NULL;
	}
	if(hUsbCmdHandle){
		CloseHandle(hUsbCmdHandle);
		hUsbCmdHandle = INVALID_HANDLE_VALUE;
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

void NotifyLogMessageCallback(TCHAR * msg){
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
	TCHAR * msg = static_cast<TCHAR *>(req->data);
  	NotifyLogMessageCallback(msg);
}

void NotifyLogMessage(TCHAR * msg){
  	if (isLogMessageRegistered) {
	    uv_work_t* reqLogChg = new uv_work_t();
		reqLogChg->data = msg;
        uv_queue_work(uv_default_loop(), reqLogChg, NotifyLogMessageAsync, (uv_after_work_cb)NotifyLogMessageFinished);
  	}else{
	  	DEBUG_LOG(msg);
  	}
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

	PMOUSE_MACRO_PACKET mouse = GetMouse(vid, pid, sLocation);
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
		//Anderson add
		key->ButtonAction = item->Get(String::NewFromUtf8(isolate,"ButtonAction"))->Uint32Value();
		FreeMouseKeyMacro(key);

		Local<Value> arrMacroVal = item->Get(String::NewFromUtf8(isolate,"MacroContent"));
		if (!arrMacroVal->IsUndefined()){
			Local<Array> arrMacro = Local<Array>::Cast(arrMacroVal);
			
			
			DEBUG_LOG("[Debug]>>######  Macro Info ######");
			key->MacroMode = arrMacro->Get(String::NewFromUtf8(isolate,"MacroMode"))->Uint32Value();
			key->MacroRepeatTimes = arrMacro->Get(String::NewFromUtf8(isolate,"MacroRepeatTimes"))->Uint32Value();		
			DEBUG_LOG("[Debug]>>key->MacroMode = %d",key->MacroMode);
			DEBUG_LOG("[Debug]>>key->MacroRepeatTimes = %d",key->MacroRepeatTimes);
			


			if (key->MacroType == 4){
				for(int n = 0; n < 255; n++)
				{
					if(arrMacro->Get(n)->IsUndefined())
						break;
					key->Cmds[n] = (PCMD_PACKET)calloc(1, sizeof(CMD_PACKET));
					Local<Object> itemMacro = arrMacro->Get(n)->ToObject();
					key->Cmds[n]->DownUp = itemMacro->Get(String::NewFromUtf8(isolate,"DownUp"))->Uint32Value();
					key->Cmds[n]->Delay = itemMacro->Get(String::NewFromUtf8(isolate,"Delay"))->Uint32Value();
					key->Cmds[n]->KeyCode = itemMacro->Get(String::NewFromUtf8(isolate,"KeyCode"))->Uint32Value();
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
	NotifyLogMessage("SetMacroKey Complete.");
}

void SetAllMacroKey(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);

	NotifyLogMessage("SetMacroKey Complete.");
}

/////////////Win32 Kb Driver//////////////////////////////
void LaunchDefaultMail()
{
	TCHAR wszBuffer[MAX_PATH] = {0};
	TCHAR strDefMailPath[MAX_PATH] = {0};

	TCHAR data[MAX_PATH]= {0};
	HKEY	hKey = NULL;
	DWORD dwType = REG_SZ;
	DWORD TempSize;
	DWORD dwSize = MAX_PATH;
	TCHAR MailPath[MAX_PATH]= {0};


	DEBUG_LOG("[Debug]>>LaunchDefaultMail() ");



	strcpy(wszBuffer,"Software\\Clients\\Mail");
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,wszBuffer,NULL,KEY_READ ,&hKey))
	{
			if(ERROR_SUCCESS == RegQueryValueEx(hKey,NULL,NULL,&dwType,(LPBYTE)data,&TempSize))
			{
				DEBUG_LOG("[Debug]>>RegGetValue SUCCESS,data=%s\n",data);
					if(data[0] != NULL)
					{
						strcat(wszBuffer,"\\");
						strcat(wszBuffer,data);
					}
					else
					{
						//If it is emptyempty，open Microsoft Outlook
						strcat(wszBuffer,"\\Microsoft Outlook");
					}
			}
	}
	else
	{
		DEBUG_LOG("[Debug]>>RegGetValue [Software\\Clients\\Media] Error ");
	}


	dwType = NULL;
	dwSize = MAX_PATH;
	if( 0 == strcmp(data,"Microsoft Outlook") )
	{
			strcat(wszBuffer,"\\shell\\open\\command");
			DEBUG_LOG("[Debug]>>wszBuffer=%s",wszBuffer);

			if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,wszBuffer,NULL,KEY_READ ,&hKey))
			{
					if(ERROR_SUCCESS == RegQueryValueEx(hKey,NULL,NULL,&dwType,(LPBYTE)MailPath,&dwSize))
					{
							//Sleep(100);
							DEBUG_LOG("[Debug]>>RegGetValue,MailPath=%s\n",MailPath);
					}
					else
					{
							DEBUG_LOG("[Debug]>>RegQueryValueEx [\\shell\\open\\command]  Error");
					}
			}
			else
			{
					DEBUG_LOG("[Debug]>>RegGetValue [Software\\Clients\\Mail\\shell\\open\\command] Error ");
			}
	}
	else if( 0 == strcmp(data,"Windows Mail") )
	{
			strcat(wszBuffer,"\\InstallInfo");
	}



	RegCloseKey(hKey);

	TCHAR TempBuf[255]={0};
	TCHAR *loc = strstr(MailPath,".EXE");
	int n = 0;

	if( NULL != loc )
	{
		n = loc-MailPath;
		memcpy(TempBuf, MailPath,n+5);
		DEBUG_LOG("[Debug]>>>>>>>>>>>> TempBuf= %s\n",TempBuf);
	}



	ExpandEnvironmentStrings(TempBuf, strDefMailPath, 255);
	//ExpandEnvironmentStrings("C:\\PROGRA~2\\MICROS~1\\Office14\\OUTLOOK.EXE", strDefMailPath, 255);
	DEBUG_LOG("[Debug]>>>>>>>>>>>> strDefMailPath= %s\n",strDefMailPath);


	ShellExecute(
							NULL,
							"open",
							strDefMailPath,
							NULL,
							NULL,
							SW_NORMAL
						);

}

void LaunchDefaultMedia()
{
		TCHAR wszBuffer[MAX_PATH] = {0};
		TCHAR strDefMediaPath[MAX_PATH] = {0};

		TCHAR data[MAX_PATH]= {0};
		HKEY	hKey = NULL;;
		DWORD dwType = REG_SZ;
		DWORD dwSize = MAX_PATH;
		TCHAR MusicPath[MAX_PATH]= {0};


		DEBUG_LOG("[Debug]>>LaunchDefaultMedia() ");

		/*
		strcpy(wszBuffer,_T("Software\\Clients\\Media"));
		if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,wszBuffer,NULL,KEY_READ ,&hKey))
		{
				if(ERROR_SUCCESS == RegQueryValueEx(hKey,NULL,NULL,&dwType,(LPBYTE)data,&dwSize))
				{
						DEBUG_LOG("[Debug]>>RegGetValue SUCCESS,data=%s",data);
						if(data[0] != NULL)
						{
							strcat(wszBuffer,"\\");
							strcat(wszBuffer,data);
						}
						else
						{
							//If it is emptyempty，open Windows Media Player
							strcat(wszBuffer,"\\Windows Media Player");
						}
				}
		}
		else
		{
			DEBUG_LOG("[Debug]>>RegGetValue [Software\\Clients\\Media] Error ");
		}
		*/

		//dwType = NULL;
		//dwSize =255;
		strcat(wszBuffer,"Software\\Clients\\Media\\Windows Media Player\\shell\\open\\command");
		//DEBUG_LOG("[Debug]>>wszBuffer=%s",wszBuffer);

		if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,wszBuffer,NULL,KEY_READ ,&hKey))
		{
				if(ERROR_SUCCESS == RegQueryValueEx(hKey,NULL,NULL,&dwType,(LPBYTE)MusicPath,&dwSize))
				{
					DEBUG_LOG("[Debug]>>RegGetValue,MusicPath=%s",MusicPath);
				}
				else
				{
						DEBUG_LOG("[Debug]>>RegQueryValueEx [\\shell\\open\\command]  Error");
				}
		}
		else
		{
			DEBUG_LOG("[Debug]>>RegGetValue [Software\\Clients\\Media\\shell\\open\\command] Error ");
		}

		RegCloseKey(hKey);




		ExpandEnvironmentStrings(MusicPath, strDefMediaPath, 255);
		DEBUG_LOG("[Debug]>>>>>>>>>>>> strDefMediaPath= %s\n",strDefMediaPath);

		/*
		if( NULL != strstr(MusicPath, "%ProgramFiles%") )
		{
				strcpy(strDefMediaPath,"C:\\Program Files");
				int nWord = sizeof("%ProgramFiles%");
				strcat(strDefMediaPath, MusicPath +nWord-1);
				DEBUG_LOG("[Debug]>>RegGetValue>> strDefMediaPath= %s\n",strDefMediaPath);
    }
		else if( NULL != strstr(MusicPath, "%ProgramFiles(x86)%") )
		{
				//DEBUG_LOG("[Debug]>>RegGetValue>> Find, loc=%d\n",loc - MusicPath);
				strcpy(strDefMediaPath,"C:\\Program Files (x86)");
				int nWord = sizeof("%ProgramFiles(x86)%");
				strcat(strDefMediaPath, MusicPath +nWord-1);
				DEBUG_LOG("[Debug]>>RegGetValue>> strDefMediaPath= %s\n",strDefMediaPath);
		}
		else
		{
				strcpy(strDefMediaPath,MusicPath);
				DEBUG_LOG("[Debug]>>RegGetValue>> No Find>> strDefMediaPath= %s\n",strDefMediaPath);
		}
*/

		ShellExecute(
								NULL,
								"open",
								strDefMediaPath,
								NULL,
								NULL,
								SW_NORMAL
							);



}


void LaunchDefaultBrowser(char *WebSite)
{
		TCHAR wszBuffer[255] = {0};


		TCHAR data[255]= {0};
		HKEY	hKey = NULL;;
		DWORD dwType = REG_SZ;
		DWORD dwSize = MAX_PATH;
		TCHAR BrowserPath[255]= {0};


		DEBUG_LOG("[Debug]>>LaunchDefaultBrowser() ");


				strcpy(wszBuffer,_T("Software\\Clients\\StartMenuInternet"));
				if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,wszBuffer,NULL,KEY_READ ,&hKey))
			  {
						if(ERROR_SUCCESS == RegQueryValueEx(hKey,NULL,NULL,&dwType,(LPBYTE)data,&dwSize))
						{
								DEBUG_LOG("[Debug]>>RegGetValue SUCCESS,data=%s",data);
								if(data[0] != NULL)
								{
									strcat(wszBuffer,"\\");
									strcat(wszBuffer,data);
								}
								else
								{
									//If it is emptyempty，open IE browser
									strcat(wszBuffer,"\\IEXPLORE.EXE");
								}
						}
        }
				else
				{
					DEBUG_LOG("[Debug]>>RegGetValue [Software\\Clients\\StartMenuInternet] Error ");
				}


				dwType = NULL;
				dwSize = MAX_PATH;
				strcat(wszBuffer,"\\shell\\open\\command");
				DEBUG_LOG("[Debug]>>wszBuffer=%s",wszBuffer);

				if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,wszBuffer,NULL,KEY_READ ,&hKey))
				{
						if(ERROR_SUCCESS == RegQueryValueEx(hKey,NULL,NULL,&dwType,(LPBYTE)BrowserPath,&dwSize))
						{
							DEBUG_LOG("[Debug]>>RegGetValue,BrowserPath=%s",BrowserPath);
							//strcpy(strDefBrowserPos,BrowserPath);
						//	DEBUG_LOG("[Debug]>>RegGetValue,strDefBrowserPos=%s",strDefBrowserPos);
						}
						else
						{
								DEBUG_LOG("[Debug]>>RegQueryValueEx [\\shell\\open\\command]  Error");
						}
				}
				else
				{
					DEBUG_LOG("[Debug]>>RegGetValue [Software\\Clients\\StartMenuInternet\\shell\\open\\command] Error ");
				}

				RegCloseKey(hKey);


				if(WebSite != NULL)
				{
					//	strcat(BrowserPath," ");
					//	strcat(BrowserPath,WebSite);
						DEBUG_LOG("[Debug]>>WebSite != NULL>> WebSite=%s",WebSite)
						ShellExecute(
								        NULL,
								        "open",
								        BrowserPath,
								        WebSite,
								        NULL,
								        SW_NORMAL
								        );
				}
				else
				{
						//Open browser
			    	ShellExecute(
								        NULL,
								        "open",
								        BrowserPath,
								        NULL,
								        NULL,
								        SW_NORMAL
								        );
				}

}



PKEYBOARD_PACKET GetKeyboard(USHORT vid, USHORT pid, char * locPath)
{
	for (int i = 0; i < MAX_KEYBOARD_QTY; i++)
	{
		if(KeyboardInfo[i] == NULL)
		{
			KeyboardInfo[i] = (PKEYBOARD_PACKET)calloc(1, sizeof(KEYBOARD_PACKET));
			KeyboardInfo[i]->VID = vid;
			KeyboardInfo[i]->PID = pid;
			strcpy(KeyboardInfo[i]->locPath, locPath);
			DEBUG_LOG("GetKeyboard>> KeyboardInfo[%d]==NULL, PID:0x%04x ,locPath=%s ,first create device data",i,KeyboardInfo[i]->PID,KeyboardInfo[i]->locPath);
			return KeyboardInfo[i];
		}
		if (KeyboardInfo[i]->VID == vid && KeyboardInfo[i]->PID == pid && stricmp(KeyboardInfo[i]->locPath, locPath) == 0)
		{
			DEBUG_LOG("GetKeyboard>> KeyboardInfo[%d]!=NULL, PID:0x%04x ,locPath=%s ,match the before device data",i,KeyboardInfo[i]->PID,KeyboardInfo[i]->locPath);
			return KeyboardInfo[i];
		}
	}
	return NULL;
}

PKEY_FUNCTION_PACKET GetKeyboardKey(PKEYBOARD_PACKET Keyboard,USHORT Index)
{
	if (Keyboard == NULL)
		return NULL;
	for (int i = 0; i < 12; i++) //12: F1~F12
	{
		if(Keyboard->Keys[i] == NULL)
		{
			Keyboard->Keys[i] = (PKEY_FUNCTION_PACKET)calloc(1, sizeof(KEY_FUNCTION_PACKET));
			Keyboard->Keys[i]->KeyIndex = Index;
			DEBUG_LOG("GetKeyboard>> KeyIndex == NULL, return Keyboard->Keys[%d]",i);
			return Keyboard->Keys[i];
		}
		if (Keyboard->Keys[i]->KeyIndex == Index)
		{
			DEBUG_LOG("GetKeyboard>> KeyIndex == Index, return Keyboard->Keys[%d]",i);
			return Keyboard->Keys[i];
		}
	}
	return NULL;
}


void SetKbFunctionAction(const v8::FunctionCallbackInfo<Value>& args)
{
		Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
		//char       buf[200]={0};
		NotifyLogMessage("##Test Log## win32Driver.cpp >>> SetKbFunctionAction start.");

		if (args.Length() != 5) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"KbFunctionList can only be 5 arguments:[vid, pid, macro, sn, Location]")));
			NotifyLogMessage("##Test Log## win32Driver.cpp >>>args.Length() != 5, return.");
			return;
		}

		if (!args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsObject() || !args[3]->IsString() || !args[4]->IsString()) {
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"KbFunctionList ,argument type can only be Int32,Int32,Object,String,String.")));
			NotifyLogMessage("##Test Log## win32Driver.cpp >>> !args[2]->IsObject() return.");
			return;
		}

		uint32_t vid = args[0]->Uint32Value();
		uint32_t pid = args[1]->Uint32Value();

		char sLocation[255] = {0};
		String::Utf8Value valLocation(args[4]->ToString());
		int DataLen = valLocation.length();
		strncpy(sLocation, *valLocation, DataLen);


		DEBUG_LOG("[Debug]>>SetKbFunctionAction()>>  VID:0x%04x, PID:0x%04x LocationPath:%s",vid,pid,sLocation);

		PKEYBOARD_PACKET keyboard = GetKeyboard(vid, pid, sLocation);


		if (keyboard == NULL){
			isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"GetKeyboard is NULL.")));
			return;
		}




		Local<Object> obj = args[2]->ToObject();
		Local<Array> ActionFunc = Local<Array>::Cast(obj);
		for (int i = 0; i < 12; i++) //12: F1~F12
		{
				if(ActionFunc->Get(i)->IsUndefined())
					break;

				Local<Object> item = ActionFunc->Get(i)->ToObject();

				USHORT Index = i;
				PKEY_FUNCTION_PACKET key = GetKeyboardKey(keyboard,Index);

				String::Utf8Value val(item->Get(String::NewFromUtf8(isolate,"keyName"))->ToString());
				DataLen = val.length();
				strncpy(key->KeyName, *val, DataLen);
				key->Group = item->Get(String::NewFromUtf8(isolate,"KeyGroup"))->Uint32Value();
				key->Function = item->Get(String::NewFromUtf8(isolate,"Function"))->Uint32Value();

		}

	}






void FunctionAction(USHORT iDevice ,USHORT KeyIndex)
{

	int ActionKeyType = 0;

	DWORD       dwRet;
	PSIMULATE_PACKET macroKeyCmd = NULL;
	macroKeyCmd = (PSIMULATE_PACKET)calloc(1, sizeof(SIMULATE_REQ));
	macroKeyCmd->Type = VIRTUAL_DEVICE_TYPE_KEYBOARD;
	macroKeyCmd->KeyNumber = 0;


	/*for(int i = 1; i < 255; i++){
		if (currRunCmds->Cmds[i] == NULL)
			break;
		macroKeyCmd->Data[i-1][0] = currRunCmds->Cmds[0]->KeyCode;
		macroKeyCmd->Data[i-1][1] = currRunCmds->Cmds[i]->KeyCode;
		macroKeyCmd->KeyNumber++;
	}*/

	PSIMULATE_KEYPRESS_PACKET keyPressCmd = NULL;
	keyPressCmd = (PSIMULATE_KEYPRESS_PACKET)calloc(1, sizeof(SIMULATE_KEYPRESS_PACKET));
	keyPressCmd->Type = VIRTUAL_DEVICE_TYPE_KEYBOARD;


	DEBUG_LOG("[Debug]>>KbPeekData()>>FunctionAction\r\n \
	VID=0x%04x,PID=0x%04x,locPath=%s,\r\n \
	iDevice=%d,KeyIndex=%d,\r\n \
	KeyName=%s \r\n \
	Group=%d,\r\n  \
	Function:%d \r\n" \
	,KeyboardInfo[iDevice]->VID,KeyboardInfo[iDevice]->PID,KeyboardInfo[iDevice]->locPath,iDevice,KeyIndex,\
	KeyboardInfo[iDevice]->Keys[KeyIndex]->KeyName,KeyboardInfo[iDevice]->Keys[KeyIndex]->Group,KeyboardInfo[iDevice]->Keys[KeyIndex]->Function);

	switch(KeyboardInfo[iDevice]->Keys[KeyIndex]->Group)
	{
		case 1: //Media Group 1
				if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 1)
				{
						DEBUG_LOG("[Debug]>>FunctionAction>> Paly/Pause");
						KeyDown(VK_MEDIA_PLAY_PAUSE);
						KeyUp(VK_MEDIA_PLAY_PAUSE);
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 2)
				{
						DEBUG_LOG("[Debug]>>FunctionAction>> PREV_TRACK");
						KeyDown(VK_MEDIA_PREV_TRACK);
						KeyUp(VK_MEDIA_PREV_TRACK);
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 3)
				{
						DEBUG_LOG("[Debug]>>FunctionAction>> NEXT_TRACK");
						KeyDown(VK_MEDIA_NEXT_TRACK);
						KeyUp(VK_MEDIA_NEXT_TRACK);
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 4)
				{
						DEBUG_LOG("[Debug]>>FunctionAction>> Music Fast Forward");

				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 5)
				{
						DEBUG_LOG("[Debug]>>FunctionAction>> Music rotation");

				}


				break;
		case 2: //Group 2
				if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 1)
				{
						DEBUG_LOG("[Debug]>>FunctionAction>> Volume Up");
						KeyDown(VK_VOLUME_UP);
						KeyUp(VK_VOLUME_UP);

				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 2)
				{
						DEBUG_LOG("[Debug]>>FunctionAction>> Volume Down");
						KeyDown(VK_VOLUME_DOWN);
						KeyUp(VK_VOLUME_DOWN);
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 3)
				{
						DEBUG_LOG("[Debug]>>FunctionAction>> Mute");
						KeyDown(VK_VOLUME_MUTE);
						KeyUp(VK_VOLUME_MUTE);
				}


				break;
		case 3: //Group 3
				if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 1)
				{
					//Open Default browser
					LaunchDefaultBrowser(0);
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 2)
				{
						LaunchDefaultMedia();
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 3)
				{
						LaunchDefaultMail();
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 4)
				{
						DEBUG_LOG("[Debug]>>KbPeekData()>>Win+F");
					macroKeyCmd->Data[0][0] =VIRTUAL_WIN_KEY;
					macroKeyCmd->Data[0][1] =0x09;//F
					macroKeyCmd->KeyNumber = 1;
					ActionKeyType = 1;
				}


				break;
		case 4:	//Group 4
				if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 1)
				{
					macroKeyCmd->Data[0][0] =VIRTUAL_WIN_KEY;
					macroKeyCmd->Data[0][1] =0x07;//D
					macroKeyCmd->KeyNumber = 1;
					ActionKeyType = 1;
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 2)
				{
					macroKeyCmd->Data[0][0] =VIRTUAL_WIN_KEY;
					macroKeyCmd->Data[0][1] =0x51;//Down
					macroKeyCmd->KeyNumber = 1;
					ActionKeyType = 1;
				}

				break;
		case 5: //Group 5
				if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 1)//我的電腦
				{
					macroKeyCmd->Data[0][0] =VIRTUAL_WIN_KEY;
					macroKeyCmd->Data[0][1] =0x08;//E
					macroKeyCmd->KeyNumber = 1;
					ActionKeyType = 1;
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 2)//鎖定PC(登出)
				{
					macroKeyCmd->Data[0][0] =VIRTUAL_WIN_KEY;
					macroKeyCmd->Data[0][1] =0x0F;//L
					macroKeyCmd->KeyNumber = 1;
					ActionKeyType = 1;
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 3)//Facebook
				{
					LaunchDefaultBrowser("www.facebook.com");
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 4)//youtube
				{
					LaunchDefaultBrowser("www.youtube.com");
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 5)//google map
				{
					LaunchDefaultBrowser("www.google.com/maps");
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 6)//Amaz0n
				{
					LaunchDefaultBrowser("www.amazon.com");
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 7)//Twitter
				{
					LaunchDefaultBrowser("twitter.com");
				}
				else if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 8)//Linkedin
				{
					LaunchDefaultBrowser("www.linkedin.com");
				}


				break;
		default: 	//Group 0
			DEBUG_LOG("[Debug]>>KbPeekData()>>FunctionAction,Group default == 0");
			if(KeyboardInfo[iDevice]->Keys[KeyIndex]->Function == 0)

				if(KeyIndex == 0)//F1
				{
					macroKeyCmd->Data[0][0] =0;
					macroKeyCmd->Data[0][1] =0x3a;//F1
					macroKeyCmd->KeyNumber = 1;
				}
				else if(KeyIndex == 1)//F2
				{
					macroKeyCmd->Data[0][0] =0;
					macroKeyCmd->Data[0][1] =0x3b;//F2
					macroKeyCmd->KeyNumber = 1;
				}
				else if(KeyIndex == 2)//F3
				{
					macroKeyCmd->Data[0][0] =0;
					macroKeyCmd->Data[0][1] =0x3c;//F3
					macroKeyCmd->KeyNumber = 1;
				}
				else if(KeyIndex == 3)//F4
				{
					macroKeyCmd->Data[0][0] =0;
					macroKeyCmd->Data[0][1] =0x3d;//F4
					macroKeyCmd->KeyNumber = 1;
				}
				else if(KeyIndex == 4)//F5
				{
					macroKeyCmd->Data[0][0] =0;
					macroKeyCmd->Data[0][1] =0x3e;//F5
					macroKeyCmd->KeyNumber = 1;
				}
				else if(KeyIndex == 5)//F6
				{
					macroKeyCmd->Data[0][0] =0;
					macroKeyCmd->Data[0][1] =0x3f;//F6
					macroKeyCmd->KeyNumber = 1;
					ActionKeyType = 1;
				}
				else if(KeyIndex == 6)//F7
				{
					macroKeyCmd->Data[0][0] =0;
					macroKeyCmd->Data[0][1] =0x40;//F7
					macroKeyCmd->KeyNumber = 1;
				}
				else if(KeyIndex == 7)//F8
				{
					macroKeyCmd->Data[0][0] =0;
					macroKeyCmd->Data[0][1] =0x41;//F8
					macroKeyCmd->KeyNumber = 1;
				}
				else if(KeyIndex == 8)//F9
				{
					macroKeyCmd->Data[0][0] =0;
					macroKeyCmd->Data[0][1] =0x42;//F9
					macroKeyCmd->KeyNumber = 1;
				}
				else if(KeyIndex == 9)//F10
				{
					macroKeyCmd->Data[0][0] =0;
					macroKeyCmd->Data[0][1] =0x43;//F10
					macroKeyCmd->KeyNumber = 1;
				}
				else if(KeyIndex == 10)//F11
				{
					macroKeyCmd->Data[0][0] =0;
					macroKeyCmd->Data[0][1] =0x44;//F11
					macroKeyCmd->KeyNumber = 1;
				}
				else if(KeyIndex == 11)//F12
				{
					macroKeyCmd->Data[0][0] =0;
					macroKeyCmd->Data[0][1] =0x45;//F12
					macroKeyCmd->KeyNumber = 1;
				}
				ActionKeyType = 1;
				break;



	}

	if(ActionKeyType == 1)
	{
		DEBUG_LOG("[Debug]>>KbPeekData()>>FunctionAction,to DeviceIoControl and free(keyPressCmd).");
		//DeviceIoControl(hVirtualDrvHandle, IOCTL_SIMULATE_PRESS_KEY, keyPressCmd, sizeof(SIMULATE_KEYPRESS_PACKET), NULL, 0, &dwRet, NULL);
		DeviceIoControl(hVirtualDrvHandle, IOCTL_SIMULATE_MACRO_KEY, macroKeyCmd, sizeof(SIMULATE_REQ), NULL, 0, &dwRet, NULL);
		free(keyPressCmd);
		keyPressCmd = NULL;
	}

}

DWORD WINAPI KbPeekData(LPVOID arg)
{
	DWORD       dwRet;
	static DWORD preT;
	SHORT iDevice =-1;
	SHORT iKey=0;

	if (hHandle != INVALID_HANDLE_VALUE)
	{
		if (!(pKbDevEvent.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL)))
		{
			CloseHandle(hHandle);
			ExitThread(0);
			return  TRUE;
		}

		ThreadAlive = TRUE;
		pKbDevEvent.hDevExist = 1;

		while (ThreadAlive)
		{
			if (DeviceIoControl(hHandle,IOCTL_REGISTER_EVENT,
								&pKbDevEvent,sizeof(EVENTPACK),
								NULL,0,&dwRet,NULL) == FALSE)
			{
				CloseHandle(pKbDevEvent.hEvent);
				CloseHandle(hHandle);
				ExitThread(0);
				return TRUE;
			}

			WaitForSingleObject(pKbDevEvent.hEvent,INFINITE);
			if (!ThreadAlive)
				break;

			if (DeviceIoControl(hHandle,IOCTL_PEEK_DATA,
								&pKbDevEvent,sizeof(EVENTPACK),
								&DeviceData,sizeof(DEVICE_DATA),
								&dwRet,NULL))
			{



				for (int i = 0; i < MAX_KEYBOARD_QTY; i++)
				{
					if(KeyboardInfo[i] == NULL)
					{
							break;
					}
					else
					{
							if(KeyboardInfo[i]->VID == DeviceData.devVID && KeyboardInfo[i]->PID == DeviceData.devPID)
							{
								iDevice = i;
								break;
							}
							else
									continue;
					}
				}

				char MultName [128];
        WideCharToMultiByte ( CP_ACP, WC_COMPOSITECHECK,
                                             DeviceData.drvName, -1,
                                             MultName, sizeof(MultName),
                                              NULL, NULL );

				char MulthwID [128];
				WideCharToMultiByte ( CP_ACP, WC_COMPOSITECHECK,
																	DeviceData.hwID, -1,
																	MulthwID, sizeof(MulthwID),
																	NULL, NULL );

				DEBUG_LOG("[Debug]>>KbPeekData()>>iDevice:%d Have key press,\n \
				VID=0x%04x,PID=0x%04x,\n DriverName=%s\n \
				nHardwareID=%s,\n Driver ID : %d",iDevice,DeviceData.devVID,DeviceData.devPID,MultName,MulthwID,DeviceData.drvID);


				if(DeviceData.keyData[2] == 0x3a)//Press F1
				{
					iKey = 0;
					DEBUG_LOG("[Debug]>>KbPeekData()>> Press F1");
					FunctionAction(iDevice ,iKey);
				}
				else if(DeviceData.keyData[2] == 0x3b)//F2
				{
					iKey = 1;
					DEBUG_LOG("[Debug]>>KbPeekData()>> Press F2");
					FunctionAction(iDevice ,iKey);
				}
				else if(DeviceData.keyData[2] == 0x3c)//F3
				{
					iKey = 2;
					DEBUG_LOG("[Debug]>>KbPeekData()>> Press F3");
					FunctionAction(iDevice ,iKey);
				}
				else if(DeviceData.keyData[2] == 0x3d)//F4
				{
					iKey = 3;
					DEBUG_LOG("[Debug]>>KbPeekData()>> Press F4");
					FunctionAction(iDevice ,iKey);
				}
				else if(DeviceData.keyData[2] == 0x3e)//F5
				{
					iKey = 4;
					DEBUG_LOG("[Debug]>>KbPeekData()>> Press F5");
					FunctionAction(iDevice ,iKey);
				}
				else if(DeviceData.keyData[2] == 0x3f)//F6
				{
					iKey = 5;
					DEBUG_LOG("[Debug]>>KbPeekData()>> Press F6");
					FunctionAction(iDevice ,iKey);
				}
				else if(DeviceData.keyData[2] == 0x40)//F7
				{
					iKey = 6;
					DEBUG_LOG("[Debug]>>KbPeekData()>> Press F7");
					FunctionAction(iDevice ,iKey);
				}
				else if(DeviceData.keyData[2] == 0x41)//F8
				{
					iKey = 7;
					DEBUG_LOG("[Debug]>>KbPeekData()>> Press F8");
					FunctionAction(iDevice ,iKey);
				}
				else if(DeviceData.keyData[2] == 0x42)//F9
				{
					iKey = 8;
					DEBUG_LOG("[Debug]>>KbPeekData()>> Press F9");
					FunctionAction(iDevice ,iKey);
				}
				else if(DeviceData.keyData[2] == 0x43)//F10
				{
					iKey = 9;
					DEBUG_LOG("[Debug]>>KbPeekData()>> Press F10");
					FunctionAction(iDevice ,iKey);
				}
				else if(DeviceData.keyData[2] == 0x44)//F11
				{
					iKey = 10;
					DEBUG_LOG("[Debug]>>KbPeekData()>> Press F11");
					FunctionAction(iDevice ,iKey);
				}
				else if(DeviceData.keyData[2] == 0x45)//F12
				{
					iKey = 11;
					DEBUG_LOG("[Debug]>>KbPeekData()>> Press F12");
					FunctionAction(iDevice ,iKey);
				}




			}
		}
		CloseHandle(pKbDevEvent.hEvent);
		CloseHandle(hHandle);
		//ExitThread(0);
		return TRUE;
	}
	return TRUE;
}

void OpenKbDriver(const v8::FunctionCallbackInfo<Value>& args)
{
	return;
	Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);


  DWORD    dwThreadId;

	//BOOL     bReturn;
	DWORD    dw;
	//char     buf[255];
	//TCHAR    FilterName[] = TEXT("\\\\.\\gKbdUpper");  //for Hid Lower Filter

	hHandle = CreateFile(KEYBOARD_FILTER_DRV_NAME,
						 GENERIC_READ|GENERIC_WRITE,
						 FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
						 OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	if (hHandle != INVALID_HANDLE_VALUE)
	{
		CreateThread(NULL,1024,(LPTHREAD_START_ROUTINE)KbPeekData,NULL,0,&dwThreadId);
		NotifyLogMessage("##Test Log## win32Driver.cpp >>> Open KB driver ( CreateThread ).");
	}
	dw = GetLastError();
	if (hHandle == INVALID_HANDLE_VALUE)
	{
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"Open KbDriver failed.")));
		return;
	}
	NotifyLogMessage("##Test Log## win32Driver.cpp >>> Open KB driver: gKbdUpper.");
	//MessageBox(NULL,L"OK",L"",MB_OK);
	//return TRUE;
}

void CloseKbDriver(const v8::FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
  	HandleScope scope(isolate);
  	NotifyLogMessage("##Test Log## win32Driver.cpp >>>  Close driver: gKbdUpper");

	DWORD       dwRet;
	BOOL        bReturn;

	if (hHandle != INVALID_HANDLE_VALUE)
	{
		pKbDevEvent.hDevExist = 0;
        bReturn = DeviceIoControl(hHandle,IOCTL_UNREGISTER_EVENT,
						&pKbDevEvent,sizeof(EVENTPACK),NULL,0,&dwRet,NULL);

		ThreadAlive = FALSE;
		CloseHandle(hHandle);
		hHandle = INVALID_HANDLE_VALUE;
	}

	//Free Keyboard
	for(int i = 0; i < MAX_KEYBOARD_QTY; i++)
	{
		if (KeyboardInfo[i] == NULL)
			break;
		for (int n = 0; n < 12; n++)
		{
			if (KeyboardInfo[i]->Keys[n] == NULL)
				break;

			free(KeyboardInfo[i]->Keys[n]);
			KeyboardInfo[i]->Keys[n] = NULL;
		}

		free(KeyboardInfo[i]);
		KeyboardInfo[i] = NULL;
	}

}



////////////////////////////////////////////////
