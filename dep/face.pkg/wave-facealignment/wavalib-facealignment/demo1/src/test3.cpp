#include "LibWaveFaceAlignment.hpp"
#include <opencv2/core/core.hpp>    
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include<stdio.h>
#include "face_quality.h"
using namespace cv;
int main(int argc, char** argv)
{
	argv[1] = "./data/image_00000009.jpg";
	argv[2] = "./models";
	LibWaveFaceAlignment face_alignment;
	FaceQuality face_quality;
	int ret = face_quality.Initial("./models/");
	if (ret)
	{
		printf("face_quality error\n");
	}
	else
	{
		printf("face_quality success\n");
	}
        ret = face_quality.Configure();
	if (ret)
	{
		printf("face_quality.Configure error\n");
	}
	else
	{
		printf("face_quality.Configure success\n");
	}
	ret = face_alignment.Initial(argv[2],-1);
	if (ret)
	{
		printf("face_alignment error\n");
	}
	else
	{
		printf("face_alignment success\n");
	}
	FaceRect faceRect;
	faceRect.x = 90;
	faceRect.y = 120;
	faceRect.width = 160;
	faceRect.height = 160;
	Mat img = imread(argv[1]);
	float* landmarks = NULL;
	ret = face_alignment.Alignment((byte*)img.data, img.cols, img.rows, faceRect, 136, &landmarks);
	if (ret)
	{
		printf("face_alignment.Alignment failed\n");
		face_alignment.Release();
		return -1;
	}
	else
	{
		printf("face_alignment.Alignment success\n");
	}
	cv::Mat shape_src(1, 136, CV_32FC1);
        FILE  * fp = fopen("1.txt","w+");
	for (int i = 0; i < 68; i++)
	{
		fprintf(fp,",%f,%f",landmarks[2 * i],landmarks[2 * i+1]);
		shape_src.at<float>(0, 2 * i) = landmarks[2 * i];
        shape_src.at<float>(0, 2 * i + 1) = landmarks[2 * i+1];
		//circle(img, Point(int(landmarks[2 * i]), int(landmarks[2 * i + 1])), 2, CV_RGB(255, 0, 0), 2);
	}
	fclose(fp);
	int label = -1;
    int re_quality = face_quality.Run(img, shape_src, &label);
    printf("%d\n", label);
	printf("face_alignment.Release start\n");
	face_alignment.Release();
	face_quality.Release();
	printf("face_alignment.Release end\n");
	imwrite("1.bmp",img);
	return 0;
}
