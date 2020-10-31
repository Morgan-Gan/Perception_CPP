#include <vector>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "caffe/caffe.hpp"
#include "caffe/layers/face_data_layer.hpp"
#include "caffe/common.hpp"
#include "face_identification.hpp"
#include "caffeFwProc.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "tracelog.h"
#include "dog.h"
#include "model.hpp"
#include <glog/logging.h>
#define WAVE_ENCRYPT
#ifdef WAVE_ENCRYPT
#endif

typedef unsigned char byte;
bool WaveID::g_bDogOpened = false;
char g_logfilename[512];

int WaveID::Initial(const char* modeldir, sem_t *pSem, int gpu_id){
	gpu_id_ = gpu_id;
	if (gpu_id_ < 0){
		caffe::Caffe::set_mode(caffe::Caffe::CPU);
	}else{
		caffe::Caffe::set_mode(caffe::Caffe::GPU);
		caffe::Caffe::SetDevice(gpu_id_);
	}
	memset(g_logfilename, 0, 512);
	memcpy(g_logfilename, modeldir, strlen(modeldir));
	strcat(g_logfilename, "/FitLog.log");
	tracelog(g_logfilename, LOG_INFO, "Version 2.1.0");
	//加密狗初始化
	#ifdef WAVE_ENCRYPT
	tracelog(g_logfilename, LOG_INFO, "checking encrypt dog...");
	pSem_ = pSem;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 100000000;  //100ms
    int s = sem_timedwait(pSem_, &ts);

	if(s == 0){
		if(!g_bDogOpened){
			//search dog
			if(DogManagerSingleton::InitDog() != 0 ){
			    sem_post(pSem_);
			    return -1;
			}
			g_bDogOpened = true;  //only open dog one time
		}
		sem_post(pSem_);
	}
	#endif
   	if (face_net1_ != nullptr || face_net2_ != nullptr){
		tracelog(g_logfilename, LOG_INFO, "facenet initial failed...");
		return -4;
	}
	//模型解密
	tracelog(g_logfilename, LOG_INFO, "decrypt model...");
	char en_modelpath[512];
	memset(en_modelpath, 0, 512);
	memcpy(en_modelpath, modeldir, strlen(modeldir));
	strcat(en_modelpath, "/face_identification_linux64_v2.1.1-1.bin");
	int suc = ForwardInit(&face_net1_, MODEL_1, en_modelpath, gpu_id_);
	if(suc || face_net1_ == nullptr) {
		tracelog(g_logfilename, LOG_INFO, "face_net1 load failed...");
		return -4;
	}
	memset(en_modelpath, 0, 512);
	memcpy(en_modelpath, modeldir, strlen(modeldir));
	strcat(en_modelpath, "/face_identification_linux64_v2.1.1-2.bin");
	suc = ForwardInit(&face_net2_, MODEL_2, en_modelpath, gpu_id_);
	if(suc || face_net2_ == nullptr) {
		tracelog(g_logfilename, LOG_INFO, "face_net2 load failed...");
		return -4;
	}
	caffe::Blob<float> *input = ((caffe::Net<float> *)face_net1_)->blob_by_name("data").get();
	img_h_ = input->height();
	img_w_ = input->width();
	feature_dim1_ = ((caffe::Net<float> *) face_net1_)->blob_by_name("embedding/normalize")->shape(1);
	feature_dim2_ = ((caffe::Net<float> *) face_net2_)->blob_by_name("embedding/normalize")->shape(1);
	feature_dim_ =  feature_dim1_ +feature_dim2_;
	return 0;
}

int WaveID::SetModeCPU(){
  caffe::Caffe::set_mode(caffe::Caffe::CPU);
  return 0;
}

int WaveID::SetModeGPU(int gpu_id){
  if (gpu_id_ < 0){
    return -1;
  }
  gpu_id_ = gpu_id;
  caffe::Caffe::set_mode(caffe::Caffe::GPU);
  caffe::Caffe::SetDevice(gpu_id_);
  return 0;
}

int WaveID::Release(){
#ifdef WAVE_ENCRYPT
	tracelog(g_logfilename, LOG_INFO, "close the dog...");
	DogManagerSingleton::CloseDog();
	g_bDogOpened = false;
#endif
	if (face_net1_ != nullptr){
		delete ((caffe::Net<float> *) face_net1_);
		face_net1_ = nullptr;
	}
	if(face_net2_ != nullptr)
	{
		delete ((caffe::Net<float> *) face_net2_);
		face_net2_ = nullptr;
	}
	usleep(50000);
	return 0;
}

//extract feature
int WaveID::ExtractFeature(const cv::Mat img, const cv::Rect facerect,const cv::Mat landmark, float* feature){
  if (gpu_id_ >= 0){
    caffe::Caffe::set_mode(caffe::Caffe::GPU);
    caffe::Caffe::SetDevice(gpu_id_);

  }
	WV_FACE_ParamIn * paramInput = NULL;
	if (img.data == NULL || img.rows <= 0|| img.cols <= 0 || img.channels() != 3
	|| landmark.data == NULL || landmark.cols*landmark.rows != 136 || feature == NULL) {
		return NP_ERROR_PARAM;
	}
	StructOutParam  OutParam;
	memset(&OutParam, 0, sizeof(StructOutParam));
	WV_FACE_ParamIn  parami;
	memset(&parami, 0, sizeof(WV_FACE_ParamIn));
	parami.nImg.height = img.rows;
	parami.nImg.width = img.cols;
	parami.nImg.nWidthStep = img.step[0];
	parami.nImg.nChannel = img.channels();
	parami.nImg.nType = WV_BGR;
	parami.nImg.pData = (unsigned char*)img.data;
	float* pData = (float*)landmark.data;
	parami.nFacePoint = new WV_FacePoints();
	parami.nFacePoint->pFaceRect.left = facerect.x;
	parami.nFacePoint->pFaceRect.top = facerect.y;
	parami.nFacePoint->pFaceRect.right = facerect.x + facerect.width -1;
	parami.nFacePoint->pFaceRect.bottom = facerect.y + facerect.height -1;
	parami.nFacePoint->nFacePointNum = 68;
	for(int i =0 ;i < 68; i++) {
		parami.nFacePoint->pPoints[i].x = pData[2*i];
		parami.nFacePoint->pPoints[i].y = pData[2*i+1];
	}

	int ret = 0;
	ret = ForwardProcess((caffe::Net<float> *)face_net1_, (caffe::Net<float> *)face_net2_, &parami, &OutParam);
	if(ret || OutParam.pdata== NULL || OutParam.pdata1 == NULL || OutParam.dim != feature_dim1_ || OutParam.dim1 != feature_dim2_
		|| OutParam.dim + OutParam.dim1 != feature_dim_) {
		delete parami.nFacePoint;
		parami.nFacePoint  = nullptr;
		return NP_ERROE_CALL;
	}
	//printf("************* feature detect success\n");
	delete parami.nFacePoint;
	parami.nFacePoint  = nullptr;
	unsigned char*  features = (unsigned char*)feature;
	memcpy(features, OutParam.pdata, sizeof(float)*OutParam.dim);
	memcpy(features + sizeof(float)*OutParam.dim, OutParam.pdata1, sizeof(float)*OutParam.dim1);
	return NP_ERROR_NONE;
}

//compute the score
double WaveID::FaceVerify(const float* feature1, const float* feature2){
	double simi1 = 0;
	const float* pData1 = feature1;
	const float* pData2 = feature2;
	for (int i = 0; i < feature_dim1_; ++i){
		simi1 += (*pData1) * (*pData2);
		pData1++;
		pData2++;
	}
	simi1 = (simi1 + 1.0) / 2.0 + 0.03;
	double simi2 = 0;
	for (int i = feature_dim1_; i < feature_dim_; ++i){
		simi2 += (*pData1) * (*pData2);
		pData1++;
		pData2++;
	}
	simi2 = (simi2 + 1.0) / 2.0 ;
	double simi = 0.0;
	double thr = 0.5;
	simi = cv::min(thr*simi1 + (1-thr)*simi2+ 0.02,1.0);
	return simi;
}
