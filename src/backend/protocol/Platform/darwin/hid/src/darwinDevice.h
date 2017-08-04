#ifndef _GENIUS_DEVICE_H
#define _GENIUS_DEVICE_H

#include <stdlib.h>
#include <stdio.h>
#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <node_object_wrap.h>

#include <map>
#include <uv.h>
#include <hidapi.h>

#define MAX_INTERFACE_NUM	20
#define MAX_DEVICE_NUM		50

using namespace v8;

struct HidInterface {
	int InterfaceNumber;
	char InterfaceType[100];
	uint32_t MaxReportLength;
	int ReportNumber;
	hid_device *HidHandle;
	char Path[255];
};

struct HidTransfer
{
	hid_device *HidHandle;
	int TransferActualLength;
	unsigned char TransferBuffer[255];
	char DeviceSN[100];
	Persistent<Value> TransferCallback;
	uv_thread_t trans_thread;
	int ReportNum;
	bool OpenStatus;
};

class Device: public node::ObjectWrap {
	/*hid_device *rwHandle;
	
	char * rwPath;
	char * transPath;
	bool bHaveTransferInterface,bHaveRWInterface;
	uv_thread_t trans_thread;*/

	int idVendor, idProduct, iFeatureReportIndex;
	char *sLocationPath;

	struct HidInterface hidInterfaces[MAX_INTERFACE_NUM];
	
public:
	static void init(Isolate* isolate,Handle<Object> exports);
	struct HidTransfer hidTransfers[MAX_INTERFACE_NUM];
	/*hid_device *transHandle;
	Persistent<Value> TransferCallback;
	unsigned char transferBuffer[65];
	int transferActualLength;
	char DevSN[20];*/

private:
	~Device();
	Device(Local<Object> obj);

	static void New(const FunctionCallbackInfo<Value>& args);
	static void Device_Open(const FunctionCallbackInfo<Value>& args);
	static void Device_Close(const FunctionCallbackInfo<Value>& args);
	static void Device_HidGetReport(const FunctionCallbackInfo<Value>& args);
	static void Device_HidSetReport(const FunctionCallbackInfo<Value>& args);
	static void Device_StartTransfer(const FunctionCallbackInfo<Value>& args);
	static void Device_SetTransferSn(const FunctionCallbackInfo<Value>& args);
	static void Device_HidRead(const FunctionCallbackInfo<Value>& args);
	static void Device_HidWrite(const FunctionCallbackInfo<Value>& args);
};

#ifdef DEBUG
	#define DEBUG_HEADER fprintf(stderr, "HID [%s:%s() %d]: ", __FILE__, __FUNCTION__, __LINE__); 
	#define DEBUG_FOOTER fprintf(stderr, "\n");
	#define DEBUG_LOG(...) DEBUG_HEADER fprintf(stderr, __VA_ARGS__); DEBUG_FOOTER
#else
 	#define DEBUG_LOG(...)
#endif

#endif