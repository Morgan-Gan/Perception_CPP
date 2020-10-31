#include "face_identification.hpp"
#include<stdio.h>
#include<sys/time.h>
#include <unistd.h>
#include <opencv2/core/core.hpp>    
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <semaphore.h>
#include <chrono>
#include <iostream>
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;




int main(int argc, char** argv)
{
	argv[1]= "../models";
	argv[2]= "../data/00000_00469_00035.jpg";
	argv[3]= "../data/00000_00469_00050.jpg";
	static sem_t sem ;
	printf("start\n");
	WaveID obj;
	obj.Initial(argv[1],&sem,6);
	WaveID obj_1;
	obj_1.Initial(argv[1],&sem,6);
	
	cv::Rect faceRect;
	faceRect.x = 0;
	faceRect.y = 0;
	faceRect.width = 0;
	faceRect.height = 0;
	cv::Mat img = cv::imread(argv[2]);
	cv::Mat img1 = cv::imread(argv[3]);
	//cv::imshow("src",img);
	cv::Mat landmarks;
	landmarks.create(1,136,CV_32FC1);
	int feature_dim = obj.GetFeatureDim();
	printf("feature dim:%d\n",feature_dim);
	float* features = new float[feature_dim];
	float* features1 = new float[feature_dim];
	clock_t start = clock();
       for(int i=0;i<1000;i++){
	obj.ExtractFeature(img,faceRect,landmarks,features);
	obj_1.ExtractFeature(img1,faceRect,landmarks,features1);
//	double fscore = obj.FaceVerify(features,features1);
//	printf("vertify score:%f\n",fscore);
	}
	clock_t end   = clock();
        std::cout << "1000 times spend" << 1000.0*(double)(end - start) / CLOCKS_PER_SEC << "ms" << std::endl;
               // obj1.ExtractFeature(img,faceRect,landmarks,features);
		
	printf("end\n");
	obj.Release();
//obj1.Release();	
	return 0;
}
