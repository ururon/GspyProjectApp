#include <windows.h>
#include <node.h>
#include <v8.h>

#include <win32Device.h>

using namespace v8;

void GetDeviceList(const FunctionCallbackInfo<Value>& args);

extern "C" {
	void Initialize(Handle<Object> exports) {
	  	Isolate* isolate = Isolate::GetCurrent();
        DEBUG_LOG("Initialize .", NULL);

		int res = hid_init();
		exports->Set(String::NewFromUtf8(isolate,"INIT_ERROR"), Number::New(isolate,res));
		if (res != 0) {
			return;
		}

		NODE_SET_METHOD(exports, "getDeviceList", GetDeviceList);
        
		Device::init(isolate,exports);
	}

	NODE_MODULE(hidWin, Initialize);
}

void GetDeviceList(const v8::FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);

    struct hid_device_info *devs, *cur_dev;
    int i, pathLen = 0, idxIface=0, idxDevs=0;
    char * tmpPath;
    Handle<Array> arrAllR = Array::New(isolate,MAX_DEVICE_NUM);
    Handle<Array> arrOneDev ;
    char strInterfaceType[100] = {0};

    devs = hid_enumerate(0x0, 0x0);
    cur_dev = devs; 
    while (cur_dev) {
        if(cur_dev->location_path == NULL){
            cur_dev = cur_dev->next;
            continue;
        }
        for (i = 0; i < MAX_DEVICE_NUM; i++)
        {
            idxDevs = i;
            idxIface = 0;
            Local<Value> aDev = arrAllR->Get(i);
            if(aDev->IsUndefined())
                break;
            bool bFound = false; 
            String::Utf8Value valPath(aDev->ToObject()->Get(String::NewFromUtf8(isolate,"sLocationPath"))->ToString()); 
            pathLen = valPath.length();
            tmpPath = (char*)calloc(pathLen+1, sizeof(char));
            strncpy(tmpPath, *valPath, pathLen);
            if (!strcmp(tmpPath, cur_dev->location_path)) { 
                bFound = true;
                Local<Array> arr=Local<Array>::Cast(aDev->ToObject()->Get(String::NewFromUtf8(isolate,"Interface")));
                for (int n = 0; n < MAX_INTERFACE_NUM; n++)
                {
                    if(arr->Get(n)->IsUndefined()){
                        break;
                    }
                    idxIface++;
                }
            } 
            free(tmpPath);
            tmpPath=NULL;
            if(bFound)
                break;
        }
        if(idxIface==0)
            arrOneDev = Array::New(isolate,MAX_INTERFACE_NUM);
        else{
            arrOneDev = Local<Array>::Cast(arrAllR->Get(idxDevs)->ToObject()->Get(String::NewFromUtf8(isolate,"Interface")));
        }

        _snprintf_s(strInterfaceType, sizeof(strInterfaceType), "%04X_%04X", cur_dev->usage_page, cur_dev->usage);

        Local<Object> oneIface = Object::New(isolate);
        oneIface->Set(String::NewFromUtf8(isolate,"iInterfaceNumber"),Int32::New(isolate,cur_dev->interface_number));
        oneIface->Set(String::NewFromUtf8(isolate,"sInterfaceType"), String::NewFromUtf8(isolate, strInterfaceType));
        oneIface->Set(String::NewFromUtf8(isolate,"iMaxReportLength"),Uint32::New(isolate,cur_dev->max_report_length));
        oneIface->Set(String::NewFromUtf8(isolate,"iReportNumber"),Int32::New(isolate,cur_dev->report_number));
        oneIface->Set(String::NewFromUtf8(isolate,"sPath"),String::NewFromUtf8(isolate,cur_dev->path));
        arrOneDev->Set(idxIface,oneIface);
        Local<Object> oneDev = Object::New(isolate);
        oneDev->Set(String::NewFromUtf8(isolate,"idVendor"),Uint32::New(isolate,cur_dev->vendor_id));
        oneDev->Set(String::NewFromUtf8(isolate,"idProduct"),Uint32::New(isolate,cur_dev->product_id));
        oneDev->Set(String::NewFromUtf8(isolate,"sLocationPath"),String::NewFromUtf8(isolate,cur_dev->location_path));
        oneDev->Set(String::NewFromUtf8(isolate,"Interface"),arrOneDev);
        arrAllR->Set(idxDevs,oneDev);
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);
    args.GetReturnValue().Set(arrAllR);
}