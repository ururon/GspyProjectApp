#include <node.h>
#include <v8.h>

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>

#include <stdint.h>
#include <vector>

#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include <IOKit/hidsystem/event_status_driver.h>

#include <pthread.h>
#include <uv.h>

using namespace v8;

#include "runas.h"

void RegisterHotplug(const FunctionCallbackInfo<Value>& args);

//output message to log file
void RegisterLogMessage(const v8::FunctionCallbackInfo<Value>& args);


#define DEBUG_HEADER fprintf(stderr, "DarwinDriver [%s:%s() %d]: ", __FILE__, __FUNCTION__, __LINE__); 
#define DEBUG_FOOTER fprintf(stderr, "\n");
#define DEBUG_LOG(...) DEBUG_HEADER fprintf(stderr, __VA_ARGS__); DEBUG_FOOTER


Persistent<Value> LogMessageCallback;
bool isLogMessageRegistered = false;
void NotifyLogMessage(char * msg);


Persistent<Value> DeviceChangeCallback;
Persistent<Value> appChangeCallback;

static pthread_t lookupThreadAppChange;
static pthread_t lookupThreadDeviceChange;

bool isAppChangeRegistered = false;
char * lastForegroundAppPath = NULL;

bool isDeviceChangeRegistered = false;
int DeviceChangeType = -1,DeviceChangeVID = 0,DeviceChangePID = 0;
char HidUsagePage[64];

static IONotificationPortRef    gNotifyPort;
static io_iterator_t            gAddedIter;
static CFRunLoopRef             gRunLoop;

CFMutableDictionaryRef  matchingDict;
CFRunLoopSourceRef      runLoopSource;

typedef struct DeviceListItem 
{
    io_object_t             notification;
    IOUSBDeviceInterface**  deviceInterface;
    UInt16              vendorId;
    UInt16              productId;
} stDeviceListItem;

//App Path,Change
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
void NotifyAppChangeEvent(){
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  if (isAppChangeRegistered) {
    Handle<Value> argv[1];
    argv[0] = String::NewFromUtf8(isolate,lastForegroundAppPath);
    Local<Value> cbVal=Local<Value>::New(isolate,appChangeCallback);
    Local<Function> cb = Local<Function>::Cast(cbVal);
    cb->Call(isolate->GetCurrentContext()->Global(), 1, argv);
  }
}
char * FSRefToPath( const FSRef * ref )
{
    unsigned long length   = 256;
    char *        fullPath = NULL;
    OSStatus      status   = noErr;
    
    fullPath = (char *)malloc( length );
    status = FSRefMakePath( ref, (UInt8 *)fullPath, length );
    
    while ( status == pathTooLongErr || status == buffersTooSmall )
    {
      length += 256;
      char *tmp = fullPath;
      fullPath = (char *)realloc( fullPath, length );
      if( !fullPath )
      {
          free( tmp );
          return NULL;
      }
      status = FSRefMakePath( ref, (UInt8 *)fullPath, length );
    }
    if ( status != noErr )
    {
        free( fullPath ); fullPath = NULL;
    }
    return fullPath;
}
char * GetForegroundAppPath(){
  ProcessSerialNumber psn = {0L,0L};
  OSStatus err = GetFrontProcess(&psn);

  FSRef fsPath;
  err = GetProcessBundleLocation(&psn,&fsPath);
  return FSRefToPath(&fsPath);
}


void NotifyAppChangeAsync(uv_work_t* req)
{
}

void NotifyAppChangeFinished(uv_work_t* req)
{
  NotifyAppChangeEvent();
}

void *RunLoopAppChange(void * arg)
{
  while(1){
    if(isAppChangeRegistered){
      char * nowAppPath = GetForegroundAppPath();
      if(strcmp(lastForegroundAppPath,nowAppPath)!=0){
        strcpy(lastForegroundAppPath,nowAppPath);
        uv_work_t* reqAppChg = new uv_work_t();
        uv_queue_work(uv_default_loop(), reqAppChg, NotifyAppChangeAsync, (uv_after_work_cb)NotifyAppChangeFinished);
      }
    }
    sleep(1);
  }
  return NULL;
}

void GetCurrentAppPath(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  
  char *sPath=GetForegroundAppPath();
  args.GetReturnValue().Set(String::NewFromUtf8(isolate,sPath));
}

void NotifyDeviceChange(){

  NotifyLogMessage("[MSG]   [system.cc]   [NotifyDeviceChange]   [DeviceChange begin]");

  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  if (isDeviceChangeRegistered) {
    Local<Value> cbVal = Local<Value>::New(isolate, DeviceChangeCallback);
    Local<Function> cb = Local<Function>::Cast(cbVal);
    
    Local<Object> baseInfo = Object::New(isolate);
    baseInfo->Set(String::NewFromUtf8(isolate,"PlugType"), Integer::New(isolate, DeviceChangeType));
    baseInfo->Set(String::NewFromUtf8(isolate,"VID"), Integer::New(isolate, DeviceChangeVID));
    baseInfo->Set(String::NewFromUtf8(isolate,"PID"), Integer::New(isolate, DeviceChangePID));

    const unsigned argc = 1;
    Local<Value> argv[argc] = { 
      baseInfo
    };
    
    cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
    
    DeviceChangeType = -1;
    DeviceChangeVID = 0;
    DeviceChangePID = 0;
  }
}

void NotifyDeviceChangeAsync(uv_work_t* req)
{
}

void NotifyDeviceChangeFinished(uv_work_t* req)
{
  NotifyDeviceChange();
}

void SendDeviceChangeCallback(){
  if (isDeviceChangeRegistered){
    uv_work_t* reqDevChg = new uv_work_t();
    uv_queue_work(uv_default_loop(), reqDevChg, NotifyDeviceChangeAsync, (uv_after_work_cb)NotifyDeviceChangeFinished);
  }
}

void DeviceRemoved(void *refCon, io_service_t service, natural_t messageType, void *messageArgument)
{
  kern_return_t   kr;
    stDeviceListItem* deviceListItem = (stDeviceListItem *) refCon;

    if (messageType == kIOMessageServiceIsTerminated) 
    {
        if (deviceListItem->deviceInterface) 
        {
            kr = (*deviceListItem->deviceInterface)->Release(deviceListItem->deviceInterface);
        }
        
        kr = IOObjectRelease(deviceListItem->notification);
        
        DeviceChangeVID = deviceListItem->vendorId;

        DeviceChangePID = deviceListItem->productId;

        NotifyLogMessage("[MSG]   [system.cc]   [DeviceRemoved]   [SendDeviceChangeCallback() - removed]");

        DeviceChangeType = 0;
        SendDeviceChangeCallback();
    }
}

void DeviceAdded(void *refCon, io_iterator_t iterator)
{
    kern_return_t       kr;
    io_service_t        usbDevice;
    IOCFPlugInInterface **plugInInterface = NULL;
    SInt32              score;
    HRESULT             res;
    
    while ((usbDevice = IOIteratorNext(iterator))) 
    {
      UInt16              vendorId;
        UInt16              productId;

        kr = IOCreatePlugInInterfaceForService(usbDevice, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score);

        if ((kIOReturnSuccess != kr) || !plugInInterface) 
        {
            fprintf(stderr, "IOCreatePlugInInterfaceForService returned 0x%08x.\n", kr);
            continue;
        }

        stDeviceListItem *deviceListItem = new stDeviceListItem();

        // Use the plugin interface to retrieve the device interface.
        res = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (LPVOID*) &deviceListItem->deviceInterface);
        
        // Now done with the plugin interface.
        (*plugInInterface)->Release(plugInInterface);
                    
        if (res || deviceListItem->deviceInterface == NULL) 
        {
            fprintf(stderr, "QueryInterface returned %d.\n", (int) res);
            continue;
        }

        kr = (*deviceListItem->deviceInterface)->GetDeviceVendor(deviceListItem->deviceInterface, &vendorId);
        if (KERN_SUCCESS != kr) 
        {
            fprintf(stderr, "GetDeviceVendor returned 0x%08x.\n", kr);
            continue;
        }
        deviceListItem->vendorId = vendorId;
        DeviceChangeVID = vendorId;
        
        kr = (*deviceListItem->deviceInterface)->GetDeviceProduct(deviceListItem->deviceInterface, &productId);
        if (KERN_SUCCESS != kr) 
        {
            fprintf(stderr, "GetDeviceProduct returned 0x%08x.\n", kr);
            continue;
        }
        deviceListItem->productId = productId;
        DeviceChangePID = productId;

        NotifyLogMessage("[MSG]   [system.cc]   [DeviceAdded]   [SendDeviceChangeCallback() - Added ]");

        DeviceChangeType = 1;
        SendDeviceChangeCallback();
        // Register for an interest notification of this device being removed. Use a reference to our
        // private data as the refCon which will be passed to the notification callback.
        kr = IOServiceAddInterestNotification(gNotifyPort,                      // notifyPort
                                              usbDevice,                        // service
                                              kIOGeneralInterest,               // interestType
                                              DeviceRemoved,                    // callback
                                              deviceListItem,                   // refCon
                                              &(deviceListItem->notification)   // notification
                                              );
                                                
        if (KERN_SUCCESS != kr) 
        {
            printf("IOServiceAddInterestNotification returned 0x%08x.\n", kr);
        }
        
        // Done with this USB device; release the reference added by IOIteratorNext
        kr = IOObjectRelease(usbDevice);
    }
}

void *RunLoopDeviceChange(void * arg)
{
    runLoopSource = IONotificationPortGetRunLoopSource(gNotifyPort);
    
    gRunLoop = CFRunLoopGetCurrent();
    CFRunLoopAddSource(gRunLoop, runLoopSource, kCFRunLoopDefaultMode);      

    // Start the run loop. Now we'll receive notifications.
    CFRunLoopRun();

    // We should never get here
    fprintf(stderr, "Unexpectedly back from CFRunLoopRun()!\n");
    return NULL;
}

// System Config
void GetMouseSpeed(const v8::FunctionCallbackInfo<Value>& args) {
  int32_t mouseSpeed = 0;
  IOByteCount speedLen = 0;
  CFStringRef device;
  NXEventHandle handle;
  kern_return_t status;
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  device = CFSTR(kIOHIDMouseAccelerationType);
  handle = NXOpenEventStatus();
  if (!handle) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"NXOpenEventStatus Error.")));
    return;
  }
  status = IOHIDGetParameter(handle,device,sizeof(int32_t),&mouseSpeed,&speedLen);
  if (status != KERN_SUCCESS) {
    NXCloseEventStatus(handle);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"IOHIDGetParameter Error.")));
    return;
  }
  NXCloseEventStatus(handle);

  args.GetReturnValue().Set(Integer::New(isolate,mouseSpeed));
}

void SetMouseSpeed(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() < 1 || !args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong arguments")));
    return;
  }
  int32_t mouseSpeed = (int32_t)args[0]->NumberValue();
  CFStringRef device;
  NXEventHandle handle;
  kern_return_t status;
  HandleScope scope(isolate);

  device = CFSTR(kIOHIDMouseAccelerationType);
  handle = NXOpenEventStatus();
  if (!handle) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"NXOpenEventStatus Error.")));
    return;
  }
  status = IOHIDSetParameter(handle,device,&mouseSpeed,sizeof(mouseSpeed));
  if (status != KERN_SUCCESS) {
    NXCloseEventStatus(handle);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"IOHIDSetParameter Error.")));
    return;
  }
  NXCloseEventStatus(handle);
}

void GetDoubleClickTime(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  NXEventHandle handle;

  handle = NXOpenEventStatus();
  if (!handle) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"NXOpenEventStatus Error.")));
    return;
  }

  double dclickTime=NXClickTime(handle);
  args.GetReturnValue().Set(Number::New(isolate,dclickTime));
}
void SetDoubleClickTime(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  NXEventHandle handle;

  if (args.Length() < 1 || !args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong arguments")));
    return;
  }
  handle = NXOpenEventStatus();
  if (!handle) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"NXOpenEventStatus Error.")));
    return;
  }
  double dclickTime = args[0]->NumberValue();
  NXSetClickTime(handle,dclickTime);
}
void GetSwapButton(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  NXEventHandle handle;
  kern_return_t status;

  handle = NXOpenEventStatus();
  if (!handle) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"NXOpenEventStatus Error.")));
    return;
  }

  int btnSwp = 0;
  status = IOHIDGetMouseButtonMode(handle, &btnSwp);
  if (status != KERN_SUCCESS) {
    NXCloseEventStatus(handle);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"IOHIDGetMouseButtonMode Error.")));
    return;
  }
  NXCloseEventStatus(handle);
  if (btnSwp == 2)
    btnSwp = 0;
  args.GetReturnValue().Set(Integer::New(isolate, btnSwp));
}

void SetSwapButton(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  NXEventHandle handle;
  kern_return_t status;

  if (args.Length() < 1 || !args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong arguments")));
    return;
  }
  int btnSwp = (int)args[0]->Int32Value() == 1 ? 1 : 2;
  handle = NXOpenEventStatus();
  if (!handle) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"NXOpenEventStatus Error.")));
    return;
  }
  status = IOHIDSetMouseButtonMode(handle, btnSwp);
  if (status != KERN_SUCCESS) {
    NXCloseEventStatus(handle);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"IOHIDSetMouseButtonMode Error.")));
    return;
  }
  NXCloseEventStatus(handle);
}

void GetWheelScrollLines(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  NXEventHandle handle = 0;
  kern_return_t status;

  handle = NXOpenEventStatus();
  if (!handle) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"NXOpenEventStatus Error.")));
    return;
  }
  double lines = 0;
  status = IOHIDGetScrollAcceleration(handle, &lines);
  if (status != KERN_SUCCESS) {
    NXCloseEventStatus(handle);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"IOHIDGetScrollAcceleration Error.")));
    return;
  }
  NXCloseEventStatus(handle);
  
  args.GetReturnValue().Set(Number::New(isolate, lines));
}

void SetWheelScrollLines(const v8::FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  NXEventHandle handle = 0;
  kern_return_t status;

  if (args.Length() < 1 || !args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"Wrong arguments")));
    return;
  }
  handle = NXOpenEventStatus();
  if (!handle) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"NXOpenEventStatus Error.")));
    return;
  }
  double lines = args[0]->NumberValue();
  status = IOHIDSetScrollAcceleration(handle, lines);
  if (status != KERN_SUCCESS) {
    NXCloseEventStatus(handle);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,"IOHIDSetScrollAcceleration Error.")));
    return;
  }
  NXCloseEventStatus(handle);
}

void InitMonitoring(){ 
    lastForegroundAppPath = (char *)malloc( 256 );

    int rc = pthread_create(&lookupThreadAppChange, NULL, RunLoopAppChange, NULL);
    if (rc)
    {
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }

    kern_return_t           kr;
    matchingDict = IOServiceMatching(kIOUSBDeviceClassName); 
    if (matchingDict == NULL) 
    {
        fprintf(stderr, "IOServiceMatching returned NULL.\n");
        return;
    }
    gNotifyPort = IONotificationPortCreate(kIOMasterPortDefault);
    kr = IOServiceAddMatchingNotification(gNotifyPort,                  // notifyPort
                                          kIOFirstMatchNotification,    // notificationType
                                          matchingDict,                 // matching
                                          DeviceAdded,                  // callback
                                          NULL,                         // refCon
                                          &gAddedIter                   // notification
                                          );        
    
    if (KERN_SUCCESS != kr) 
    {
        printf("IOServiceAddMatchingNotification returned 0x%08x.\n", kr);
        return;
    }

    // Iterate once to get already-present devices and arm the notification
    DeviceAdded(NULL, gAddedIter);

    rc = pthread_create(&lookupThreadDeviceChange, NULL, RunLoopDeviceChange, NULL);
    if (rc)
    {
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
    }
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
  int options = runas::OPTION_NONE;
  if (v_options->Get(String::NewFromUtf8(isolate,"hide"))->BooleanValue())
    options |= runas::OPTION_HIDE;
  if (v_options->Get(String::NewFromUtf8(isolate,"admin"))->BooleanValue())
    options |= runas::OPTION_ADMIN;

  std::string std_input;
  Handle<Value> v_stdin = v_options->Get(String::NewFromUtf8(isolate,"stdin"));
  if (!v_stdin->IsUndefined())
    std_input = *String::Utf8Value(v_stdin);

  std::string std_output, std_error;
  bool catch_output = v_options->Get(String::NewFromUtf8(isolate,"catchOutput"))->BooleanValue();

  int code = -1;
  runas::Runas(command, c_args,
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

  NODE_SET_METHOD(exports, "RegisterAppChange", RegisterAppChange);
  NODE_SET_METHOD(exports, "RegisterHotplug", RegisterHotplug);
  NODE_SET_METHOD(exports, "Runas", Runas);

  NODE_SET_METHOD(exports, "RegisterLogMessage", RegisterLogMessage);

  InitMonitoring();
}

NODE_MODULE(sysDarwin, Init)

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