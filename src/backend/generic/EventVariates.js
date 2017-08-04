/*!---------------------------------------------------------
* Copyright (C) Genius Corporation. All rights reserved.
*--------------------------------------------------------*/
'use strict';

var eventTypes = {
	//发生错误
	Error : 'Error',
    //读写USB时出错
    USBError: 'USBError',
	//刷新设备列表
    RefreshDevice : 'RefreshDevice',
    //分位不支持
    FirmwareNotSupport: 'FwNotSupport',
    //切换DPI
    DPIChanged: 'DPIChanged',
    //程序切换
    AppChanged: 'AppChanged',
    //Windows Session切换
    SessionChanged: 'SessionChanged',
    //设备拔插
    HotPlug : 'HotPlug',
    //SmartProtocol通过此事件通知
    ProtocolMessage : 'ProtocolMessage',
    //下载进度
    DownloadProgress : 'DownloadProgress',
    //需要通过安装包更新
    InstallPackUpdate : 'InstallPackUpdate',
    //检查更新下载完成
    CheckUpdateComplete : 'CheckUpdateComplete',
    //需要安装驱动
    InstallDriver : "InstallDriver",
    //开始写入分位
    StartWriteFirmware : 'StartWriteFirmware',
    //分位更新进度
    UpdateFirmwareProgess : 'UpdateFirmwareProgess',
    //更新分位完成
    UpdateFirmwareComplete : 'UpdateFirmwareComplete',
    //更新分位后，等待设备连接超时
    UpdateFirmwareReConnectTimeout : 'UpdateFirmwareReConnectTimeout',
    //电池电量状态改变
    BatteryStateChanged : 'BatteryStateChanged',
    //工作状态改变
    WorkStateChanged : 'WorkStateChanged',
    //充电状态改变
    ChargingStateChanged : 'ChargingStateChanged',
    //电池电量改变
    ADCStateChanged : 'ADCStateChanged',
    //设备驱动更新
    DeviceDriverUpdate : 'DeviceDriverUpdate',
    //新通知通知
    NewNotification :'NewNotification',
    //网络在线通知
    NetWorkOnline:"NetWorkOnline",
    //网络掉线通知
    NetWorkOffline:"NetWorkOffline"
};

exports.EventTypes = eventTypes;