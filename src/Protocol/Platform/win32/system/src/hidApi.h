#ifndef _GENIUS_HID_API_H
#define _GENIUS_HID_API_H

#include <windows.h>

typedef struct _HIDD_ATTRIBUTES{
	ULONG Size;
	USHORT VendorID;
	USHORT ProductID;
	USHORT VersionNumber;
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

typedef USHORT USAGE;
typedef struct _HIDP_CAPS {
	USAGE Usage;
	USAGE UsagePage;
	USHORT InputReportByteLength;
	USHORT OutputReportByteLength;
	USHORT FeatureReportByteLength;
	USHORT Reserved[17];
	USHORT fields_not_used_by_hidapi[10];
} HIDP_CAPS, *PHIDP_CAPS;
typedef void* PHIDP_PREPARSED_DATA;

#define HIDP_STATUS_SUCCESS 0x110000

typedef BOOLEAN (__stdcall *HidD_GetAttributes_)(HANDLE device, PHIDD_ATTRIBUTES attrib);
typedef BOOLEAN (__stdcall *HidD_GetSerialNumberString_)(HANDLE device, PVOID buffer, ULONG buffer_len);
typedef BOOLEAN (__stdcall *HidD_GetManufacturerString_)(HANDLE handle, PVOID buffer, ULONG buffer_len);
typedef BOOLEAN (__stdcall *HidD_GetProductString_)(HANDLE handle, PVOID buffer, ULONG buffer_len);
typedef BOOLEAN (__stdcall *HidD_SetFeature_)(HANDLE handle, PVOID data, ULONG length);
typedef BOOLEAN (__stdcall *HidD_GetFeature_)(HANDLE handle, PVOID data, ULONG length);
typedef BOOLEAN (__stdcall *HidD_GetIndexedString_)(HANDLE handle, ULONG string_index, PVOID buffer, ULONG buffer_len);
typedef BOOLEAN (__stdcall *HidD_GetPreparsedData_)(HANDLE handle, PHIDP_PREPARSED_DATA *preparsed_data);
typedef BOOLEAN (__stdcall *HidD_FreePreparsedData_)(PHIDP_PREPARSED_DATA preparsed_data);
typedef ULONG (__stdcall *HidP_GetCaps_)(PHIDP_PREPARSED_DATA preparsed_data, HIDP_CAPS *caps);
typedef BOOLEAN (__stdcall *HidD_SetNumInputBuffers_)(HANDLE handle, ULONG number_buffers);
typedef VOID (__stdcall *HidD_GetHidGuid_)(LPGUID);

HidD_GetAttributes_ HidD_GetAttributes;
HidD_GetSerialNumberString_ HidD_GetSerialNumberString;
HidD_GetManufacturerString_ HidD_GetManufacturerString;
HidD_GetProductString_ HidD_GetProductString;
HidD_SetFeature_ HidD_SetFeature;
HidD_GetFeature_ HidD_GetFeature;
HidD_GetIndexedString_ HidD_GetIndexedString;
HidD_GetPreparsedData_ HidD_GetPreparsedData;
HidD_FreePreparsedData_ HidD_FreePreparsedData;
HidP_GetCaps_ HidP_GetCaps;
HidD_SetNumInputBuffers_ HidD_SetNumInputBuffers;
HidD_GetHidGuid_ HidD_GetHidGuid;

HMODULE lib_handle = NULL;
int InitHidFunctions();
int FreeHidFunctions();

#endif