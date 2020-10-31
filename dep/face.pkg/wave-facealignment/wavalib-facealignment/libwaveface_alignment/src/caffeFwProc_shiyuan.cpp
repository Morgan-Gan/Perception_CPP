// Copyright 2014 BVLC and contributors.
//
// This is a simple script that allows one to quickly test a network whose
// structure is specified by text format protocol buffers, and whose parameter
// are loaded from a pre-trained network.
// Usage:
//    test_net net_proto pretrained_net_proto iterations [CPU/GPU]

//#include "caffeFwProc.h"
#include "face_alignment.h"
#include "caffeFwProc_shiyuan.hpp"
#include <sys/stat.h>  
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
#include <glog/logging.h>
#include "boost/algorithm/string.hpp"

//#include "commctrl.h" 
//#pragma comment(lib, "comctl32.lib")
using namespace cv;
using namespace caffe;  // NOLINT(build/namespaces)
using std::max;
using std::min;
void MatTransform1(const cv::Mat cv_img,Blob<float>* transformed_blob) {
	const int img_channels = cv_img.channels();
	const int img_height = cv_img.rows;
	const int img_width = cv_img.cols;
	const int channels = transformed_blob->channels();
	const int height = transformed_blob->height();
	const int width = transformed_blob->width();
	const int num = transformed_blob->num();
	CHECK_EQ(channels, img_channels);
	CHECK_LE(height, img_height);
	CHECK_LE(width, img_width);
	CHECK_GE(num, 1);
	CHECK(cv_img.depth() == CV_8U) << "Image data type must be unsigned byte";
	float* transformed_data = transformed_blob->mutable_cpu_data();
	int top_index;

	for (int h = 0; h < img_height; ++h) {
		const uchar* ptr = cv_img.ptr<uchar>(h);
		int img_index = 0;
		for (int w = 0; w < img_width; ++w) {
			for (int c = 0; c < img_channels; ++c) {
				top_index = (c * img_height + h) * img_width + w;
				float pixel;
				pixel = (static_cast<float>(ptr[img_index++]) - 127.5)*0.0078125;
				transformed_data[top_index] = pixel;	
			}
		}
	}
}

void MatArrayTransform1(const vector<cv::Mat> & mat_vector,Blob<float>* transformed_blob) {
	const int mat_num = mat_vector.size();
	const int num = transformed_blob->num();
	const int channels = transformed_blob->channels();
	const int height = transformed_blob->height();
	const int width = transformed_blob->width();
	CHECK_GT(mat_num, 0) << "There is no MAT to add";
	CHECK_EQ(mat_num, num) <<
		"The size of mat_vector must be equals to transformed_blob->num()";
	Blob<float> uni_blob(1, channels, height, width);
	for (int item_id = 0; item_id < mat_num; ++item_id) {
		int offset = transformed_blob->offset(item_id);
		uni_blob.set_cpu_data(transformed_blob->mutable_cpu_data() + offset);
		MatTransform1(mat_vector[item_id], &uni_blob);
	}
}

int caffeALLForwardProcess1(void *net, char* rgb, int height, int width, float**pFeature1, int& pfeaturelen1, float**pFeature2, int& pfeaturelen2) {
	try
	{
		if (net == NULL || rgb == NULL || height <= 0 || width <= 0) {
			return -1;
		}
		Net<float> * caffe_net =(Net<float>*) net;
		//StructNSBDHandle * procHandel = (StructNSBDHandle*)handle;
		cv::Mat  cv_img_origin(cv::Size(width, height), CV_8UC3, rgb, 3*width);
		
		Blob<float>* input = caffe_net->blob_by_name("data").get();;
		int rwidth = input->width();
		int rheight = input->height();
		int rchannels = input->channels();
		Size dsize = Size(rwidth, rheight);
		Mat resizeMat;
		resize(cv_img_origin, resizeMat, dsize);
		Mat gray;
		cvtColor(resizeMat, gray, COLOR_BGR2GRAY);
		vector<Mat> matarray;
		matarray.push_back(gray);	
		MatArrayTransform1(matarray, input);
		matarray.clear();
		caffe_net->Forward();
		Blob<float>* features1 = caffe_net->blob_by_name("fc_2").get();//
		*pFeature1 = features1->mutable_cpu_data(); 
		pfeaturelen1 = features1->count();
		Blob<float>* features2 = caffe_net->blob_by_name("fc_2_3_1").get();//
		*pFeature2 = features2->mutable_cpu_data();
		pfeaturelen2 = features2->count();
		return 0;
	}
	catch (...)
	{
		
		return -1;
	}
	return 0;
}
