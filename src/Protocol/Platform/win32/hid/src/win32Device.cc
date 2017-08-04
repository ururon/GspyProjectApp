#include "win32Device.h"

using namespace v8;

void handleCompletion(HidTransfer* trans);
UVQueue<HidTransfer*> completionQueue(handleCompletion);

Device::Device(Local<Object> obj){
	Isolate* isolate = Isolate::GetCurrent();

	Local<Value> arrVal = obj->Get(String::NewFromUtf8(isolate,"Interface"));
	idVendor = obj->Get(String::NewFromUtf8(isolate,"idVendor"))->Uint32Value();
	idProduct = obj->Get(String::NewFromUtf8(isolate,"idProduct"))->Uint32Value();

	String::Utf8Value valLocationPath(obj->Get(String::NewFromUtf8(isolate,"sLocationPath"))->ToString());
	int pathLen = valLocationPath.length();
	sLocationPath = (char*)calloc(pathLen+1, sizeof(char));
	strncpy(sLocationPath, *valLocationPath, pathLen);

	Local<Array> arr=Local<Array>::Cast(arrVal);
	memset(hidInterfaces, 0, sizeof(hidInterfaces));
	memset(hidTransfers, 0, sizeof(hidTransfers));
	iFeatureReportIndex = -1;

	for (int i = 0; i < MAX_INTERFACE_NUM; i++)
	{
		if(arr->Get(i)->IsUndefined())
			break;
		Local<Object> item = arr->Get(i)->ToObject();
		hidInterfaces[i].InterfaceNumber = item->Get(String::NewFromUtf8(isolate,"iInterfaceNumber"))->Int32Value();
		hidInterfaces[i].MaxReportLength = item->Get(String::NewFromUtf8(isolate,"iMaxReportLength"))->Uint32Value();
		hidInterfaces[i].ReportNumber = item->Get(String::NewFromUtf8(isolate,"iReportNumber"))->Int32Value();
		String::Utf8Value valFaceType(item->Get(String::NewFromUtf8(isolate,"sInterfaceType"))->ToString());
		pathLen = valFaceType.length();
		strncpy(hidInterfaces[i].InterfaceType, *valFaceType, pathLen);

		String::Utf8Value valPath(item->Get(String::NewFromUtf8(isolate,"sPath"))->ToString());
		pathLen = valPath.length();
		strncpy(hidInterfaces[i].Path, *valPath, pathLen);
	}
	DEBUG_LOG("Created device %p", this);
}

Device::~Device(){
	DEBUG_LOG("Freed device %p", this);
	for (int i = 0; i < MAX_INTERFACE_NUM; i++)
	{
		if(hidTransfers[i].OpenStatus){
			hidTransfers[i].OpenStatus = false;
		}
		if(hidInterfaces[i].HidHandle){
			hid_close(hidInterfaces[i].HidHandle);
			hidInterfaces[i].HidHandle = NULL;
		}
	}
}

void Device::New(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
	if (!args.IsConstructCall()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"USB Device function can only be used as a constructor")));
		return;
	}
	if (args.Length() < 1 || args[0]->IsUndefined()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"USB Device constructor requires at least one argument")));
		return;
	}
	if (!args[0]->IsObject()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"Argument can only be Object")));
		return;
	}
	Local<Object> obj = args[0]->ToObject();
  	Device* dev = new Device(obj);

  	dev->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
}

void Device::Device_Open(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	if (args.Length() != 2) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 2 argument:[interfaceType,reportNumber]")));
		return;
	}
	if (!args[0]->IsString() || !args[1]->IsInt32()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"argument type can only be String,Int32.")));
		return;
	}

	char sFaceType[100] = {0};
	String::Utf8Value valFaceType(args[0]->ToString());
	int pathLen = valFaceType.length();
	strncpy(sFaceType, *valFaceType, pathLen);
	int iReportNum = args[1]->Int32Value();

	Device* dev = Device::Unwrap<Device>(args.Holder());
	if (dev->iFeatureReportIndex > -1){
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"alread opened.")));
		return;
	}
	bool bOpened = false;
	for (int i = 0; i < MAX_INTERFACE_NUM; i++)
	{
		if(stricmp(dev->hidInterfaces[i].InterfaceType, sFaceType) == 0 )
		{
			DEBUG_LOG("InterfaceType == sFaceType ,InterfaceType =%s, ReportNumber =%d",dev->hidInterfaces[i].InterfaceType, dev->hidInterfaces[i].ReportNumber );
		}
		if(stricmp(dev->hidInterfaces[i].InterfaceType, sFaceType) == 0 && dev->hidInterfaces[i].ReportNumber == iReportNum)
		{
			DEBUG_LOG("Check to open device ,InterfaceType =%s, ReportNumber =%d",dev->hidInterfaces[i].InterfaceType, dev->hidInterfaces[i].ReportNumber );
			if (!dev->hidInterfaces[i].HidHandle)
				dev->hidInterfaces[i].HidHandle = hid_open_path(dev->hidInterfaces[i].Path);
			if (dev->hidInterfaces[i].HidHandle)
				bOpened = true;
			if (bOpened)
				dev->iFeatureReportIndex = i;
			break;
		}
	}
	args.GetReturnValue().Set(bOpened);
	DEBUG_LOG("open device %p", dev);
}

void Device::Device_Close(const FunctionCallbackInfo<Value>& args){
	Device* dev = Device::Unwrap<Device>(args.Holder());
	DEBUG_LOG("start close device %p", dev);
	for (int i = 0; i < MAX_INTERFACE_NUM; i++)
	{
		if(dev->hidTransfers[i].OpenStatus){
			dev->hidTransfers[i].OpenStatus = false;
		}

		if(dev->hidInterfaces[i].HidHandle){
			hid_close(dev->hidInterfaces[i].HidHandle);
			dev->hidInterfaces[i].HidHandle = NULL;
		}
	}
	dev->iFeatureReportIndex = -1;
	DEBUG_LOG("close device complete %p", dev);
}

void Device::Device_HidGetReport(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	if (args.Length() != 2) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 2 argument:[reportId,wLength]")));
		return;
	}
	if (!args[0]->IsInt32() || !args[1]->IsInt32()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"argument type can only be Int32.")));
		return;
	}

  	HandleScope scope(isolate);
	Device* dev = Device::Unwrap<Device>(args.Holder());
	if (dev->iFeatureReportIndex == -1){
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"please open first.")));
		return;
	}
	uint32_t uReportId = args[0]->Uint32Value();
	uint32_t wLength = args[1]->Uint32Value();

	HidInterface stHid = dev->hidInterfaces[dev->iFeatureReportIndex];
	if (stHid.MaxReportLength != wLength + 1){
		char buf[100];
		sprintf(buf,"wrong length,must be %d.",stHid.MaxReportLength - 1);
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, buf)));
		return;
	}
	wLength++;

	unsigned char *data=(unsigned char *)calloc(wLength+1, sizeof(unsigned char));
 	if(data==NULL){
 		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"calloc memory error.")));
		return;
 	}
	data[0] = uReportId;
	int res = hid_get_feature_report(stHid.HidHandle, data, wLength);
	if (res < 0) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"Unable to get a feature report.")));
		return;
	}

 	//Local<Object> buf ;
//	node::Buffer::Copy(isolate, (char *)(data+1), res-1).ToLocal(&buf);

	Local<Object> buf  = node::Buffer::Copy(isolate, (char *)(data+1), res-1).ToLocalChecked();

	free(data);
	data = NULL;
	args.GetReturnValue().Set(buf);
}

void Device::Device_HidSetReport(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	if (args.Length() != 3) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 3 argument:[reportId,wLength,bBuffer]")));
		return;
	}
	if (!args[0]->IsInt32() || !args[1]->IsInt32()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"2 argument type can only be Int32.")));
		return;
	}
	if (!args[2]->IsObject()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"last argument type can only be Object.")));
		return;
	}

	HandleScope scope(isolate);
	Device* dev = Device::Unwrap<Device>(args.Holder());

	if (dev->iFeatureReportIndex == -1){
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"please open first.")));
		return;
	}
	uint32_t uReportId = args[0]->Uint32Value();
	uint32_t wLength = args[1]->Uint32Value();

	HidInterface stHid = dev->hidInterfaces[dev->iFeatureReportIndex];
	if (stHid.MaxReportLength != wLength + 1){
		char buf[100];
		sprintf(buf,"wrong length,must be %d.",stHid.MaxReportLength - 1);
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, buf)));
		return;
	}

	unsigned char* inBuf = (unsigned char*)node::Buffer::Data(args[2]->ToObject());
	int res;
	unsigned char *data=(unsigned char *)calloc(wLength+1, sizeof(unsigned char));
	if(data==NULL){
 		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"calloc memory error.")));
		return;
 	}
	data[0] = uReportId;
	memcpy(data + 1, inBuf, wLength);
	res = hid_send_feature_report(stHid.HidHandle, data, wLength+1);
	free(data);
	data = NULL;
	if (res < 0) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"Unable to send a feature report.")));
		return;
	}
	args.GetReturnValue().Set(res);
}

void Device::Device_HidRead(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	if (args.Length() != 2) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 2 argument:[reportId,wLength]")));
		return;
	}
	if (!args[0]->IsInt32() || !args[1]->IsInt32()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"argument type can only be Int32.")));
		return;
	}

  	HandleScope scope(isolate);
	Device* dev = Device::Unwrap<Device>(args.Holder());
	if (dev->iFeatureReportIndex == -1){
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"please open first.")));
		return;
	}
	uint32_t uReportId = args[0]->Uint32Value();
	uint32_t wLength = args[1]->Uint32Value();

	HidInterface stHid = dev->hidInterfaces[dev->iFeatureReportIndex];

	unsigned char *data=(unsigned char *)calloc(wLength + 1, sizeof(unsigned char));
 	if(data==NULL){
 		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"calloc memory error.")));
		return;
 	}
	data[0] = uReportId;
	hid_set_nonblocking(stHid.HidHandle, 0);
	int res = hid_read(stHid.HidHandle, data, wLength + 1);
	if (res < 0) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"Unable to get a feature report.")));
		return;
	}

	//Local<Object> buf ;
	//node::Buffer::Copy(isolate, (char *)(data+1), res-1).ToLocal(&buf);


	Local<Object> buf = node::Buffer::Copy(isolate, (char *)(data+1), res-1).ToLocalChecked();
	//String::NewFromUtf8 aa = buf.ToString() ;
	//MessageBox(Null, buf.ToString(), "Msg title", MB_OK | MB_ICONQUESTION);


 	//Local<Object> buf;
    //if (Buffer::Copy(isolate, (char *)(data + 1)).ToLocal(&buf))

	//MaybeLocal<Object> buf = node::Buffer::New(isolate, (char *)(data + 1), res - 1);

	//MaybeLocal<Uint32> buf1 = buf.ToUint32();
	//Local<Object> buf = node::Buffer::New(isolate, (char *)(data + 1), res - 1).ToLocal();
	//Local<Object> buf = node::Buffer::New(isolate, (char *)(data + 1), res - 1);

	free(data);
	data = NULL;
	args.GetReturnValue().Set(buf);
}

void Device::Device_HidWrite(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	if (args.Length() != 3) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 3 argument:[reportId,wLength,bBuffer]")));
		return;
	}
	if (!args[0]->IsInt32() || !args[1]->IsInt32()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"2 argument type can only be Int32.")));
		return;
	}
	if (!args[2]->IsObject()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"last argument type can only be Object.")));
		return;
	}

	HandleScope scope(isolate);
	Device* dev = Device::Unwrap<Device>(args.Holder());

	if (dev->iFeatureReportIndex == -1){
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"please open first.")));
		return;
	}
	uint32_t uReportId = args[0]->Uint32Value();
	uint32_t wLength = args[1]->Uint32Value();

	HidInterface stHid = dev->hidInterfaces[dev->iFeatureReportIndex];

	unsigned char* inBuf = (unsigned char*)node::Buffer::Data(args[2]->ToObject());
	int res;
	unsigned char *data=(unsigned char *)calloc(wLength+1, sizeof(unsigned char));
	if(data==NULL){
 		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"calloc memory error.")));
		return;
 	}
	data[0] = uReportId;
	memcpy(data + 1, inBuf, wLength);
	res = hid_write(stHid.HidHandle, data, wLength + 1);
	free(data);
	data = NULL;
	if (res < 0) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"Unable to send a feature report.")));
		return;
	}
	args.GetReturnValue().Set(res);
}

void TransferThreadFn(void* d)
{
	int res = 0;
	HidTransfer * trans=(HidTransfer *)d;
	DEBUG_LOG("TransferThreadFn:%p", trans);
	while(trans && trans->OpenStatus)
	{
		memset(trans->TransferBuffer, 0, sizeof(trans->TransferBuffer));
		res = hid_read(trans->HidHandle, trans->TransferBuffer, sizeof(trans->TransferBuffer));
		if(res > 0){
			completionQueue.ref();
			trans->TransferActualLength = res;
			completionQueue.post(trans);
		}
		Sleep(200);
	}

	trans->HidHandle = NULL;
	DEBUG_LOG("TransferThreadFn Exit : %p", trans);
}

void handleCompletion(HidTransfer* trans)
{
	Isolate* isolate = Isolate::GetCurrent();
  	HandleScope scope(isolate);
  	completionQueue.unref();

  	Local<Value> cbVal=Local<Value>::New(isolate, trans->TransferCallback);
	Local<Function> cb = Local<Function>::Cast(cbVal);
	const unsigned argc = 2;

	//Local<Object> buf  ;
	//node::Buffer::Copy(isolate, (char *)trans->TransferBuffer, trans->TransferActualLength).ToLocal(&buf);

    Local<Object> buf  = node::Buffer::Copy(isolate, (char *)trans->TransferBuffer, trans->TransferActualLength).ToLocalChecked();

	//Local<Object> buf = node::Buffer::New(isolate, (char *)trans->TransferBuffer, trans->TransferActualLength).ToLocal();
	//Local<Object> buf = node::Buffer::New(isolate, (char *)trans->TransferBuffer, trans->TransferActualLength);

	Local<Value> argv[argc] = {
		String::NewFromUtf8(isolate,trans->DeviceSN),
		buf
	};
	cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
}


void Device::Device_StartTransfer(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	HandleScope scope(isolate);
	if (args.Length() != 4 || !args[3]->IsFunction()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 4 argument:[interfaceType, reportNumber, sn, callback]")));
		return;
	}
	if (!args[0]->IsString() || !args[1]->IsInt32()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"argument [interfaceType, reportNumber] can only be String,Int32.")));
		return;
	}
	char sFaceType[100] = {0};
	String::Utf8Value valFaceType(args[0]->ToString());
	int pathLen = valFaceType.length();
	strncpy(sFaceType, *valFaceType, pathLen);
	int iReportNum = args[1]->Int32Value();

	Device* dev = Device::Unwrap<Device>(args.Holder());
	DEBUG_LOG("StartTransfer : %p",dev);
	int i = 0;
	bool bOpened = false;
	HidTransfer * hidTrans;
	for (i = 0; i < MAX_INTERFACE_NUM; i++)
	{
		if(stricmp(dev->hidInterfaces[i].InterfaceType, sFaceType) == 0 && dev->hidInterfaces[i].ReportNumber == iReportNum){
			if (!dev->hidInterfaces[i].HidHandle)
				dev->hidInterfaces[i].HidHandle = hid_open_path(dev->hidInterfaces[i].Path);
			if (dev->hidInterfaces[i].HidHandle){
				bOpened = true;
				dev->hidTransfers[i].HidHandle = dev->hidInterfaces[i].HidHandle;
				dev->hidTransfers[i].OpenStatus = true;
				hidTrans = &dev->hidTransfers[i];
			}
			break;
		}
	}
	if (!bOpened) {
		args.GetReturnValue().Set(false);
		return;
	}
	hid_set_nonblocking(hidTrans->HidHandle, 1);
	hidTrans->TransferCallback.Reset(isolate,args[3]);
	String::Utf8Value valSn(args[2]->ToString());
	strncpy(hidTrans->DeviceSN, *valSn, 100);

	uv_thread_create(&hidTrans->trans_thread, TransferThreadFn, hidTrans);
	args.GetReturnValue().Set(true);
}


void Device::Device_SetTransferSn(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	HandleScope scope(isolate);
	if (args.Length() != 3) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 3 argument:[interfaceType, reportNumber, sn]")));
		return;
	}
	if (!args[0]->IsString() || !args[1]->IsInt32()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"argument [interfaceType, reportNumber] can only be String,Int32.")));
		return;
	}
	char sFaceType[100] = {0};
	String::Utf8Value valFaceType(args[0]->ToString());
	int pathLen = valFaceType.length();
	strncpy(sFaceType, *valFaceType, pathLen);
	int iReportNum = args[1]->Int32Value();

	Device* dev = Device::Unwrap<Device>(args.Holder());
	DEBUG_LOG("SetTransferSn : %p",dev);
	int i = 0;

	HidTransfer * hidTrans = NULL;
	for (i = 0; i < MAX_INTERFACE_NUM; i++)
	{
		if(stricmp(dev->hidInterfaces[i].InterfaceType, sFaceType) == 0 && dev->hidInterfaces[i].ReportNumber == iReportNum){
			if (dev->hidInterfaces[i].HidHandle && dev->hidTransfers[i].OpenStatus){
				hidTrans = &dev->hidTransfers[i];
			}
			break;
		}
	}
	if (hidTrans == NULL) {
		args.GetReturnValue().Set(false);
		return;
	}

	String::Utf8Value valSn(args[2]->ToString());
	strncpy(hidTrans->DeviceSN, *valSn, 100);

	args.GetReturnValue().Set(true);
}

void Device::init(Isolate* isolate, Handle<Object> exports)
{
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  	tpl->SetClassName(String::NewFromUtf8(isolate, "Device"));
  	tpl->InstanceTemplate()->SetInternalFieldCount(1);

  	NODE_SET_PROTOTYPE_METHOD(tpl, "open", Device_Open);
  	NODE_SET_PROTOTYPE_METHOD(tpl, "close", Device_Close);
  	NODE_SET_PROTOTYPE_METHOD(tpl, "hidGetReport", Device_HidGetReport);
  	NODE_SET_PROTOTYPE_METHOD(tpl, "hidSetReport", Device_HidSetReport);

	NODE_SET_PROTOTYPE_METHOD(tpl, "hidRead", Device_HidRead);
  	NODE_SET_PROTOTYPE_METHOD(tpl, "hidWrite", Device_HidWrite);

  	NODE_SET_PROTOTYPE_METHOD(tpl, "startTransfer", Device_StartTransfer);
  	NODE_SET_PROTOTYPE_METHOD(tpl, "setTransferSn", Device_SetTransferSn);

  	exports->Set(String::NewFromUtf8(isolate, "Device"),tpl->GetFunction());
}
