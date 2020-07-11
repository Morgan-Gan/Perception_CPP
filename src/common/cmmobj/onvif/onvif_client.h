#pragma once

#define  MAX_STREAM_URI_LEN 256

enum ENUM_IPC_STATE 
{
	IPC_MEDIA_ERROR = -1,
	IPC_MEDIA_OK = 0
};

struct IpcInformation
{
	char* ip;
	char* username;
	char* password;
	int channel;
	char* profile_token;
	char* encodec_token;
	enum ENUM_IPC_STATE state;
	int width;
	int height;
	char rtsp_uri[MAX_STREAM_URI_LEN];
};

/************************************************************************
**函数：ONVIF_GetProfiles
**功能：获取IPC的信息
**参数：
        [in]  ip            - IPC的IP
        [in]  username      - 有户名
        [in]  password      - 密码
        [out] channel       - 码流
        [in]  ipc_info      - 获取的IPC会保存在此ipc_info_t结构体中
**返回：
        0成功，非0失败
************************************************************************/

int ONVIF_GetIPCInformation(const char *ip, const char* username, const char* password, int channel, struct IpcInformation* ipc_info);

