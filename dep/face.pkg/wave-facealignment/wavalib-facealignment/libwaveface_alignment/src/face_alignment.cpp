#include "face_alignment.h"
#include "opencv2/opencv.hpp"
#include <glog/logging.h>
#include "boost/algorithm/string.hpp"
#include "caffe/caffe.hpp"
#include "caffeFwProc_shiyuan.hpp"
#include <sys/stat.h>  
#include <cstring>
#include <map>
#include <string>
#include <vector>
using namespace cv;
using namespace caffe;  // NOLINT(build/namespaces)
using std::max;
using std::min;
#include "GpuAlloc.h"
using namespace cv;
#define  LANDMARK 68
#define  IMGSIZE  112

float max_min[] = { 27.07073828, 29.18361271, 85.53304006, 88.44473259 };
extern char landmark_net[];
float meanshape[] = { 27.07073828, 40.82285702, 27.23700584, 48.69697332, 28.13616493
, 56.61469355, 29.71880067, 64.42449433, 32.53966357, 71.62860404
, 36.94256555, 77.96991301, 42.54833787, 83.27223551, 49.0898234
, 87.27404899, 56.55019928, 88.44473259, 63.95435631, 87.18051466
, 70.46482926, 83.15999686, 76.05522789, 77.84549007, 80.37374406
, 71.38877838, 83.12297152, 64.04028684, 84.65294974, 56.22152822
, 85.42889203, 48.24360896, 85.53304006, 40.29287418, 31.97296755
, 34.12401035, 35.52475334, 30.59829692, 40.68888172, 29.50112247
, 46.03643371, 30.26772612, 50.91698125, 32.2223827, 60.87000138
, 31.92663397, 65.94151562, 29.89449003, 71.33842456, 29.18361271
, 76.53186259, 30.33783098, 80.06956662, 33.87597444, 55.95697748
, 39.00673694, 55.95489265, 44.07291718, 55.95384538, 49.00515766
, 55.97424838, 54.12163136, 49.74302324, 58.53163751, 52.77265678
, 59.53911397, 56.06082163, 60.27281704, 59.32781592, 59.48990353
, 62.32249866, 58.5382276, 37.53495382, 40.52156226, 40.81390227
, 38.80547415, 44.63455685, 38.8332459, 48.07630179, 40.96052256
, 44.54131404, 41.80399039, 40.73827957, 41.84358277, 63.80350807
, 40.8324307, 67.28417686, 38.5902594, 71.1085882, 38.57729696
, 74.39712714, 40.21324876, 71.27499455, 41.51062622, 67.53309373
, 41.6027777, 45.20366465, 69.13995397, 49.14968056, 66.84226068
, 53.22458641, 65.61720842, 56.09967159, 66.39386474, 59.12968227
, 65.61371934, 63.29210515, 66.8192887, 67.20418052, 69.01108312
, 63.44313283, 72.53049169, 59.50923752, 74.20910401, 56.15705774
, 74.55059435, 52.95575727, 74.25455012, 49.0368352, 72.60384779
, 46.88947732, 69.18414025, 53.16874556, 68.55473865, 56.14203285
, 68.83368789, 59.25473132, 68.51232491, 65.52511074, 69.08928986
, 59.28956968, 70.18609756, 56.100913, 70.55705049, 53.1093319
, 70.23984673 };

unsigned long get_file_size(const char *path) {  
  unsigned long filesize = -1;      
  struct stat statbuff;  
  if(stat(path, &statbuff) < 0){  
    return filesize;  
  } else {  
    filesize = statbuff.st_size;  
  }  
  return filesize;  
} 

void EncodeModel(const char* modelfile) {
  char filename[512];
  memset(filename,0,512);
  sprintf(filename, "model.cpp");
  FILE* pfOut = fopen(filename, "w+");
  fprintf(pfOut, "char landmark_net[] = {\n");
  FILE* pfIn = fopen(modelfile, "rb");
  int ret = 0;
  while (!feof(pfIn)) {
		char c ;
		ret = fread(&c, 1, 1, pfIn);
		if (ret<=0) {
			continue;
		}
		c = ~c;
		fprintf(pfOut,"%d, ", c);
	}
	fprintf(pfOut,"\n0 };");
	fclose(pfIn);
	fclose(pfOut);
}

void DecodeModel(const char* src, char* dst) {
	int len = strlen(src);
	for (int i = 0; i < len; i++){
		dst[i] = ~src[i];
	}
}

bool IsWeightsNeedDecode(unsigned char* ptr) {
	char buf[50];
	memcpy(buf, ptr + 2, 7);
	buf[7] = '\0';
	if (strcmp(buf, "AlexNet") == 0||strcmp(buf,"FaceNet")==0||strcmp(buf,"Deep-al")==0)
		return false;
	return true;
}

void DecodeWeights(unsigned char* ptr, int len) {
	int nPair = len / 2;
	for (int i = 0; i < nPair; i++){
		unsigned char tmp;
		tmp = ~ptr[i];
		ptr[i] = ~ptr[len - 1 - i];
		ptr[len - 1 - i] = tmp;
	}
	if (len > 2 * nPair)
		ptr[nPair] = ~ptr[nPair];
}

int AffineLandmark1(float* Landmark, const Mat& R, const Mat& C, float* LandmarkOut, bool isInv = false) {
	if (Landmark == NULL || LandmarkOut == NULL) {
		return -1;
	}
	Mat A, T;
	if (isInv == false) {
		A = R.inv();
		T.create(1, 2, CV_32FC1);
		float *pData = (float*)T.data;
		float *pData1 = (float*)C.data;
		float* pData2 = (float*)A.data;
		pData[0] = (pData1[0] * pData2[0] + pData1[1] * pData2[2])*(-1.0);
		pData[1] = (pData1[0] * pData2[1] + pData1[1] * pData2[3])*(-1.0);
	}	else{
		R.copyTo(A);
		C.copyTo(T);
	}

	float* a = (float*)A.data;
	float* b = (float*)T.data;
	for (int i = 0; i < LANDMARK; i++) {
    LandmarkOut[2*i] = Landmark[2*i] * a[0] + Landmark[2*i+1] * a[2] + b[0];
    LandmarkOut[2*i+1] = Landmark[2*i] * a[1] + Landmark[2*i+1] * a[3] + b[1];
	}
	return 0;
}

int MinMaxCoordinate(float* landmarks,float* rect){
  if(landmarks==NULL || rect==NULL)
    return -1;
  float x_min = 100000,y_min = 100000,x_max = 0,y_max = 0;
  for (int i = 0; i < LANDMARK; i++) {
			if (landmarks[2*i]<x_min)	{
				x_min = landmarks[2*i];
			}
			if (landmarks[2*i] > x_max)	{
				x_max = landmarks[2*i];
			}
			if (landmarks[2*i+1]<y_min)	{
				y_min = landmarks[2*i+1];
			}
			if (landmarks[2*i+1] > y_max) {
				y_max = landmarks[2*i+1];
			}
		}
		rect[0] = x_min;
		rect[1] = y_min;
		rect[2] = x_max;
		rect[3] = y_max;
  return 0;
}

int bestFitRect(float* landmarks, float* meanShape, float*box, float* S0) {
	if ((landmarks == NULL&&box == NULL) || meanShape == NULL || S0 == NULL) {
		return -1;
	}
  // step 1 landmarks's max min and center
	float rect[4];
	if (box == NULL) {
    MinMaxCoordinate(landmarks,rect);
	}	else{
		memcpy(rect, box, sizeof(float)* 4);
	}
	float boxCenter_x, boxCenter_y, boxWidth, boxHeight;
	boxCenter_x = (rect[0] + rect[2]) / 2;
	boxCenter_y = (rect[1] + rect[3]) / 2;
	boxWidth = rect[2] - rect[0];
	boxHeight = rect[3] - rect[1];
  //step 2 meanShape's max min and center
	memset(rect,0,sizeof(float)*4);
  MinMaxCoordinate(meanShape,rect);
	float meanShapeWidth = rect[2] - rect[0];
	float meanShapeHeight = rect[3] - rect[1];
	float scaleWidth = boxWidth / meanShapeWidth;
	float	scaleHeight = boxHeight / meanShapeHeight;
	float scale = (scaleWidth + scaleHeight) / 2;
	int len = LANDMARK * 2;
	for (int i = 0; i < len; i++)	{
		S0[i] = meanShape[i]*scale;
	}
  //step 4 S0's max min and center
	memset(rect,0,sizeof(float)*4);
  MinMaxCoordinate(S0,rect);
	float S0Center_x = (rect[0] + rect[2]) / 2;
	float S0Center_y = (rect[1] + rect[3]) / 2;
	float deta_x = boxCenter_x - S0Center_x;
	float deta_y = boxCenter_y - S0Center_y;
  //step 5 new S0
	for (int i = 0; i < LANDMARK; i++) {
		S0[2*i] += deta_x;
		S0[2*i+1] += deta_y;
	}
	return 0;
}

int GetAffineParam1(float*ShapesFrom, float* ShapeTo, Mat& T, Mat& C) {
	if (ShapesFrom == NULL || ShapeTo == NULL) {
		return -1;
	}
  //step mean
	float dest_mean_x = 0,dest_mean_y = 0,src_mean_x = 0,src_mean_y = 0;
	float* pData;
  float* pData1;;
	for (int i = 0; i < LANDMARK; i++) {
		dest_mean_x += ShapeTo[2*i];
		dest_mean_y += ShapeTo[2*i+1];
    src_mean_x += ShapesFrom[2*i];
		src_mean_y += ShapesFrom[2*i+1];
	}
	dest_mean_x /= LANDMARK;
	dest_mean_y /= LANDMARK;
	src_mean_x /= LANDMARK;
	src_mean_y /= LANDMARK;
  //step 2 srcVec destVec
	int len = LANDMARK * 2;
	float *srcVec = new float[len];
	float *destVec = new float[len];
	for (int i = 0; i < LANDMARK; i++) {
		srcVec[2*i] = ShapesFrom[2*i] - src_mean_x;
		srcVec[2*i+1] = ShapesFrom[2*i+1] - src_mean_y;
    destVec[2*i] = ShapeTo[2*i] - dest_mean_x;
		destVec[2*i+1] = ShapeTo[2*i+1]- dest_mean_y;;
	}
  //step 3 a and b
	float sum1 = 0;
	float sum2 = 0.0;
	pData = srcVec;
	pData1 = destVec;
	for (int i = 0; i < len; i++)	{
		sum1 += srcVec[i]*destVec[i];
		sum2 += srcVec[i]*srcVec[i];
	}
	float a = sum1 / sum2;
	float b = 0;
	for (int i = 0; i < LANDMARK; i++) {
		b += srcVec[2*i] * destVec[2*i+1] - srcVec[2*i+1] * destVec[2*i];
	}
	b = b / sum2;
	T.create(2, 2, CV_32FC1);
	pData = (float*)T.data;
	pData[0] = a;
	pData[1] = b;
	pData[2] = -b;
	pData[3] = a;
	float c1 = src_mean_x*a - b*src_mean_y;
	float c2 = b*src_mean_x + a*src_mean_y;
	C.create(1, 2, CV_32FC1);
	pData = (float*)C.data;
	pData[0] = dest_mean_x - c1;
	pData[1] = dest_mean_y - c2;
	delete[] srcVec;
	delete[] destVec;
	return 0;
}

int RandomSRT(const Mat color, float*landmark, float*MeanShape, Mat& ImgVec, float* Land) {
	if (MeanShape == NULL || landmark == NULL || color.data == NULL || Land == NULL) {
		return -1;
	}
  //step 1 R and T
	float* S0 = new float[LANDMARK * 2];
	int ret = bestFitRect(MeanShape, landmark, NULL, S0);
	if (ret) {
		delete[] S0;
		return ret;
	}
	Mat R, T;
	ret = GetAffineParam1(landmark, S0, R, T);
	if (ret) {
		delete[] S0;
		return ret;
	}
  //step 2 R2 and T2
	Mat R2 = R.inv();
	Mat T2;
	T2.create(1, 2, CV_32FC1);
	float* pData = (float*)T2.data;
	float* pData1 = (float*)T.data;
	float* pData2 = (float*)R2.data;
	pData[0] = (pData1[0] * pData2[0] + pData1[1] * pData2[2])*(-1.0);
	pData[1] = (pData1[0] * pData2[1] + pData1[1] * pData2[3])*(-1.0);

	ImgVec.create(IMGSIZE, IMGSIZE, CV_8UC3);
	unsigned char* ImageDst = (unsigned char*)ImgVec.data;
	unsigned char* ImageSrc = (unsigned char*)color.data;
	pData2 = (float*)R2.data;
	pData1 = (float*)T2.data;
	for (int i = 0; i < IMGSIZE; i++)	{
		for (int j = 0; j < IMGSIZE; j++)	{
			float x1 = j * pData2[0] + i * pData2[2] + pData1[0];
			float y1 = j * pData2[1] + i * pData2[3] + pData1[1];
			int x = x1;
			int y = y1;
			if (x < 1 || x >= color.cols - 1 || y < 1 || y >= color.rows - 1)	{
				ImageDst[i*ImgVec.step[0] + 3 * j] = 0;
				ImageDst[i*ImgVec.step[0] + 3 * j + 1] = 0;
				ImageDst[i*ImgVec.step[0] + 3 * j + 2] = 0;
			}	else {
				int channel = 3;
				unsigned char* p = ImageSrc + y*color.step[0];
				unsigned char* p1 = ImageSrc + (y + 1)*color.step[0];
				float f1x = p[x * channel] * (1 - (x1 - x)) + p[(x + 1) * channel] * (x1 - x);
				float f1y = p[x * channel + 1] * (1 - (x1 - x)) + p[(x + 1) * channel + 1] * (x1 - x);
				float f1z = p[x * channel + 2] * (1 - (x1 - x)) + p[(x + 1) * channel + 2] * (x1 - x);
				float f2x = p1[x * channel] * (1 - (x1 - x)) + p1[(x + 1) * channel] * (x1 - x);
				float f2y = p1[x * channel + 1] * (1 - (x1 - x)) + p1[(x + 1) * channel + 1] * (x1 - x);
				float f2z = p1[x * channel + 2] * (1 - (x1 - x)) + p1[(x + 1) * channel + 2] * (x1 - x);
				ImageDst[i*ImgVec.step[0] + 3 * j] = f1x*(1 - (y1 - y)) + f2x*((y1 - y));
				ImageDst[i*ImgVec.step[0] + 3 * j + 1] = f1y*(1 - (y1 - y)) + f2y*((y1 - y));
				ImageDst[i*ImgVec.step[0] + 3 * j + 2] = f1z*(1 - (y1 - y)) + f2z*((y1 - y));
			}

		}
	}
	memcpy(Land, S0, sizeof(float)*LANDMARK * 2);
	delete[] S0;
	return 0;
}


int FaceAlignment::Initial(const char* modelpath, int gpu_id) {       
	if (modelpath == NULL) {
		return -1;
	}
  // read model path
	char nWeightsPath[1024];
	memset(nWeightsPath, 0, 1024);
	sprintf(nWeightsPath, "%s/face_alignment_centos64_v0.1.1.wave", modelpath);
  gpu_id_ = gpu_id;
	const char *FLAGS_model = landmark_net;
	const char *FLAGS_weights = nWeightsPath;
	char *FLAGS_iterations = "1";
	CHECK_GT(strlen(FLAGS_model), 0) << "Need a model definition to score.";
	CHECK_GT(strlen(FLAGS_weights), 0) << "Need model weights to score.";
	if (gpu_id >= 0) {
	  Caffe::SetDevice(gpu_id);
	  Caffe::set_mode(Caffe::GPU);
	}	else {
	  Caffe::set_mode(Caffe::CPU);
	}
  // Decode Model
	int Len = strlen(FLAGS_model);
	Net<float> *caffe_net = NULL;
	if (Len < 1000){ 
		caffe_net = new Net<float>(FLAGS_model, caffe::TEST);
		EncodeModel(FLAGS_model);
	}	else {
		NetParameter param;
		char* buf = new char[Len + 1];
		DecodeModel(FLAGS_model, buf);
		buf[Len] = '\0';
		bool success = ReadProtoFromText(buf, &param);
		if (!success)	{
			delete[] buf;
			return -1;
		}
		param.mutable_state()->set_phase(TEST);
		caffe_net = new Net<float>(param);
		delete[] buf;
	}
  // Decode Weights
	unsigned long fileSize = get_file_size(FLAGS_weights);
	unsigned char* weights = new unsigned char[fileSize];
	FILE *pf = fopen(FLAGS_weights, "rb");
	fread(weights, 1, fileSize, pf);
	fclose(pf);
	if (IsWeightsNeedDecode(weights)) {
		DecodeWeights(weights, fileSize);
	}	else {
		DecodeWeights(weights, fileSize);
		char filename[512];
		memset(filename,0,512);
		sprintf(filename, "%s.bin", FLAGS_weights);
		FILE* pfOut = fopen(filename, "wb+");
		fwrite(weights, 1, fileSize, pfOut);
		fclose(pfOut);
		DecodeWeights(weights, fileSize);
	}
  // create network
	NetParameter trained_net_param;
	CHECK(ReadProtoFromBinaryMemory(weights, fileSize, &trained_net_param)) << "Failed to parse NetParameter file";
	delete[] weights;
	caffe_net->CopyTrainedLayersFrom(trained_net_param);
	face_alignment_ = caffe_net;
	if (face_alignment_ == NULL) {
		int nRet = Release();
	  return -1;
	}
	return 0;
}

int FaceAlignment::Alignment(const  cv::Mat &img,
		const  cv::Rect &bbox, cv::Mat* landmarks) {
	if(gpu_id_>=0) {
	  Caffe::set_mode(Caffe::GPU);
    Caffe::SetDevice(gpu_id_);  
	}
	if (img.data==NULL || img.cols<=0 || img.rows<=0 || img.channels() !=3)	{
		return -1;
	}

	int h_w = min(bbox.height, bbox.width);
	float scale = h_w / (float)((max_min[2] - max_min[0]));
	float* initlandmarks = new float[LANDMARK * 2];
	float* lands = new float[LANDMARK * 2];
	for (int i = 0; i < LANDMARK; i++)	{
		initlandmarks[2*i] = (meanshape[2*i]  - max_min[0])*scale + bbox.x;
		initlandmarks[2*i+1] = (meanshape[2*i+1] - max_min[1])*scale + bbox.y;
	}
  // step 1
	Mat image;
	int ret = RandomSRT(img, initlandmarks, meanshape, image, lands);
	if (ret) {
		delete[] lands;
		delete[] initlandmarks;
		return ret;	
	}
  // step 2
	Mat R, A ;
	ret = GetAffineParam1(initlandmarks, lands, R, A);
	if (ret) {
		delete[] lands;
		delete[] initlandmarks;
		return ret;
	}
  // step 3
	float* pFeature1 = NULL;
	float* pFeature2 = NULL;
	int  pLen1 = 0;
	int  pLen2 = 0;
	ret = caffeALLForwardProcess1(face_alignment_,(char*) image.data, image.rows, image.cols, &pFeature1, pLen1, &pFeature2, pLen2);	
	if (ret|| pFeature1== NULL|| pFeature2 == NULL || pLen1 != pLen2 || pLen1 != LANDMARK * 2) {
		delete[] lands;
		delete[] initlandmarks;
		return ret;
	}
	//step 4
  Mat T, C;
	for (int i = 0; i < pLen1; i++)	{
		pFeature1[i] += meanshape[i];
	}
	GetAffineParam1(pFeature1, meanshape, T, C);
	float* outLandmarks = new float[pLen1];
	AffineLandmark1(pFeature1, T, C, outLandmarks, true);
	for (int i = 0; i < pLen2; i++)	{
		pFeature2[i] += outLandmarks[i];	
	}
  //step 5
	AffineLandmark1(pFeature2, T, C, outLandmarks, false);
	AffineLandmark1(outLandmarks, R, A, initlandmarks, false);
	*landmarks = cv::Mat(1, 136, CV_32FC1);
	memcpy(landmarks->data, initlandmarks, sizeof(float)*pLen2);
	delete[] lands;
	delete[] initlandmarks;
	delete[] outLandmarks;
	return 0;
}

int FaceAlignment::GetMeanShape(cv::Mat* mean_landmarks) {
	*mean_landmarks = cv::Mat(1, 136, CV_32FC1);
	memcpy(mean_landmarks->data, meanshape, sizeof(float)*LANDMARK * 2);
	return 0;
}

int FaceAlignment::Release() {
	if (NULL != face_alignment_) {		
		delete (Net<float>*)face_alignment_;
    face_alignment_ = NULL;
 		usleep(50000);
	}   	
  return 0;
}
const int FaceAlignment::GetDeviceID(){
  return gpu_id_;
}
const int FaceAlignment::GetNeedMemory(){
  return 120;
}
