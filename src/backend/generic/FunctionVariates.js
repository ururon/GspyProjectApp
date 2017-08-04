/*!---------------------------------------------------------
* Copyright (C) Genius Corporation. All rights reserved.
*--------------------------------------------------------*/
'use strict';

var FuncName = {
	//设备初始化
    InitDevice : "InitDevice",
    //获取滚轮速度
    GetDeviceWheelSpeed : "GetDeviceWheelSpeed",
    //设置滚轮速度
    SetDeviceWheelSpeed : "SetDeviceWheelSpeed",
    //获取系统鼠标移动速度
    GetMousePointSpeed : "GetMousePointSpeed",
    //设置系统鼠标移动速度
    SetMousePointSpeed : "SetMousePointSpeed",
    //获取系统双击速度
    GetMouseDoubleClickSpeed : "GetMouseDoubleClickSpeed",
    //设置系统双击速度
    SetMouseDoubleClickSpeed : "SetMouseDoubleClickSpeed",
    //获取左右键状态
    GetSwapButtonState : "GetSwapButtonState",
    //设置左右键状态
    SetSwapButtonState : "SetSwapButtonState",
    //设置DPI档位，并开启
    SetDeviceGradOnValue : "SetDeviceGradOnValue",
    //关闭指定档位
    SetDeviceGradOff : "SetDeviceGradOff",
    //获取当前档位
    GetDeviceGradToggle : "GetDeviceGradToggle",
    //设置当前档位
    SetDeviceGradToggle : "SetDeviceGradToggle",
    //设置滚轮LED显示模式
    SetDeviceWheelLedShowMode : "SetDeviceWheelLedShowMode",
    //设置Logo LED显示模式
    SetDeviceLogoLedShowMode : "SetDeviceLogoLedShowMode",
    //设置Logo LED自定颜色
    SetDeviceLogoLedColorCustom : "SetDeviceLogoLedColorCustom",
    //设置Logo LED预设颜色
    SetDeviceLogoLedColorSystem : "SetDeviceLogoLedColorSystem",
    //设置Logo LED呼吸速度
    SetDeviceLogoLedBreathRate : "SetDeviceLogoLedBreathRate",
    //设置Logo LED档位打开
    SetDeviceLogoLedBreathGradeOn : "SetDeviceLogoLedBreathGradeOn",
    //设置Logo LED档位关闭
    SetDeviceLogoLedBreathGradeOff : "SetDeviceLogoLedBreathGradeOff",
    //设置ReportRate
    SetDeviceReportRate : "SetDeviceReportRate",
    //设置按键、键盘功能
    SetDeviceButtonKey : "SetDeviceButtonKey",
    //设置宏
    SetDeviceMacro : "SetDeviceMacro",
    //删除宏
    DeleteDeviceMacro : "DeleteDeviceMacro",
    //切换profile
    ToggleProfile : "ToggleProfile",
    //添加profile
    AddProfile : "AddProfile",
    //修改Profile
    ModifyProfile:"ModifyProfile",
    //删除profile
    DeleteProfile : "DeleteProfile",
    //刷新profile
    RefreshProfile : "RefreshProfile",
    //修改App关联
    ModifyProfileAppList : "ModifyProfileAppList",
    //恢复出厂设置
    FactoryReset : "FactoryReset",
    //更新分位
    UpdateFirmware : "UpdateFirmware",
    //更新分位和Boot
    UpdateFirmwareBoot : "UpdateFirmwareBoot",
    //设置语言
    SetLanguage : "SetLanguage",
    //获取电池状态
    GetBatteryStatus : "GetBatteryStatus",
    //获取电池状态
    GetChargeStatus : "GetChargeStatus",
    //下载安装包
    DownloadInstallPackage : "DownloadInstallPackage",
    //下载安装驱动
    DownloadInstallDriver : "DownloadInstallDriver",
    //更新驱动
    UpdateDriver : "UpdateDriver",
    //检测驱动是否安装
    CheckDriverInstall : "CheckDriverInstall",
    //检测更新
    CheckUpdateFromNet : "CheckUpdateFromNet",
    //初始指定设备，读取、写入Profile
    InitGetSNProfiles : "InitGetSNProfiles",
    //设置键盘值
    SetKeyBoardKeyValue:"SetKeyBoardKeyValue",
     //新通知通知
    DeviceDriverUpdate : "DeviceDriverUpdate",
    //新通知通知
    NewNotification :'NewNotification',
    //找到未烧序号的Device
    NotFormatSn:"NotFormatSn",
    //获取可执行文件的图标
    getApplicationIcon:"getApplicationIcon",
    //修改Macro通知
    MacroModifyNotice:'MacroModifyNotice',
    //删除Macro通知
    MacroDeleteNotice:'MacroDeleteNotice',  
};

var FuncType = {
    System : 0x01,
    Mouse : 0x02,
    Keyboard :0x03
};

exports.Names = FuncName;
exports.Types = FuncType;