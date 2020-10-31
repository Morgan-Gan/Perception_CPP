#include "FaceAlg.h"
#include "log4cxx/Loging.h"
#include "boost/BoostFun.h"
#include "SubTopic.h"
#include "endecode/Base64.h"
#include "comm/CommFun.h"
#include "CommData.h"
#include "comm/FuncInitLst.h"

using namespace std;
using namespace cv;
using namespace common_cmmobj;
using namespace common_template;
using namespace Vision_FaceAlg;

const int net_width = 800;
const int net_height = 464;

bool CFaceAlg::Init(const MsgBusShrPtr& ptrMsgBus,const Json& taskCfg,const Json& algCfg,const Json& DataSrcCfg)
{
    //消息总线初始化
	if(!m_ptrMsgBus)
	{
		m_ptrMsgBus = ptrMsgBus;
	}
	
	//结果上报主题列表
	string&& strKey = string(taskCfg[strAttriType]) + string(taskCfg[strAttriIdx]);
	auto it = m_mapResultReportTopic.find(strKey);
	if(it == m_mapResultReportTopic.end())
	{
		m_mapResultReportTopic[move(strKey)] = 0;
	}

	//算法初始化
	AlgInitLst(algCfg,m_ptrFaceDetect,m_ptrFaceAlign);

	//数据源配置初始化
	return DataSrcInit(DataSrcCfg);
}

bool CFaceAlg::DataSrcInit(const Json& DataSrcCfg)
{
	//订阅视频流数据
	string&& strIp = DataSrcCfg[strAttriIp];
	string&& strTopic = RtspStreamTopic(strIp,DataSrcCfg[strAttriParams][strAttriCode]);
	auto&& fun = [this](const string& strIp,const string& strCamCode,const cv::Mat& srcImg){ProcVideoMat(strIp,strCamCode,srcImg);};
	m_ptrMsgBus->Attach(move(fun),move(strTopic));
	return true;
}

void CFaceAlg::ProcVideoMat(const std::string& strIp,const std::string& strCameCode,const cv::Mat& srcImg)
{
    /*预处理*/
	int s32Width = net_width;
	int s32Height = net_height;
    cv::Mat dstImg;
    bool bResize = Preprocess(srcImg,dstImg,s32Width,s32Height);
}

bool CFaceAlg::Preprocess(const cv::Mat& srcImg,cv::Mat& dstImg,int& s32Width,int& s32Height)
{
    bool bResize = false;
	if(net_width != srcImg.cols || net_height != srcImg.rows)
	{
		s32Height = s32Width * srcImg.rows / srcImg.cols;
		if(s32Height > net_height)
		{
			s32Height = net_height;
			s32Width = s32Height * srcImg.cols / srcImg.rows;
		}

		cv::resize(srcImg, dstImg, cv::Size(s32Width, s32Height));
    	cv::copyMakeBorder(dstImg, dstImg, 0, net_height - s32Height, 0, net_width - s32Width,cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
		bResize = true;
	}
	else
	{
		srcImg.copyTo(dstImg);
	}
	
	return bResize;
}