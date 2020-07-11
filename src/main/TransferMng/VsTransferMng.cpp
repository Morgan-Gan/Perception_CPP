#include "VsTransferMng.h"
#include "SubTopic.h"

using namespace std;
using namespace MAIN_MNG;
using namespace IF_VIDEO_SERVICE;

void CVsTransferMng::StartCameras()
{
	//获取rtsp通信方式配置对象
	vector<tuple<unsigned short, int>> vecRtspParam;
	vector<nlohmann::json> vecSouthRtspTransCfg;
	string&& strKey = strTransferNode + strTransferSouthNode + strRtspCliType;
	SCCfgMng.GetCfgJsonObj(strKey,vecSouthRtspTransCfg);

	//启动多个摄像头拉流
    MapCamBaseInfo mapCameraInfo;
	for (auto elm : vecSouthRtspTransCfg)
	{
        TCameraBaseInfo tCameraInfo;
        tCameraInfo.strIP = elm["ip"];
        tCameraInfo.s32Port = ::atoi(string(elm["port"]).c_str());
        tCameraInfo.strUserName = elm["username"];
        tCameraInfo.strPassword = elm["pwd"];
        tCameraInfo.strCameraCode = elm["code"];

        mapCameraInfo.insert(pair<string,TCameraBaseInfo>(tCameraInfo.strCameraCode,tCameraInfo));
	}

    GetVSModuleInstance()->StartModule(mapCameraInfo);
}

bool CVsTransferMng::ReportCameraRunState(const string& CameraCode, const int state)
{
    return true;
}