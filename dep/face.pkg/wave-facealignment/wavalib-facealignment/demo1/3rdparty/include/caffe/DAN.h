#ifndef __NP_DAN_H_INCLUDE__
#define __NP_DAN_H_INCLUDE__
#pragma once 
#include <opencv2/core/core.hpp>    
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;
using namespace std;
int GetAffineParam(float*ShapesFrom, float* ShapeTo, Mat& T, Mat& C);
int generratePixels(float * pixels);
int AffineLandmark(float* Landmark, const Mat& R, const Mat& C, float* LandmarkOut, bool isInv = false);
int AffineImage(Mat Image, const Mat& R, const Mat& C, Mat& outImage, bool isInv = false);
int AffinefloatImage(float* Image, int nheight, int nwidth, int nchannels, const Mat& R, const Mat& C, float* outImage, float value, bool isInv = false);
int GetHeatMap(float* Landmark, int HalfSize, float* HeatMap);
#endif