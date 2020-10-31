#ifndef __NP_FACESEGEMENT_H_INCLUDE__
#define __NP_FACESEGEMENT_H_INCLUDE__
#pragma once 
#include "opencv2/opencv.hpp"
int CropImgByFace(cv::Mat img, cv::Mat& crop_img, cv::Rect rect, cv::Point2f pts[68], int pointNum, int isIDCardPthoto = 0);

#endif
