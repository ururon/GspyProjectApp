#include "darwinDevice.h"

using namespace v8;

#define RPT_QTY 5

struct input_report {
	uint8_t *data;
	size_t len;
	struct input_report *next;
};

void handleCompletion(HidTransfer* trans);

pthread_mutex_t mutexInputReport;
bool isTransferGetOtherReport = false;
uint32_t reportNumList[RPT_QTY];
struct input_report * inputReportBufs[20] = {NULL};

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
	pthread_mutex_init(&mutexInputReport, NULL);
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
	pthread_mutex_destroy(&mutexInputReport);
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
		if(strcmp(dev->hidInterfaces[i].InterfaceType, sFaceType) == 0 && dev->hidInterfaces[i].ReportNumber == iReportNum){
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
	uint32_t adjLen = uReportId == 0 ? 0 : 1;

	HidInterface stHid = dev->hidInterfaces[dev->iFeatureReportIndex];
	if (stHid.MaxReportLength != wLength + adjLen){
		char buf[100];
		sprintf(buf,"wrong length,must be %d.",stHid.MaxReportLength - adjLen);
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, buf)));
		return;
	}
	wLength++;

	unsigned char *data=(unsigned char *)calloc(wLength + 1, sizeof(unsigned char));
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

	Local<Object> buf;
	if (uReportId == 0)
		buf = node::Buffer::Copy(isolate, (char *)data, res).ToLocalChecked();
	else
		buf = node::Buffer::Copy(isolate, (char *)(data+1), res-1).ToLocalChecked();
	free(data);
	data = NULL;
	printf("buf:");
	printf("\r\n");
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
	uint32_t adjLen = uReportId == 0 ? 0 : 1;

	HidInterface stHid = dev->hidInterfaces[dev->iFeatureReportIndex];
	if (stHid.MaxReportLength != wLength + adjLen){
		char buf[100];
		sprintf(buf,"wrong length,must be %d.",stHid.MaxReportLength - adjLen);
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, buf)));
		return;
	}

	unsigned char* inBuf = (unsigned char*)node::Buffer::Data(args[2]->ToObject());
	int res;
	unsigned char *data=(unsigned char *)calloc(wLength + 2, sizeof(unsigned char));
	if(data==NULL){
 		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"calloc memory error.")));
		return;
 	}
	data[0] = uReportId;
	memcpy(data + 1, inBuf, wLength);
	res = hid_send_feature_report(stHid.HidHandle, data, wLength + 1);
	free(data);
	data = NULL;
	if (res < 0) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"Unable to send a feature report.")));
		return;
	}
	args.GetReturnValue().Set(res);
}

bool isHasReportNumber(uint32_t num)
{
	for(int i = 0;i < RPT_QTY; i++){
		if (reportNumList[i] == 0)
			break;
		if (reportNumList[i] == num)
			return true;
	}
	return false;
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
	wLength++;

	unsigned char *data=(unsigned char *)calloc(wLength + 1, sizeof(unsigned char));
 	if(data==NULL){
 		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"calloc memory error.")));
		return;
 	}
	int res = 0;
	uint32_t readCount = 0;
	if (isTransferGetOtherReport && uReportId < 20 && uReportId > 0 && isHasReportNumber(uReportId)){
		while(res == 0){
			pthread_mutex_lock(&mutexInputReport);
			if (inputReportBufs[uReportId] != NULL){
				struct input_report *rpt = inputReportBufs[uReportId];
				res = rpt->len;
				memcpy(data, rpt->data, res);
				inputReportBufs[uReportId] = rpt->next;
				free(rpt->data);
				free(rpt);
			}
			pthread_mutex_unlock(&mutexInputReport);
			if (res == 0){
				readCount++;
				if (readCount > 80)
					break;
				usleep(10000);
			}
		}
	}else{
		while(true)
		{
			data[0] = uReportId;
			hid_set_nonblocking(stHid.HidHandle, 0);
			res = hid_read(stHid.HidHandle, data, wLength + 1);
			if (res < 0) {
				isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"Unable to read data.")));
				return;
			}
			if (res != wLength || (uReportId != 0 && uReportId != data[0])){
				readCount++;
				if (readCount > 50)
					break;
				DEBUG_LOG("Read data error,Length:%d,first byte:%d.",res,data[0]);
			}else
				break;
		}
	}
	if (uReportId > 0 && res <= 0){
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"Read Data Error,has no data.")));
		return;
	}
	Local<Object> buf;
	if (uReportId == 0)
		buf = node::Buffer::Copy(isolate, (char *)data, res).ToLocalChecked();
	else
		buf = node::Buffer::Copy(isolate, (char *)(data+1), res - 1).ToLocalChecked();
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
	unsigned char *data=(unsigned char *)calloc(wLength + 1, sizeof(unsigned char));
	if(data==NULL){
 		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"calloc memory error.")));
		return;
 	}
	if (isTransferGetOtherReport && uReportId < 20 && uReportId > 0 && isHasReportNumber(uReportId)){
		pthread_mutex_lock(&mutexInputReport);
		while (inputReportBufs[uReportId] != NULL){
			struct input_report *rpt = inputReportBufs[uReportId];
			inputReportBufs[uReportId] = rpt->next;
			free(rpt->data);
			free(rpt);
		}
		pthread_mutex_unlock(&mutexInputReport);
	}
	data[0] = uReportId;
	memcpy(data + 1, inBuf, wLength);
	res = hid_write(stHid.HidHandle, data, wLength + 1);
	free(data);
	data = NULL;
	if (res < 0) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"Unable to write data.")));
		return;
	}
	args.GetReturnValue().Set(res);
}

void TransferMessageAsync(uv_work_t* req)
{
}
void TransferMessageFinished(uv_work_t* req)
{
	HidTransfer * trans = static_cast<HidTransfer *>(req->data);
  	handleCompletion(trans);
}

void TransferMessage(HidTransfer * trans){
    uv_work_t* reqLogChg = new uv_work_t();
	reqLogChg->data = trans;
    uv_queue_work(uv_default_loop(), reqLogChg, TransferMessageAsync, (uv_after_work_cb)TransferMessageFinished);
}

void TransferThreadFn(void* d)
{
	int res = 0;
	HidTransfer * trans=(HidTransfer *)d;
	HidTransfer * transCallback;
	DEBUG_LOG("TransferThreadFn:%p", trans);
	while(trans && trans->OpenStatus)
	{
		memset(trans->TransferBuffer, 0, sizeof(trans->TransferBuffer));
		if (trans->ReportNum > 0)
			trans->TransferBuffer[0] = trans->ReportNum;
		hid_set_nonblocking(trans->HidHandle, 0);
		res = hid_read(trans->HidHandle, trans->TransferBuffer, sizeof(trans->TransferBuffer));
		if(res > 0){
			if(trans->ReportNum <= 0 || trans->ReportNum == trans->TransferBuffer[0]){
				trans->TransferActualLength = res;
				transCallback = (HidTransfer *)calloc(1, sizeof(HidTransfer));
				memcpy(transCallback, trans, sizeof(HidTransfer));
				TransferMessage(transCallback);
			}else{
				if (isTransferGetOtherReport && trans->TransferBuffer[0] < 20 && trans->TransferBuffer[0] > 0 && isHasReportNumber(trans->TransferBuffer[0])){
					struct input_report *rpt  = (struct input_report *)calloc(1, sizeof(struct input_report));
					rpt->data = (uint8_t *)calloc(1, res);
					memcpy(rpt->data, trans->TransferBuffer, res);
					rpt->len = res;
					rpt->next = NULL;
					
					pthread_mutex_lock(&mutexInputReport);
					struct input_report *numRpt = inputReportBufs[trans->TransferBuffer[0]];
					if (numRpt == NULL) {
						inputReportBufs[trans->TransferBuffer[0]] = rpt;
					}
					else {
						struct input_report *cur = numRpt;
						while (cur->next != NULL) {
							cur = cur->next;
						}
						cur->next = rpt;
					}
					pthread_mutex_unlock(&mutexInputReport);
				}
			}
		}else
			usleep(5000);
	}
	trans->HidHandle = NULL;
	DEBUG_LOG("TransferThreadFn Exit : %p", trans);
}

void handleCompletion(HidTransfer* trans)
{
	Isolate* isolate = Isolate::GetCurrent();
  	HandleScope scope(isolate);

  	Local<Value> cbVal=Local<Value>::New(isolate, trans->TransferCallback);
	Local<Function> cb = Local<Function>::Cast(cbVal);
	const unsigned argc = 2;
	Local<Object> buf = node::Buffer::Copy(isolate, (char *)trans->TransferBuffer, trans->TransferActualLength).ToLocalChecked();
	Local<Value> argv[argc] = { 
		String::NewFromUtf8(isolate,trans->DeviceSN),
		buf
	};
	free(trans);
	trans = NULL;
	cb->Call(isolate->GetCurrentContext()->Global(), argc, argv);
}


void Device::Device_StartTransfer(const FunctionCallbackInfo<Value>& args){
	Isolate* isolate = args.GetIsolate();
	HandleScope scope(isolate);
	if (args.Length() < 4) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"function can only be 4 argument:[interfaceType, reportNumber, sn, callback]")));
		return;
	}
	if (!args[0]->IsString() || !args[1]->IsInt32() || !args[2]->IsString() || !args[3]->IsFunction()) {
		isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,"arguments can only be String,Int32,String,Function.")));
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
		if(strcmp(dev->hidInterfaces[i].InterfaceType, sFaceType) == 0){
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
	isTransferGetOtherReport = false;
	if (!args[4]->IsUndefined() && args[4]->IsArray()){
		Local<Array> arr = Local<Array>::Cast(args[4]->ToObject());
		for (i = 0; i < RPT_QTY; i++)
		{
			if(arr->Get(i)->IsUndefined() || !arr->Get(i)->IsUint32())
				break;
			isTransferGetOtherReport = true;
			reportNumList[i] = arr->Get(i)->Uint32Value();
		}
	}
	hidTrans->ReportNum = iReportNum;
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

	Device* dev = Device::Unwrap<Device>(args.Holder());
	DEBUG_LOG("SetTransferSn : %p",dev);
	int i = 0;

	HidTransfer * hidTrans = NULL;
	for (i = 0; i < MAX_INTERFACE_NUM; i++)
	{
		if(strcmp(dev->hidInterfaces[i].InterfaceType, sFaceType) == 0){
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