// Copyright [2018] <Copyright wavewisdom-bj>
// All right reserved
//
// Authors: luyaxuan@wavewisdom-bj.com (yaxuan lu)
#ifndef FACEQUALITY_H_
#define FACEQUALITY_H_
#define LIBFACEQUALITY_EXPORTS
#ifdef LIBFACEQUALITY_EXPORTS
#define LIBFACEQUALITY_API __declspec(dllexport)
#else
#define LIBFACEQUALITY_API __declspec(dllimport)
#endif

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>

#ifndef FACE_QUALITY_LOG 
#define FACE_QUALITY_LOG "./logs/face_quality_log"
#endif

//---------------------------------------------------------------
/**
\brief   人脸质量类
*/
//---------------------------------------------------------------
class FaceQuality {
public:
  //----------------------------------------------------------------
  //brief: 初始化函数
  //input: model_path, 网络模型文件所在的文件夹路径，如："E:\\model\\"
  //return: 0正常，-1异常
  //-----------------------------------------------------------------
  int Initial(const char* model_path);

  //----------------------------------------------------------------
  //brief: 修改判断为清晰或模糊图像的阈值
  //input: thresh，默认0.5，实验时选用0.3，如果 >= thresh，则该人脸为
  //       清晰图像，否则为模糊图像
  //return: 0正常，-1异常
  //-----------------------------------------------------------------
  int Configure(const float& thresh = 0.5);

  //----------------------------------------------------------------
  //brief: 图像质量判定函数
  //input: img, rgb, 原图
  //input: landmarks, cv::Mat CV_32FC1, 1*136, x1,y1,x2,y2,...,x68,y68
  //output: label, 等于0为模糊图像，等于1为清晰图像
  //return: 0正常，-1异常
  //-----------------------------------------------------------------
  int Run(const cv::Mat& img, const cv::Mat& landmarks, int* label);

  //----------------------------------------------------------------
  //brief: 构造函数，初始化成员变量
  //-----------------------------------------------------------------
  FaceQuality();

  //----------------------------------------------------------------
  //brief: 析构函数，释放成员变量
  //-----------------------------------------------------------------
  ~FaceQuality();

  //----------------------------------------------------------------
  //brief: 释放函数
  //return: 0正常，-1异常
  //-----------------------------------------------------------------
  int Release();
private:
  void *quality_model_;

  // input: thresh, 判断单个人脸图像清晰还是模糊的阈值，
  // 如果 >= thresh，则该人脸为清晰，否则为模糊
  float thresh_;
};

#endif
