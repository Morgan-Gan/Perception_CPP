#pragma once
#include "IVideoServiceModule.h"

//设置摄像头信息
static void SetCameraBaseInfo(const IF_VIDEO_SERVICE::TCameraBaseInfo& SrcInfo, IF_VIDEO_SERVICE::TCameraBaseInfo& DesInfo)
{
    DesInfo.s64Id = SrcInfo.s64Id;
	DesInfo.strCameraCode = SrcInfo.strCameraCode;
	DesInfo.strCameraName=(SrcInfo.strCameraName);
	DesInfo.strDiscernSysCode=(SrcInfo.strDiscernSysCode);
	DesInfo.strDoorCode=(SrcInfo.strDoorCode);
	DesInfo.strIP=(SrcInfo.strIP);
	DesInfo.strMac=(SrcInfo.strMac);
	DesInfo.s32Port=(SrcInfo.s32Port);
	DesInfo.strUserName=(SrcInfo.strUserName);
	DesInfo.strPassword=(SrcInfo.strPassword);
	DesInfo.strProtocol=(SrcInfo.strProtocol);
	DesInfo.strCameraPixel=(SrcInfo.strCameraPixel);
	DesInfo.s32GetStreamMode=(SrcInfo.s32GetStreamMode);
	DesInfo.strTaskSet=(SrcInfo.strTaskSet);
	DesInfo.strImageGroupSet=(SrcInfo.strImageGroupSet);
	DesInfo.s32State=(SrcInfo.s32State);
	DesInfo.strInstallAddr=(SrcInfo.strInstallAddr);
	DesInfo.strCreateTime=(SrcInfo.strCreateTime);
	DesInfo.strRegion=(SrcInfo.strRegion);
	DesInfo.s32IsReport=(SrcInfo.s32IsReport);
	DesInfo.s32OffOrOn=(SrcInfo.s32OffOrOn);
	DesInfo.strRemark=(SrcInfo.strRemark);
	DesInfo.strStartTime=(SrcInfo.strStartTime);
	DesInfo.strEndTime=(SrcInfo.strEndTime);
	DesInfo.s32CoordinateX=(SrcInfo.s32CoordinateX);
	DesInfo.s32CoordinateY=(SrcInfo.s32CoordinateY);
	DesInfo.s32Width=(SrcInfo.s32Width);
	DesInfo.s32Height=(SrcInfo.s32Height);
	DesInfo.strCameraLiveId=(SrcInfo.strCameraLiveId);
}