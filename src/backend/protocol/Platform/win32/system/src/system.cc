#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <uv.h>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <dbt.h>
#include <WtsApi32.h>
#include <stdlib.h>
#include <stdio.h>
#include <hidApi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <Shellapi.h>
#include <setupapi.h>
#include <winioctl.h>
#include <accctrl.h>
#include <aclapi.h>

using namespace v8;

#include <string>
#include <vector>
#include <ntsecapi.h>
typedef LONG NTSTATUS;
#define NT_SUCCESS(status)((NTSTATUS)(status)>=0)
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

enum Options {
  OPTION_NONE  = 0,
  // Hide the command's window.
  OPTION_HIDE  = 1 << 1,
  // Run as administrator.
  OPTION_ADMIN = 1 << 2,
};

#define MAX_THREAD_WINDOW_NAME 64
#define VID_TAG "VID_"
#define PID_TAG "PID_"

void RegisterAppChange(const v8::FunctionCallbackInfo<Value>& args);
void RegisterHotplug(const FunctionCallbackInfo<Value>& args);
void RegisterSessionChange(const FunctionCallbackInfo<Value>& args);

Persistent<Value> appChangeCallback;
Persistent<Value> DeviceChangeCallback;
Persistent<Value> SessionChangeCallback;
Persistent<Value> LogMessageCallback;

#define DEBUG_HEADER fprintf(stderr, "System [%s:%s() %d]: ", __FILE__, __FUNCTION__, __LINE__); 
#define DEBUG_FOOTER fprintf(stderr, "\n");
#define DEBUG_LOG(...) DEBUG_HEADER fprintf(stderr, __VA_ARGS__); DEBUG_FOOTER

DWORD dwPID = -1;
bool isAppChangeRegistered = false, isDeviceChangeRegistered = false, isSessionChangeRegistered = false, g_bWaitThreadAlive;
DWORD threadId, waitThreadId;
HANDLE threadHandle, waitThreadHandle,g_EventDevChg;
HANDLE onAppChangeRegisteredEvent, onSessionChangeRegisteredEvent; 
int g_WaitIndex = 0;

int SessionType = 0, DeviceChangeType = -1;
char HidUsagePage[MAX_THREAD_WINDOW_NAME];

//GUID GUID_DEVINTERFACE_DISK = {0x53F56307L, 0xB6BF, 0x11D0, 0x94, 0xF2, 0x00, 0xA0, 0xC9, 0x1E, 0xFB, 0x8B};
GUID GuidHid;

DWORD WINAPI ListenerThread( LPVOID lpParam );
DWORD WINAPI WaitThreadProc( LPVOID lpParam ) ;
void StopWaitThread();


typedef struct
{
    WORD idReserved; // must be 0
    WORD idType; // 1 = ICON, 2 = CURSOR
    WORD idCount; // number of images (and ICONDIRs)
} ICONHEADER;


typedef struct
{
    BYTE bWidth;
    BYTE bHeight;
    BYTE bColorCount;
    BYTE bReserved;
    WORD wPlanes; // for cursors, this field = wXHotSpot
    WORD wBitCount; // for cursors, this field = wYHotSpot
    DWORD dwBytesInRes;
    DWORD dwImageOffset; // file-offset to the start of ICONIMAGE

} ICONDIR;


typedef struct
{
    BITMAPINFOHEADER biHeader; // header for color bitmap (no mask header)

} ICONIMAGE;



typedef struct _ST_HID_DEVICE
{
    USHORT VID;
    USHORT PID;
    char sPath[255];
    char sUsagePage[20];
    int InterfaceNumber;
    int ReportNumber;
}ST_HID_DEVICE;
typedef struct _ST_DEVICE_CHANGE_CALLBACK
{
    USHORT VID;
    USHORT PID;
    int PlugType;
    int DeviceType;
    char UsagePage[20];
    int InterfaceNumber;
    int ReportNumber;
}ST_DEVICE_CHANGE_CALLBACK;
#define HID_DEVICE_QTY  500
ST_HID_DEVICE * hidDevices[HID_DEVICE_QTY] = {0};
ST_DEVICE_CHANGE_CALLBACK * devChangeCallback = NULL;

ST_HID_DEVICE * NewHidDeviceInfo()
{
    for(int i = 0;i < HID_DEVICE_QTY; i++){
        if (hidDevices[i] == NULL){
            hidDevices[i] = (ST_HID_DEVICE *)calloc(1, sizeof(ST_HID_DEVICE));
            return hidDevices[i];
        }
    }
    return NULL;
}
ST_HID_DEVICE * GetHidDeviceInfo(char * path)
{
    for(int i = 0;i < HID_DEVICE_QTY; i++){
        if (hidDevices[i] != NULL && stricmp(hidDevices[i]->sPath, path) == 0)
            return hidDevices[i];
    }
    return NULL;
}

BOOL CALLBACK EnumChildWindowsProc(HWND hWnd, LPARAM lParam)
{
  if(hWnd == (HWND)lParam){
    GetWindowThreadProcessId(hWnd,&dwPID);
  }
  return TRUE;
} 

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
  if(hWnd == (HWND)lParam){
    GetWindowThreadProcessId(hWnd,&dwPID);
  }
  else{
    EnumChildWindows(hWnd,EnumChildWindowsProc,lParam);
  }
  return TRUE;
}
 
BOOL DosPathToNtPath(TCHAR *pszDosPath, TCHAR *pszNtPath)  
{  
    TCHAR           szDriveStr[500];  
    TCHAR           szDrive[3];  
    TCHAR           szDevName[100];  
    INT             cchDevName;  
    INT             i;  
       
    if(!pszDosPath || !pszNtPath )  
        return FALSE;  
    if(GetLogicalDriveStrings(sizeof(szDriveStr), szDriveStr)) {  
        for(i = 0; szDriveStr[i]; i += 4) {  
            if(!lstrcmpi(&(szDriveStr[i]), "A:\\") || !lstrcmpi(&(szDriveStr[i]), "B:\\"))  
                continue;   
            szDrive[0] = szDriveStr[i];  
            szDrive[1] = szDriveStr[i + 1];  
            szDrive[2] = '\0';  
            if(!QueryDosDevice(szDrive, szDevName, 100))  
                return FALSE;  
            cchDevName = lstrlen(szDevName);  
            if(strnicmp(pszDosPath, szDevName, cchDevName) == 0) {  
                lstrcpy(pszNtPath, szDrive); 
                lstrcat(pszNtPath, pszDosPath + cchDevName);  
                return TRUE;  
            }             
        }  
    }  
  
    lstrcpy(pszNtPath, pszDosPath);  
    return FALSE;  
}   

BOOL enablePrivilege( LPCTSTR lpszPrivilegeName, BOOL bEnable )
{
  HANDLE      hToken;
  TOKEN_PRIVILEGES  tp;
  LUID      luid;
  BOOL      ret;

  if( !OpenProcessToken( GetCurrentProcess(),
    TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_READ, &hToken ) )
  {
    return FALSE;
  }

  if( !LookupPrivilegeValue( NULL, lpszPrivilegeName, &luid ) )
  {
    return FALSE;
  }

  tp.PrivilegeCount      = 1;
  tp.Privileges[0].Luid    = luid;
  tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

  ret = AdjustTokenPrivileges( hToken, FALSE, &tp, 0, NULL, NULL );

  CloseHandle( hToken );

  return ret;
}


bool AdjustPrivileges() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    TOKEN_PRIVILEGES oldtp;
    DWORD dwSize=sizeof(TOKEN_PRIVILEGES);
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        if (GetLastError()==ERROR_CALL_NOT_IMPLEMENTED) return true;
        else return false;
    }
    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
        CloseHandle(hToken);
        return false;
    }
    ZeroMemory(&tp, sizeof(tp));
    tp.PrivilegeCount=1;
    tp.Privileges[0].Luid=luid;
    tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
    /* Adjust Token Privileges */
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &oldtp, &dwSize)) {
        CloseHandle(hToken);
        return false;
    }
    // close handles
    CloseHandle(hToken);
    return true;
} 

BOOL GetProcessPath( DWORD dwProcessId,TCHAR* pszExePath)
{
        HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
        if (hSnapshot == INVALID_HANDLE_VALUE)
        {
                DWORD dwRetCode = ::GetLastError();
               DEBUG_LOG("CreateToolhelp32Snapshot  error %d",dwRetCode);   
                return FALSE;
        }
        MODULEENTRY32 me32;
        me32.dwSize = sizeof(MODULEENTRY32);
        if (!Module32First(hSnapshot, &me32))
        {
                CloseHandle(hSnapshot);
                return FALSE;
        }
        strcpy(pszExePath,me32.szExePath);
        CloseHandle(hSnapshot);
        return TRUE;
}
 

BOOL ProcessPrivilege(BOOL bEnable)
{
 BOOL                   bResult = TRUE;
 HANDLE               hToken=INVALID_HANDLE_VALUE;
 TOKEN_PRIVILEGES     TokenPrivileges;
 if(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,&hToken) == 0)
 {
     printf("OpenProcessToken Error: %d\n",GetLastError());
     bResult = FALSE;
 }
 TokenPrivileges.PrivilegeCount           = 1;
 TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
 LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&TokenPrivileges.Privileges[0].Luid);
 AdjustTokenPrivileges(hToken,FALSE,&TokenPrivileges,sizeof(TOKEN_PRIVILEGES),NULL,NULL);
    if(GetLastError() != ERROR_SUCCESS)
 {
     bResult = FALSE;
 }
 CloseHandle(hToken);
    
 return bResult;
}


static void PrintWin32Error(char *message,DWORD dwMessageId)
{
    char *errMsg;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,dwMessageId,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPTSTR)&errMsg,0,NULL);
    DEBUG_LOG("%s: %s",message,errMsg);
    LocalFree(errMsg);
    return;
}
static PVOID GetFromToken(HANDLE TokenHandle,TOKEN_INFORMATION_CLASS TokenInformationClass)
{
    DWORD needed=0;
    PVOID buf=NULL;
    DWORD error;
    BOOL errflag=FALSE;
    if(FALSE==GetTokenInformation(TokenHandle,TokenInformationClass,NULL,0,&needed))
    {
        error=GetLastError();
        if(error!=ERROR_INSUFFICIENT_BUFFER)
        {
            PrintWin32Error("GetTokenInformation() failed",error);
            errflag=TRUE;
            goto GetFromToken_exit;
        }
    }
    if(NULL==(buf=calloc(needed,1)))
    {
        fprintf(stderr,"calloc(%u,1) failed\n",needed);
        goto GetFromToken_exit;
    }
    if(FALSE==GetTokenInformation(TokenHandle,TokenInformationClass,buf,needed,&needed))
    {
        PrintWin32Error("GetTokenInformation() failed",GetLastError());
        errflag=TRUE;
        goto GetFromToken_exit;
    }
GetFromToken_exit:
    if(errflag==TRUE)
    {
        if(buf!=NULL)
        {
            free(buf);
            buf=NULL;
        }
    }
    return(buf);
}
static BOOL AddPrivilege(LSA_HANDLE PolicyHandle,PSID AccountSid,LPWSTR PrivilegeName)
{
    BOOL ret=FALSE;
    LSA_UNICODE_STRING UserRights;
    USHORT StringLength;
    NTSTATUS status;
    if(PrivilegeName==NULL)
    {
        goto AddPrivilege_exit;
    }
    StringLength=wcslen(PrivilegeName);
    UserRights.Buffer=PrivilegeName;
    UserRights.Length=StringLength * sizeof(WCHAR);
    UserRights.MaximumLength=(StringLength + 1) * sizeof(WCHAR);
    status=LsaAddAccountRights(PolicyHandle,AccountSid,&UserRights,1);
    if(status!=STATUS_SUCCESS)
    {
        PrintWin32Error("LsaAddAccountRights() failed",LsaNtStatusToWinError(status));
        goto AddPrivilege_exit;
    }
    ret=TRUE;
AddPrivilege_exit:
    return(ret);
}

static BOOL AddCurrentProcessPrivilege(LPWSTR PrivilegeName)
{
    NTSTATUS status;
    BOOL ret=FALSE;
    LSA_HANDLE PolicyHandle=NULL;
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE CurrentProcessToken=NULL;
    PTOKEN_USER token_user=NULL;
    ZeroMemory(&ObjectAttributes,sizeof(ObjectAttributes));
    status=LsaOpenPolicy(NULL,&ObjectAttributes,POLICY_ALL_ACCESS,&PolicyHandle);
    if(status!=STATUS_SUCCESS)
    {
        PrintWin32Error("LsaOpenPolicy() failed",LsaNtStatusToWinError(status));
       // goto AddCurrentProcessPrivilege_exit;
    }
    if(FALSE==OpenProcessToken(GetCurrentProcess(),
        TOKEN_QUERY,
        &CurrentProcessToken))
    {
        PrintWin32Error("OpenProcessToken() failed",GetLastError());
        goto AddCurrentProcessPrivilege_exit;
    }
    if(NULL==(token_user=(PTOKEN_USER)GetFromToken(CurrentProcessToken,TokenUser)))
    {
        goto AddCurrentProcessPrivilege_exit;
    }
    if(FALSE==AddPrivilege(PolicyHandle,
        token_user->User.Sid,
        PrivilegeName))
    {
        goto AddCurrentProcessPrivilege_exit;
    }
    ret=TRUE;
AddCurrentProcessPrivilege_exit:
    if(NULL!=token_user)
    {
        free(token_user);
        token_user=NULL;
    }
    if(NULL!=CurrentProcessToken)
    {
        CloseHandle(CurrentProcessToken);
        CurrentProcessToken=NULL;
    }
    if(NULL!=PolicyHandle)
    {
        LsaClose(PolicyHandle);
        PolicyHandle=NULL;
    }
    return(ret);
}




BOOL FindWindowProcessModule(HWND Hwnd, TCHAR *szName,size_t iMaxLen)
{
      if(AddCurrentProcessPrivilege(L"SeDebugPrivilege"))
      {
         DEBUG_LOG("AddCurrentProcessPrivilege OK!"); 
      }else
      {
         DEBUG_LOG("AddCurrentProcessPrivilege NG!");
      }
      BOOL a = enablePrivilege(SE_TCB_NAME, true );
      enablePrivilege(SE_SYSTEM_PROFILE_NAME, true );
      enablePrivilege(SE_SECURITY_NAME, true );
      enablePrivilege(SE_DEBUG_NAME, true );
      ProcessPrivilege(true);
      if(a)
      {
         DEBUG_LOG("enablePrivilege OK!");
      }
      else
      {
         DEBUG_LOG("enablePrivilege NG!");
      }
      TCHAR tmpPath[MAX_PATH] = {0};   
      DEBUG_LOG("szName = %s",szName);
      EnumWindows(EnumWindowsProc,(LPARAM)Hwnd);
      DEBUG_LOG("dwPID = %d",dwPID);  
      if(-1 != dwPID) 
      {
        HMODULE hModule = NULL;
        DWORD dwNeeded = NULL;
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE,dwPID); 
        DEBUG_LOG("hProcess -->%d",hProcess);
        if (NULL != hProcess) 
        {
            if((DWORD)hProcess > 0)
            { 
                if(EnumProcessModules(hProcess,&hModule,sizeof(hModule),&dwNeeded))
                {
                  DEBUG_LOG("EnumProcessModules hModule -->%d",hModule);
                  GetModuleFileNameEx(hProcess,hModule,szName,iMaxLen);  
                }
                DEBUG_LOG("szName:"+strcmp(szName,""));
              
                if(strcmp(szName,"")==0)
                {
                  if(GetProcessImageFileName(hProcess,tmpPath,MAX_PATH))
                  {
                     DosPathToNtPath(tmpPath, szName);
                  }
                }
                
                CloseHandle(hProcess); 
            }
            else
            {
                GetProcessPath((DWORD)dwPID,szName);

            }
        }
        else
        {
              GetProcessPath((DWORD)dwPID,szName);
              DEBUG_LOG("szName>>>>>>>:"+strcmp(szName,""));
        }
      }
      return NULL != szName[0];
}

void GetCurrentAppPath(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  TCHAR szName[MAX_PATH] = {0};
  HWND Hwnd = GetForegroundWindow();
  FindWindowProcessModule(Hwnd,szName,MAX_PATH);
  DEBUG_LOG("GetCurrentAppPath--->", szName);
  args.GetReturnValue().Set(String::NewFromUtf8(isolate,szName));
}

void GetMouseSpeed(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  int mouseSpeed = 0;
  if(!SystemParametersInfo(SPI_GETMOUSESPEED, 0, &mouseSpeed, 0)){
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Get Mouse Speed Error.")));
    return;
  }
  args.GetReturnValue().Set(Integer::New(isolate,mouseSpeed));
}

void SetMouseSpeed(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  if (args.Length() < 1 || !args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong arguments")));
    return;
  }
  int mouseSpeed = (int)args[0]->Int32Value();
  if(!SystemParametersInfo(SPI_SETMOUSESPEED, 0, (VOID*)mouseSpeed, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE)){
	  isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Set Mouse Speed Error.")));
    return;
  }
}

void GetDoubleClickTime(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  int dclickTime=GetDoubleClickTime();
  args.GetReturnValue().Set(Integer::New(isolate,dclickTime));
}

void SetDoubleClickTime(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  if (args.Length() < 1 || !args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong arguments")));
    return;
  }
  int dclickTime = (int)args[0]->Int32Value();
  if(!SystemParametersInfo(SPI_SETDOUBLECLICKTIME, dclickTime, 0, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE)){
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Set DoubleClick Time Error.")));
    return;
  }
}

void GetSwapButton(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  int btnSwp = GetSystemMetrics(SM_SWAPBUTTON);
  args.GetReturnValue().Set(Integer::New(isolate, btnSwp));
}

void SetSwapButton(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  if (args.Length() < 1 || !args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong arguments")));
    return;
  }
  int btnSwp = (int)args[0]->Int32Value() == 0 ? 0 : 1;
  if(!SystemParametersInfo(SPI_SETMOUSEBUTTONSWAP, btnSwp, 0, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE)){
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Set SwapButton Error.")));
    return;
  }
}

void GetWheelScrollLines(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  int lines = 0;
  if(!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &lines, 0)){
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"GetWheelScrollLines Error.")));
    return;
  }
  args.GetReturnValue().Set(Integer::New(isolate, lines));
}

void SetWheelScrollLines(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  if (args.Length() < 1 || !args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong arguments")));
    return;
  }
  int lines = (int)args[0]->Int32Value();
  if(!SystemParametersInfo(SPI_SETWHEELSCROLLLINES, lines, 0, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE)){
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"SetWheelScrollLines Error.")));
    return;
  }
}

//切换App消息
void RegisterAppChange(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  if (args.Length() < 1 || !args[0]->IsFunction()) 
  {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"First argument must be a function")));
    return;
  } 
  appChangeCallback.Reset(isolate,args[0]);
  isAppChangeRegistered=true;
}

void NotifyAppChange(){
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  if (isAppChangeRegistered) {
    Handle<Value> argv[1];
    TCHAR szName[MAX_PATH] = {0};
	  HWND Hwnd = GetForegroundWindow();
    DEBUG_LOG("Hwnd -->%d",Hwnd);
	  FindWindowProcessModule(Hwnd,szName,MAX_PATH);  

    argv[0] = String::NewFromUtf8(isolate,szName); 
    String::Utf8Value valFrom(argv[0]->ToString());
 

    DEBUG_LOG(*valFrom);

    Local<Value> cbVal=Local<Value>::New(isolate,appChangeCallback);
    Local<Function> cb = Local<Function>::Cast(cbVal);
    cb->Call(isolate->GetCurrentContext()->Global(), 1, argv);
  }
}

void NotifyAppChangeAsync(uv_work_t* req){
    WaitForSingleObject(onAppChangeRegisteredEvent, INFINITE);
}

void NotifyAppChangeFinished(uv_work_t* req){
    NotifyAppChange();
    uv_queue_work(uv_default_loop(), req, NotifyAppChangeAsync, (uv_after_work_cb)NotifyAppChangeFinished);
}

void NotifyDeviceChangeCallback(ST_DEVICE_CHANGE_CALLBACK * callBack){
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    if (isDeviceChangeRegistered) {
      Local<Value> cbVal=Local<Value>::New(isolate, DeviceChangeCallback);
      Local<Function> cb = Local<Function>::Cast(cbVal);
      //
      Local<Object> baseInfo = Object::New(isolate);
      baseInfo->Set(String::NewFromUtf8(isolate,"PlugType"), Integer::New(isolate, callBack->PlugType));
      baseInfo->Set(String::NewFromUtf8(isolate,"VID"), Integer::New(isolate, callBack->VID));
      baseInfo->Set(String::NewFromUtf8(isolate,"PID"), Integer::New(isolate, callBack->PID));
      baseInfo->Set(String::NewFromUtf8(isolate,"DeviceType"), Integer::New(isolate, callBack->DeviceType));
      //
      Local<Object> extInfo = Object::New(isolate);
      if (callBack->DeviceType == 1 && callBack->PlugType == 1){
        extInfo->Set(String::NewFromUtf8(isolate,"HidUsage"),String::NewFromUtf8(isolate, callBack->UsagePage));
        extInfo->Set(String::NewFromUtf8(isolate,"HidInterfaceNumber"), Integer::New(isolate, callBack->InterfaceNumber));
        extInfo->Set(String::NewFromUtf8(isolate,"HidReportNumber"), Integer::New(isolate, callBack->ReportNumber));
      }
  
      const unsigned argc = 2;
      Local<Value> argv[argc] = { 
        baseInfo,
        extInfo
      };
      free(callBack);
      callBack = NULL;
      SetEvent(g_EventDevChg);
      cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
    }
}
void NotifyDeviceChangeAsync(uv_work_t* req){
}

void NotifyDeviceChangeFinished(uv_work_t* req){
    ST_DEVICE_CHANGE_CALLBACK * callBack = static_cast<ST_DEVICE_CHANGE_CALLBACK *>(req->data);
    NotifyDeviceChangeCallback(callBack);
}
void NotifyDeviceChange(ST_DEVICE_CHANGE_CALLBACK * callBack){
    uv_work_t* reqDevChg = new uv_work_t();
    reqDevChg->data = callBack;
    uv_queue_work(uv_default_loop(), reqDevChg, NotifyDeviceChangeAsync, (uv_after_work_cb)NotifyDeviceChangeFinished);
}
void NotifySessionChange(){
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    if (isSessionChangeRegistered) {
        Local<Value> cbVal=Local<Value>::New(isolate,SessionChangeCallback);
        Local<Function> cb = Local<Function>::Cast(cbVal);
        const unsigned argc = 1;
        Local<Value> argv[argc] = { 
            Integer::New(isolate,SessionType)
        };
        SessionType = 0;
        cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
    }
}
void NotifySessionChangeAsync(uv_work_t* req){
    WaitForSingleObject(onSessionChangeRegisteredEvent, INFINITE);
}

void NotifySessionChangeFinished(uv_work_t* req){
    NotifySessionChange();
    uv_queue_work(uv_default_loop(), req, NotifySessionChangeAsync, (uv_after_work_cb)NotifySessionChangeFinished);
}

void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,LONG idObject, LONG idChild,DWORD dwEventThread, DWORD dwmsEventTime){
	  SetEvent(onAppChangeRegisteredEvent);
}

void RegisterHotplug(const FunctionCallbackInfo<Value>& args){
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  if (args.Length() != 1 || !args[0]->IsFunction()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 1 argument:[function]")));
    return;
  }
  DeviceChangeCallback.Reset(isolate,args[0]);
  isDeviceChangeRegistered = true;
}

void RegisterSessionChange(const FunctionCallbackInfo<Value>& args){
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);
    if (args.Length() != 1 || !args[0]->IsFunction()) {
        isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 1 argument:[function]")));
        return;
    }
    SessionChangeCallback.Reset(isolate,args[0]);
    isSessionChangeRegistered = true;
}

void ToUpper(char * buf)
{
    char* c = buf;
    while (*c != '\0') 
    {
        *c = toupper((unsigned char)*c);
        c++;
    }
}

void GetDevTypeVidPid(char * buf, ST_DEVICE_CHANGE_CALLBACK * callBack){
  if(buf == NULL || callBack == NULL)
        return;
    ToUpper(buf);

    char* string;
    char* temp;
    char* pidStr, *vidStr, *usbTypeStr;

    string = new char[strlen(buf) + 1];
    memcpy(string, buf, strlen(buf) + 1);

    vidStr = strstr(string, VID_TAG);
    pidStr = strstr(string, PID_TAG);
    usbTypeStr = strstr(string, "HID");
    if(usbTypeStr != NULL){
        callBack->DeviceType = 1;
    }else{
        if(strstr(string, "NXP") != NULL && strstr(string, "IFLASH") != NULL && (strstr(string, "LPC1XXX") != NULL || strstr(string, "LPC134X") != NULL))
        {
            callBack->DeviceType = 3;
        }else{
            usbTypeStr = strstr(string, "USB");
            if(usbTypeStr != NULL){
                callBack->DeviceType = 2;
            }else{
                callBack->DeviceType = 0;
            }
        }
    }
    if (callBack->DeviceType == 1 && callBack->PlugType == 0){
        ST_HID_DEVICE * aHidDev = GetHidDeviceInfo(buf);
        if (aHidDev != NULL){
            callBack->VID = aHidDev->VID;
            callBack->PID = aHidDev->PID;
            delete string;
            //free(aHidDev);
            aHidDev = NULL;
            return;
        }
    }

    if(vidStr != NULL)
    {
        temp = (char*) (vidStr + strlen(VID_TAG));
        temp[4] = '\0';
        callBack->VID = strtol (temp, NULL, 16);
    }
    if(pidStr != NULL)
    {
        temp = (char*) (pidStr + strlen(PID_TAG));
        temp[4] = '\0';
        callBack->PID = strtol (temp, NULL, 16);
    }
    delete string;
}

HANDLE open_device(const char *path, BOOL enumerate)
{
  HANDLE handle;
  DWORD desired_access = (enumerate) ? 0: (GENERIC_WRITE | GENERIC_READ);
  handle = CreateFileA(path, desired_access, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);

  return handle;
}

void GetHidInfo(char * buf, ST_DEVICE_CHANGE_CALLBACK * callBack){
    HANDLE handle;
    PHIDP_PREPARSED_DATA pp_data = NULL;
    HIDP_CAPS caps;
    HIDD_ATTRIBUTES attrib;
    BOOLEAN res;
    ULONG nt_res;
    char HidUsagePage[20];
  
    USHORT usage_page = 0;
    USHORT usage = 0;
  
    if (!buf)
        return;
    handle = open_device(buf, FALSE);
    if (INVALID_HANDLE_VALUE == handle)
        handle = open_device(buf, TRUE);
    if (INVALID_HANDLE_VALUE == handle)
        return;
    res = HidD_GetPreparsedData(handle, &pp_data);
    if (res) {
        nt_res = HidP_GetCaps(pp_data, &caps);
        if (nt_res == HIDP_STATUS_SUCCESS) {
            usage_page = caps.UsagePage;
            usage = caps.Usage;
        }
        HidD_FreePreparsedData(pp_data);
    }
    attrib.Size = sizeof(HIDD_ATTRIBUTES);
    HidD_GetAttributes(handle, &attrib);
    CloseHandle(handle);
    _snprintf_s(HidUsagePage, MAX_THREAD_WINDOW_NAME, "%04X_%04X", usage_page,usage);
    if (callBack != NULL)
    {
        lstrcpy(callBack->UsagePage, HidUsagePage);
        callBack->VID = attrib.VendorID;
        callBack->PID = attrib.ProductID;
        callBack->InterfaceNumber = -1;
        char *interface_component = strstr(buf, "&MI_");
        if (interface_component) {
            char *hex_str = interface_component + 4;
            char *endptr = NULL;
            callBack->InterfaceNumber = strtol(hex_str, &endptr, 16);
            if (endptr == hex_str) 
                callBack->InterfaceNumber = -1;
        }
        
        callBack->ReportNumber = -1;
        char *report_component = strstr(buf, "&COL");
        if (report_component) {
            char *hex_str = report_component + 4;
            char *endptr = NULL;
            callBack->ReportNumber = strtol(hex_str, &endptr, 16);
            if (endptr == hex_str) 
                callBack->ReportNumber = -1;
        }
    }

    ST_HID_DEVICE * aHidDev = GetHidDeviceInfo(buf);
    if (aHidDev == NULL){
        aHidDev = NewHidDeviceInfo();
    }
    if (aHidDev == NULL){
        DEBUG_LOG("NewHidDeviceInfo NULL.");
        return;
    }
    aHidDev->VID = attrib.VendorID;
    aHidDev->PID = attrib.ProductID;
    lstrcpy(aHidDev->sUsagePage, HidUsagePage);
    lstrcpy(aHidDev->sPath, buf);
}

LRESULT CALLBACK DetectCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{ 
    if (msg == WM_DEVICECHANGE)
    { 
          DEBUG_LOG("WM_DEVICECHANGE ...."); 
          if(DeviceChangeType ==0 &&  DBT_DEVNODES_CHANGED == wParam)
          {
              g_WaitIndex = 10;
              return 1;
          }
          if (DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam) 
          {
              WaitForSingleObject(g_EventDevChg, 2000);
              PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
              if(pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
              {
                  PDEV_BROADCAST_DEVICEINTERFACE pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr; 
                  if(DBT_DEVICEARRIVAL == wParam)
                      DeviceChangeType = 1;
                  else
                      DeviceChangeType = 0;
                  ResetEvent(g_EventDevChg);
                  ST_DEVICE_CHANGE_CALLBACK * tmpCallback = (ST_DEVICE_CHANGE_CALLBACK *)calloc(1, sizeof(ST_DEVICE_CHANGE_CALLBACK));
                  if (tmpCallback == NULL){
                      DEBUG_LOG("DetectCallback:tmpCallback NULL.");
                      return 1;
                  }
                  tmpCallback->PlugType = DeviceChangeType;
                  GetDevTypeVidPid(pDevInf->dbcc_name, tmpCallback);
                  
                  if (tmpCallback->DeviceType == 1)
                      GetHidInfo(pDevInf->dbcc_name, tmpCallback);
                  if (devChangeCallback != NULL){
                      free(devChangeCallback);
                      devChangeCallback = NULL;
                  }
                  if (tmpCallback->PlugType == 1){
                      g_WaitIndex = 0;
                      NotifyDeviceChange(tmpCallback);
                  }else{
                      g_WaitIndex = 10;
                      devChangeCallback = (ST_DEVICE_CHANGE_CALLBACK *)calloc(1, sizeof(ST_DEVICE_CHANGE_CALLBACK));
                      memcpy(devChangeCallback, tmpCallback, sizeof(ST_DEVICE_CHANGE_CALLBACK));
                      SetEvent(g_EventDevChg);
                  }
              }
          }
    }else if (msg == WM_WTSSESSION_CHANGE){
        SessionType = wParam;
        SetEvent(onSessionChangeRegisteredEvent);
   }else if (msg == WM_MOUSEMOVE){
       DEBUG_LOG("WM_MOUSEMOVE ...."); 
   }else if (msg == WM_LBUTTONDBLCLK){
       DEBUG_LOG("WM_LBUTTONDBLCLK ...."); 
   }   
   return 1;
}

//Goble
DWORD WINAPI ListenerThread( LPVOID lpParam ){ 
    char className[MAX_THREAD_WINDOW_NAME];
    HDEVNOTIFY hDevNotify;
    int iRegSessionTimes=0;
    _snprintf_s(className, MAX_THREAD_WINDOW_NAME, "ListnerThreadGeniusApp_%d", GetCurrentThreadId());
    WNDCLASSA wincl = {0};
    wincl.hInstance = GetModuleHandle(0);
    wincl.lpszClassName = className;
    wincl.lpfnWndProc = DetectCallback;  


    if (!RegisterClassA(&wincl)){
        DWORD le = GetLastError();
        DEBUG_LOG("RegisterClassA() failed [Error: %x]", le);
        return 1;
    }    
    HWND hwnd = CreateWindowExA(WS_EX_TOPMOST, className, className, 0, 0, 0, 0, 0, NULL, 0, 0, 0);
    if (!hwnd){
        DWORD le = GetLastError();
        DEBUG_LOG("CreateWindowExA() failed [Error: %x]", le);
        return 1;
    }

    DEV_BROADCAST_DEVICEINTERFACE_A notifyFilterUsb = {0};
    notifyFilterUsb.dbcc_size = sizeof(notifyFilterUsb);
    notifyFilterUsb.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    notifyFilterUsb.dbcc_classguid = GUID_DEVINTERFACE_DISK;

    hDevNotify = RegisterDeviceNotificationA(hwnd, &notifyFilterUsb, DEVICE_NOTIFY_WINDOW_HANDLE);
    if (!hDevNotify)
    {
        DWORD le = GetLastError();
        DEBUG_LOG("RegisterDeviceNotificationA() USB failed [Error: %x]", le);
        return 1;
    }
    
    DEV_BROADCAST_DEVICEINTERFACE_A notifyFilterHid = {0};
    notifyFilterHid.dbcc_size = sizeof(notifyFilterHid);
    notifyFilterHid.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    notifyFilterHid.dbcc_classguid = GuidHid;

    hDevNotify = RegisterDeviceNotificationA(hwnd, &notifyFilterHid, DEVICE_NOTIFY_WINDOW_HANDLE);
    if (!hDevNotify)
    {
        DWORD le = GetLastError();
        DEBUG_LOG("RegisterDeviceNotificationA() HID failed [Error: %x]", le);
        return 1;
    }

    HWINEVENTHOOK g_hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND ,EVENT_SYSTEM_FOREGROUND , NULL, HandleWinEvent, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    while(!WTSRegisterSessionNotification(hwnd, NOTIFY_FOR_ALL_SESSIONS)){//NOTIFY_FOR_THIS_SESSION
        iRegSessionTimes++;
        Sleep(200);
        if(iRegSessionTimes>100)
        {
            DEBUG_LOG("WTSRegisterSessionNotification failed [Error: %x]", GetLastError());
            return 1;
        }
        DEBUG_LOG("WTSRegisterSessionNotification [Error: %d]",iRegSessionTimes);
    }

    MSG msg;
    while(TRUE){
        BOOL bRet = GetMessage(&msg, hwnd, 0, 0);
        if ((bRet == 0) || (bRet == -1)){
            break;
        } 
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    WTSUnRegisterSessionNotification(hwnd);
    UnhookWinEvent(g_hook);
    StopWaitThread();
    FreeHidFunctions();
    CloseHandle(g_EventDevChg);
    return 0;
} 

void GetHidDeviceList(){
    BOOL res;
    if (InitHidFunctions() < 0)
    {
        DEBUG_LOG("InitHidFunctions failed [Error: %x]", GetLastError());
        return;
    }
    HidD_GetHidGuid (&GuidHid);
    SP_DEVINFO_DATA devinfo_data;
    SP_DEVICE_INTERFACE_DATA device_interface_data;
    SP_DEVICE_INTERFACE_DETAIL_DATA_A *device_interface_detail_data = NULL;
    HDEVINFO device_info_set = INVALID_HANDLE_VALUE;
    int device_index = 0;
    
    memset(&devinfo_data, 0x0, sizeof(devinfo_data));
    devinfo_data.cbSize = sizeof(SP_DEVINFO_DATA);
    device_interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    device_info_set = SetupDiGetClassDevsA(&GuidHid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    for (;;) {
        DWORD required_size = 0;
    
        res = SetupDiEnumDeviceInterfaces(device_info_set, NULL, &GuidHid, device_index, &device_interface_data);
        if (!res) {
            break;
        }
        res = SetupDiGetDeviceInterfaceDetailA(device_info_set, &device_interface_data, NULL, 0, &required_size, NULL);
        device_interface_detail_data = (SP_DEVICE_INTERFACE_DETAIL_DATA_A*) malloc(required_size);
        device_interface_detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
    
        res = SetupDiGetDeviceInterfaceDetailA(device_info_set, &device_interface_data, device_interface_detail_data, required_size, NULL, NULL);
        if (!res) {
          goto cont;
        }
    
        devinfo_data.cbSize = sizeof(SP_DEVINFO_DATA);
        res = SetupDiEnumDeviceInfo(device_info_set, device_index, &devinfo_data);
        if (!res)
            goto cont;
        GetHidInfo(device_interface_detail_data->DevicePath, NULL);
    cont:
        free(device_interface_detail_data);
        device_index++;
    }
    SetupDiDestroyDeviceInfoList(device_info_set);
}

void InitMonitoring(){
    DEBUG_LOG("InitMonitoring.");
    GetHidDeviceList(); 

    g_EventDevChg = CreateEvent(NULL, TRUE, TRUE, NULL);
    onAppChangeRegisteredEvent = CreateEvent(NULL, false, false, "");
    onSessionChangeRegisteredEvent = CreateEvent(NULL, false, false, "");
    threadHandle = CreateThread(NULL, 0, ListenerThread, NULL, 0, &threadId);   
    waitThreadHandle = CreateThread(NULL, 0, WaitThreadProc, NULL, 0, &waitThreadId); 

    uv_work_t* reqAppChange = new uv_work_t();
    uv_queue_work(uv_default_loop(), reqAppChange, NotifyAppChangeAsync, (uv_after_work_cb)NotifyAppChangeFinished);
    uv_work_t* reqSessionChange = new uv_work_t();
    uv_queue_work(uv_default_loop(), reqSessionChange, NotifySessionChangeAsync, (uv_after_work_cb)NotifySessionChangeFinished);
}

std::string QuoteCmdArg(const std::string& arg) {
  if (arg.size() == 0)
    return arg;

  // No quotation needed.
  if (arg.find_first_of(" \t\"") == std::string::npos)
    return arg;

  // No embedded double quotes or backlashes, just wrap quote marks around
  // the whole thing.
  if (arg.find_first_of("\"\\") == std::string::npos)
    return std::string("\"") + arg + '"';

  // Expected input/output:
  //   input : hello"world
  //   output: "hello\"world"
  //   input : hello""world
  //   output: "hello\"\"world"
  //   input : hello\world
  //   output: hello\world
  //   input : hello\\world
  //   output: hello\\world
  //   input : hello\"world
  //   output: "hello\\\"world"
  //   input : hello\\"world
  //   output: "hello\\\\\"world"
  //   input : hello world\
  //   output: "hello world\"
  std::string quoted;
  bool quote_hit = true;
  for (size_t i = arg.size(); i > 0; --i) {
    quoted.push_back(arg[i - 1]);

    if (quote_hit && arg[i - 1] == '\\') {
      quoted.push_back('\\');
    } else if (arg[i - 1] == '"') {
      quote_hit = true;
      quoted.push_back('\\');
    } else {
      quote_hit = false;
    }
  }

  return std::string("\"") + std::string(quoted.rbegin(), quoted.rend()) + '"';
}

bool RunCommand(const std::string& command,
           const std::vector<std::string>& args,
           const std::string& std_input,
           std::string* std_output,
           std::string* std_error,
           int options,
           int* exit_code) {
  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

  std::string parameters;
  for (size_t i = 0; i < args.size(); ++i)
    parameters += QuoteCmdArg(args[i]) + ' ';

  SHELLEXECUTEINFO sei = { sizeof(sei) };
  sei.fMask = SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS;
  sei.lpVerb = (options & OPTION_ADMIN) ? "runas" : "open";
  sei.lpFile = command.c_str();
  sei.lpParameters = parameters.c_str();
  sei.nShow = SW_NORMAL;

  if (options & OPTION_HIDE)
    sei.nShow = SW_HIDE;

  if (::ShellExecuteEx(&sei) == FALSE || sei.hProcess == NULL)
    return false;

  // Wait for the process to complete.
  ::WaitForSingleObject(sei.hProcess, INFINITE);

  DWORD code;
  if (::GetExitCodeProcess(sei.hProcess, &code) == 0)
    return false;

  *exit_code = code;
  return true;
}

void Runas(const FunctionCallbackInfo<Value>& args){
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  if (!args[0]->IsString() || !args[1]->IsArray() || !args[2]->IsObject()){
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"Bad argument.")));
    return;
  }
  std::string command(*String::Utf8Value(args[0]));
  std::vector<std::string> c_args;

  Handle<Array> v_args = Handle<Array>::Cast(args[1]);
  uint32_t length = v_args->Length();

  c_args.reserve(length);
  for (uint32_t i = 0; i < length; ++i) {
    std::string arg(*String::Utf8Value(v_args->Get(i)));
    c_args.push_back(arg);
  }

  Handle<Object> v_options = args[2]->ToObject();
  int options = OPTION_NONE;
  if (v_options->Get(String::NewFromUtf8(isolate,"hide"))->BooleanValue())
    options |= OPTION_HIDE;
  if (v_options->Get(String::NewFromUtf8(isolate,"admin"))->BooleanValue())
    options |= OPTION_ADMIN;

  std::string std_input;
  Handle<Value> v_stdin = v_options->Get(String::NewFromUtf8(isolate,"stdin"));
  if (!v_stdin->IsUndefined())
    std_input = *String::Utf8Value(v_stdin);

  std::string std_output, std_error;
  bool catch_output = v_options->Get(String::NewFromUtf8(isolate,"catchOutput"))->BooleanValue();

  int code = -1;
  RunCommand(command, c_args,
               std_input,
               catch_output ? &std_output : NULL,
               catch_output ? &std_error : NULL,
               options,
               &code);

  if (catch_output) {
    Handle<Object> result = Object::New(isolate);
    result->Set(String::NewFromUtf8(isolate,"exitCode"), Integer::New(isolate, code));
    result->Set(String::NewFromUtf8(isolate,"stdout"), String::NewFromUtf8(isolate,std_output.data()));
    result->Set(String::NewFromUtf8(isolate,"stderr"), String::NewFromUtf8(isolate,std_error.data()));
    args.GetReturnValue().Set(result);
  } else {
    args.GetReturnValue().Set(Integer::New(isolate, code));
  }
}


static UINT WriteIconHeader(HANDLE hFile, int nImages)
{
    ICONHEADER iconheader;
    DWORD nWritten;
 
    iconheader.idReserved = 0; 
    iconheader.idType = 1; 
    iconheader.idCount = nImages; 

    WriteFile( hFile, &iconheader, sizeof(iconheader), &nWritten, 0);

    return nWritten;
}

static UINT NumBitmapBytes(BITMAP *pBitmap)
{
    int nWidthBytes = pBitmap->bmWidthBytes;

    if(nWidthBytes & 3)
        nWidthBytes = (nWidthBytes + 4) & ~3;

    return nWidthBytes * pBitmap->bmHeight;
}

static UINT WriteIconImageHeader(HANDLE hFile, BITMAP *pbmpColor, BITMAP *pbmpMask)
{
    BITMAPINFOHEADER biHeader;
    DWORD nWritten;
    UINT nImageBytes;

    nImageBytes = NumBitmapBytes(pbmpColor) + NumBitmapBytes(pbmpMask);

   ZeroMemory(&biHeader, sizeof(biHeader));

    biHeader.biSize = sizeof(biHeader);
    biHeader.biWidth = pbmpColor->bmWidth;
    biHeader.biHeight = pbmpColor->bmHeight * 2; // height of color+mono
    biHeader.biPlanes = pbmpColor->bmPlanes;
    biHeader.biBitCount = pbmpColor->bmBitsPixel;
    biHeader.biSizeImage = nImageBytes;

    WriteFile(hFile, &biHeader, sizeof(biHeader), &nWritten, 0);

    if(pbmpColor->bmBitsPixel == 2 || pbmpColor->bmBitsPixel == 8)
    {

    }

    return nWritten;
}


static BOOL GetIconBitmapInfo(HICON hIcon, ICONINFO *pIconInfo, BITMAP *pbmpColor, BITMAP *pbmpMask)
{
    if(!GetIconInfo(hIcon, pIconInfo))
        return FALSE;

    if(!GetObject(pIconInfo->hbmColor, sizeof(BITMAP), pbmpColor))
        return FALSE;

    if(!GetObject(pIconInfo->hbmMask, sizeof(BITMAP), pbmpMask))
        return FALSE;

    return TRUE;
}

 
static UINT WriteIconDirectoryEntry(HANDLE hFile, int nIdx, HICON hIcon, UINT nImageOffset)
{
    ICONINFO iconInfo;
    ICONDIR iconDir;

    BITMAP bmpColor;
    BITMAP bmpMask;

    DWORD nWritten;
    UINT nColorCount;
    UINT nImageBytes;

    GetIconBitmapInfo(hIcon, &iconInfo, &bmpColor, &bmpMask);

    nImageBytes = NumBitmapBytes(&bmpColor) + NumBitmapBytes(&bmpMask);

    if(bmpColor.bmBitsPixel >= 8)
        nColorCount = 0;
    else
        nColorCount = 1 << (bmpColor.bmBitsPixel * bmpColor.bmPlanes);

    iconDir.bWidth = (BYTE)bmpColor.bmWidth;
    iconDir.bHeight = (BYTE)bmpColor.bmHeight;
    iconDir.bColorCount = nColorCount;
    iconDir.bReserved = 0;
    iconDir.wPlanes = bmpColor.bmPlanes;
    iconDir.wBitCount = bmpColor.bmBitsPixel;
    iconDir.dwBytesInRes = sizeof(BITMAPINFOHEADER) + nImageBytes;
    iconDir.dwImageOffset = nImageOffset;

    WriteFile(hFile, &iconDir, sizeof(iconDir), &nWritten, 0);

    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    return nWritten;
}

static UINT WriteIconData(HANDLE hFile, HBITMAP hBitmap)
{
    BITMAP bmp;
    int i;
    BYTE * pIconData;

    UINT nBitmapBytes;
    DWORD nWritten;

    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    nBitmapBytes = NumBitmapBytes(&bmp);

    pIconData = (BYTE *)malloc(nBitmapBytes);

    GetBitmapBits(hBitmap, nBitmapBytes, pIconData);

    for(i = bmp.bmHeight - 1; i >= 0; i--)
    {
        WriteFile(
            hFile,
            pIconData + (i * bmp.bmWidthBytes), // calculate offset to the line
            bmp.bmWidthBytes, // 1 line of BYTES
            &nWritten,
            0);

          if(bmp.bmWidthBytes & 3)
        {
            DWORD padding = 0;
            WriteFile(hFile, &padding, 4 - bmp.bmWidthBytes, &nWritten, 0);
        }
    }

    free(pIconData);

    return nBitmapBytes;
}


BOOL SaveIcon3(TCHAR *szIconFile, HICON hIcon[], int nNumIcons)
{
    HANDLE hFile;
    int i;
    int * pImageOffset;

    if(hIcon == 0 || nNumIcons < 1)
        return FALSE;

    hFile = CreateFile(szIconFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

    if(hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    WriteIconHeader(hFile, nNumIcons);

    SetFilePointer(hFile, sizeof(ICONDIR) * nNumIcons, 0, FILE_CURRENT);

    pImageOffset = (int *)malloc(nNumIcons * sizeof(int));

    for(i = 0; i < nNumIcons; i++)
    {
        ICONINFO iconInfo;
        BITMAP bmpColor, bmpMask;

        GetIconBitmapInfo(hIcon[i], &iconInfo, &bmpColor, &bmpMask);

        pImageOffset[i] = SetFilePointer(hFile, 0, 0, FILE_CURRENT);

        WriteIconImageHeader(hFile, &bmpColor, &bmpMask);

        WriteIconData(hFile, iconInfo.hbmColor);
        WriteIconData(hFile, iconInfo.hbmMask);

        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
    }

    SetFilePointer(hFile, sizeof(ICONHEADER), 0, FILE_BEGIN);

    for(i = 0; i < nNumIcons; i++)
    {
        WriteIconDirectoryEntry(hFile, i, hIcon[i], pImageOffset[i]);
    }

    free(pImageOffset);

    CloseHandle(hFile);

    return TRUE;
}

void RegisterLogMessage (const FunctionCallbackInfo<Value>& args) 
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate); 
    if (args.Length() < 1 || !args[0]->IsFunction()) 
    {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"First argument must be a function")));
        return;
    }
    LogMessageCallback.Reset(isolate, args[0]);
}

void getApplicationIcon(const FunctionCallbackInfo<Value>& args) 
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate); 

    if (args.Length() < 3 || !args[0]->IsString() || !args[1]->IsString() || !args[2]->IsNumber()) {
      isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
      return;
    }
    HICON   hSmIcon, hBigIcon;
    char sFrom[100] = {0};
    String::Utf8Value valFrom(args[0]->ToString());
    int pathLen = valFrom.length();
    strncpy(sFrom, *valFrom, pathLen);
  
    char sTo[100] = {0};
    String::Utf8Value valTo(args[1]->ToString());
    int pathLenTo = valTo.length();
    strncpy(sTo, *valTo, pathLenTo);
   
    UINT uiRet = ExtractIconEx(sFrom,0,&hBigIcon,&hSmIcon,1);

    if (hBigIcon)
    {
       SaveIcon3(sTo, &hBigIcon,1);
    }
   
}



extern "C" {
  void Init(Handle<Object> exports) {
    NODE_SET_METHOD(exports, "GetForegroundApp", GetCurrentAppPath);
    NODE_SET_METHOD(exports, "GetMouseSpeed", GetMouseSpeed);
    NODE_SET_METHOD(exports, "SetMouseSpeed", SetMouseSpeed);
    NODE_SET_METHOD(exports, "GetDoubleClickTime", GetDoubleClickTime);
    NODE_SET_METHOD(exports, "SetDoubleClickTime", SetDoubleClickTime);
    NODE_SET_METHOD(exports, "GetSwapButton", GetSwapButton);
    NODE_SET_METHOD(exports, "SetSwapButton", SetSwapButton);
    NODE_SET_METHOD(exports, "GetWheelScrollLines", GetWheelScrollLines);
    NODE_SET_METHOD(exports, "SetWheelScrollLines", SetWheelScrollLines);
    NODE_SET_METHOD(exports, "RegisterLogMessage", RegisterLogMessage);
    NODE_SET_METHOD(exports, "RegisterAppChange", RegisterAppChange);
    NODE_SET_METHOD(exports, "RegisterHotplug", RegisterHotplug);
    NODE_SET_METHOD(exports, "RegisterSessionChange", RegisterSessionChange);
    NODE_SET_METHOD(exports, "Runas", Runas);

    NODE_SET_METHOD(exports, "getApplicationIcon", getApplicationIcon);

    InitMonitoring();
  }

  NODE_MODULE(sysWin, Init)
}

int InitHidFunctions()
{
  lib_handle = LoadLibraryA("hid.dll");
  if (lib_handle) {
#define RESOLVE(x) x = (x##_)GetProcAddress(lib_handle, #x); if (!x) return -1;
    RESOLVE(HidD_GetAttributes);
    RESOLVE(HidD_GetSerialNumberString);
    RESOLVE(HidD_GetManufacturerString);
    RESOLVE(HidD_GetProductString);
    RESOLVE(HidD_SetFeature);
    RESOLVE(HidD_GetFeature);
    RESOLVE(HidD_GetIndexedString);
    RESOLVE(HidD_GetPreparsedData);
    RESOLVE(HidD_FreePreparsedData);
    RESOLVE(HidP_GetCaps);
    RESOLVE(HidD_SetNumInputBuffers);
    RESOLVE(HidD_GetHidGuid);
#undef RESOLVE
  }
  else
    return -1;

  return 0;
}

int FreeHidFunctions()
{
    DEBUG_LOG("FreeHidFunctions");
    if (lib_handle)
        FreeLibrary(lib_handle);
    lib_handle = NULL;
    return 1;
}

DWORD WINAPI WaitThreadProc( LPVOID lpParam )
{
    g_bWaitThreadAlive = true;
    
    while(g_bWaitThreadAlive)
    {
        if(g_WaitIndex > 0)
        {
            g_WaitIndex--;

            if(g_WaitIndex == 0)
            {
                ResetEvent(g_EventDevChg);
                if (devChangeCallback != NULL){
                    ST_DEVICE_CHANGE_CALLBACK * tmpCallback = (ST_DEVICE_CHANGE_CALLBACK *)calloc(1, sizeof(ST_DEVICE_CHANGE_CALLBACK));
                    memcpy(tmpCallback, devChangeCallback, sizeof(ST_DEVICE_CHANGE_CALLBACK));
                    NotifyDeviceChange(tmpCallback);
                }else
                    SetEvent(g_EventDevChg);
            }   
        }
        Sleep(500);
    }
    return 0;
}

void StopWaitThread()
{
    DWORD res;
    g_bWaitThreadAlive = false;
    res = WaitForSingleObject(waitThreadHandle,2000);
    if(res == WAIT_TIMEOUT || res == WAIT_FAILED || res == WAIT_ABANDONED)
        TerminateThread(waitThreadHandle,0);
    CloseHandle(waitThreadHandle);
}


