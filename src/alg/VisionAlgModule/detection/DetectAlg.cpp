#include "DetectAlg.h"
#include "log4cxx/Loging.h"
#include "boost/BoostFun.h"
#include "SubTopic.h"
#include "endecode/Base64.h"
#include "comm/CommFun.h"

using namespace std;
using namespace common_cmmobj;
using namespace common_template;
using namespace Vision_DetectAlg;

const int s32Classes = 24;
const int net_width = 640;
const int net_height = 640;
const int img_channels = 3;
const int input_size = 1;
const int output_size = 4;

const float NMS_THRESH = 0.4;   
const float OBJ_THRESH = 0.5;             
const float DRAW_CLASS_THRESH = 0.3;   

bool CDetectAlg::Init(const MsgBusShrPtr& ptrMsgBus,const Json& taskCfg,const Json& algCfg,const Json& DataSrcCfg)
{
	//消息总线初始化
	if(!m_ptrMsgBus)
	{
		m_ptrMsgBus = ptrMsgBus;
	}
	
	//结果上报主题列表
	string&& strTopicKey = string(taskCfg[strAttriType]) + string(taskCfg[strAttriIdx]);
	auto ite = m_mapResultReportTopic.find(strTopicKey);
	if(ite == m_mapResultReportTopic.end())
	{
		m_mapResultReportTopic[move(strTopicKey)] = 0;
	}

	//算法初始化
	if(!AlgInit(algCfg))
	{
		return false;
	}

	//数据源配置初始化
	return DataSrcInit(DataSrcCfg);
}

bool CDetectAlg::DataSrcInit(const Json& DataSrcCfg)
{
	//订阅视频流数据
	string&& strIp = DataSrcCfg[strAttriIp];
	string&& strTopic = RtspStreamTopic(strIp,DataSrcCfg[strAttriParams][strAttriCode]);
	auto&& fun = [this](const string& strIp,const string& strCamCode,const cv::Mat& srcImg){ProcVideoMat(strIp,strCamCode,srcImg);};
	m_ptrMsgBus->Attach(move(fun),move(strTopic));
	return true;
}

bool CDetectAlg::AlgInit(const Json& algCfg)
{
	//保存算法模型
	string&& strAlgMapKey = string(algCfg[strAttriType]) + string(algCfg[strAttriIdx]);
	auto ite = m_mapAlgModel.find(strAlgMapKey);
	if(ite == m_mapAlgModel.end())
	{
		rknn_context tx;
		m_mapAlgModel.insert(make_pair<string,rknn_context>(string(strAlgMapKey),move(tx)));
	}
	else
	{
		return true;
	}
	
	// open and load model file
	string strPath = string("./model/") + string(algCfg[strAttriModel][strAttriName]);
	FILE *fp = fopen(strPath.c_str(), "rb");
	if(!fp) 
	{
		return false;
	}

	fseek(fp, 0, SEEK_END);   
	int model_len = ftell(fp);   
	void *model = malloc(model_len);
	fseek(fp, 0, SEEK_SET);   
	if(model_len != fread(model, 1, model_len, fp)) 
	{
		free(model);
		fclose(fp);
		return false;
	}
	fclose(fp);

	//init model
	int ret = rknn_init(&m_mapAlgModel[strAlgMapKey],model,model_len,RKNN_FLAG_PRIOR_MEDIUM);
	if(ret < 0) 
	{
		LOG_INFO("yolo_alg") << "rknn_init fail\n";
		free(model);
		return false;
    }
	LOG_INFO("yolo_alg") << string_format("rknn_init successful ret : %d and m_tx : %d\n",ret,m_mapAlgModel[strAlgMapKey]);

	//rknn query
	rknn_tensor_attr outputs_attr[output_size];
	outputs_attr[0].index = 0;
	ret = rknn_query(m_mapAlgModel[strAlgMapKey], RKNN_QUERY_OUTPUT_ATTR, &(outputs_attr[0]), sizeof(outputs_attr[0]));
	if (ret < 0)
	{
		LOG_ERROR("yolo_alg") << string_format("rknn_query fail! ret=%d\n", ret);
		free(model);
		return false;
	}

	return true;
}

void CDetectAlg::PreprocImg(const cv::Mat& srcImg,cv::Mat& dstImg)
{
	int s32Scale = srcImg.cols / dstImg.cols;
	int s32SrcMidCols = srcImg.cols / s32Scale;
	int s32SrcMidRows = srcImg.rows / s32Scale;
	int dh = (dstImg.rows - s32SrcMidRows) / 2;
	if(dstImg.cols != s32SrcMidCols || 0 > dh)
	{
		return;
	}

	unsigned char* ptrDataDst = dstImg.data + dh * dstImg.cols * img_channels;
	for(int i = dh;i < dstImg.rows - dh;i++)
	{
		for(int j = 0;j < dstImg.cols;j++)
		{
			memcpy(ptrDataDst,srcImg.data + ((i - dh) * srcImg.cols + j) * s32Scale * img_channels,img_channels);
			ptrDataDst += img_channels;
		}
	}
}

void CDetectAlg::ProcVideoMat(const string& strIp,const string& strCamCode,const cv::Mat& srcImg)
{	
	//img resize
	cv::Mat dstImg(net_width, net_height, CV_8UC3, Scalar(113, 113, 113));
	PreprocImg(srcImg,dstImg);

	//def input
	rknn_input inputs[input_size];
	inputs[0].index = 0;
	inputs[0].size = net_width * net_height * img_channels;
	inputs[0].pass_through = false;         
	inputs[0].type = RKNN_TENSOR_UINT8;
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	inputs[0].buf = dstImg.data;

	for(auto val : m_mapAlgModel)
	{
		//query
		rknn_tensor_attr outputs_attr[output_size];
		outputs_attr[0].index = 0;
		int ret = rknn_query(val.second, RKNN_QUERY_OUTPUT_ATTR, &(outputs_attr[0]), sizeof(outputs_attr[0]));
		if (ret < 0)
		{
			LOG_ERROR("yolo_alg") << string_format("rknn_query fail! ret=%d\n", ret);
			return;
		}

		//img input
		ret = rknn_inputs_set(val.second, 1, inputs);
		if (ret < 0) 
		{
			LOG_ERROR("yolo_alg") << string_format("rknn_inputs_set fail! ret=%d\n", ret);
			return;
		}
		
		//rknn run
		ret = rknn_run(val.second, nullptr);
		if (ret < 0) 
		{
			LOG_ERROR("yolo_alg") << string_format("rknn_run fail! ret=%d\n", ret);
			return;
		}

		//rknn outputs
		rknn_output outputs[output_size];
		for(auto& out : outputs)
		{
			out.want_float = true;
			out.is_prealloc = false;
		}
		ret = rknn_outputs_get(val.second, output_size, outputs, NULL);  //4表示输出的个数
		if (ret < 0)
		{
			LOG_ERROR("yolo_alg") << string_format("rknn_outputs_get fail! ret=%d\n", ret);
			return;
		}

		//result transfor
		detection* dets = nullptr;
		dets = (detection*) calloc(nboxes_total,sizeof(detection));
		for(int i = 0; i < nboxes_total; ++i)
		{
			dets[i].prob = (float*) calloc(s32Classes,sizeof(float));
		}
		
		int nboxes_left = outputs_transform(outputs, net_width, net_height,s32Classes,OBJ_THRESH,NMS_THRESH,dets);
		if(0 < nboxes_left)
		{
			for(auto& val : m_mapResultReportTopic)
			{
				m_ptrMsgBus->SendReq<void,const std::string&,const cv::Mat&,const detection*,const int&,const int&,const float&>(strIp,srcImg,dets,nboxes_left,s32Classes,DRAW_CLASS_THRESH,val.first);
			}
		}

		//release resource
		rknn_outputs_release(val.second, output_size,outputs);
		free_detections(dets,nboxes_total);
	}
}

void CDetectAlg::Release()
{
	for(auto& val : m_mapAlgModel)
	{
		rknn_destroy(val.second);
	}
}