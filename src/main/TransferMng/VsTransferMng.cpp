#include "VsTransferMng.h"
#include "SubTopic.h"
#include "log4cxx/Loging.h"
#include "comm/ReleaseThread.h"
#include "comm/CommFun.h"
#include "MsgBusMng/MsgBusMng.h"

using namespace std;
using namespace MAIN_MNG;
using namespace IF_VIDEO_SERVICE;
using namespace common_cmmobj;
using namespace common_template;

void CVsTransferMng::StartCameras()
{
	//获取rtsp通信方式配置对象
	vector<nlohmann::json> vecSouthRtspTransCfg;
	string&& strKey = strTransferNode + strTransferSouthVideoNode + strAttriRtspCliValue;
	SCCfgMng.GetSysCfgJsonObj(strKey,vecSouthRtspTransCfg);
    if(vecSouthRtspTransCfg.empty())
    {
        return;
    }

	//启动多个摄像头拉流
    MapCamBaseInfo mapCameraInfo;
	for (auto elm : vecSouthRtspTransCfg)
	{
        string&& strEnable = elm[strAttriEnable];
        if("0" == strEnable)
        {
            continue;
        } 
        TCameraBaseInfo tCameraInfo;
        tCameraInfo.strIP = elm[strAttriIp];
        tCameraInfo.s32Port = ::atoi(string(elm[strAttriPort]).c_str());
        tCameraInfo.strUserName = elm[strAttriParams][strAttriUserName];
        tCameraInfo.strPassword = elm[strAttriParams][strAttriPwd];
        tCameraInfo.strCameraCode = elm[strAttriParams][strAttriCode];

        mapCameraInfo.insert(pair<string,TCameraBaseInfo>(tCameraInfo.strCameraCode,tCameraInfo));
	}
    if(mapCameraInfo.empty())
    {
        return;
    }

    //init vs module
    bool bRet = GetVSModuleInstance()->InitModule(this);
    if(!bRet)
    {
        return;
    }

    //start vs module
    bRet = GetVSModuleInstance()->StartModule(mapCameraInfo);
    if(!bRet)
    {
        return;
    }

    //start pull vides thread
    for (auto elm : vecSouthRtspTransCfg)
	{
        string&& strIp = elm[strAttriIp];
        string&& strCamCode = elm[strAttriParams][strAttriCode];
        if(!strCamCode.empty() || !strIp.empty())
        {
            m_mapPullStream[strCamCode] = ThreadShrPtr(new common_cmmobj::CThread(0, 10, "AliveLoop",-1,
                                                                                  std::bind(&CVsTransferMng::PullVideLoop,this,placeholders::_1,placeholders::_2),
                                                                                  strIp,
                                                                                  strCamCode));
        }
    }
}

bool CVsTransferMng::ReportCameraRunState(const string& CameraCode, const int state)
{
    return true;
}

void CVsTransferMng::PullVideLoop(const string& strIp,const string& strCamCode)
{
    st_cvMat cvMat;
    GetVSModuleInstance()->GetCmmImage(strCamCode,cvMat);
    if(0 >= cvMat.srcMat.cols || 0 >= cvMat.srcMat.rows)
    {
        return;
    }

    string&& strTopic = RtspStreamTopic(strIp,strCamCode);
	SCMsgBusMng.GetMsgBus()->SendReq<void, const string&,const string&,const cv::Mat&>(strIp,strCamCode,cvMat.srcMat, strTopic);
}

CVsTransferMng::~CVsTransferMng()
{
    for (auto val : m_mapPullStream)
    {
        ReleaseThread(val.second);
    }
}