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
\brief   ����������
*/
//---------------------------------------------------------------
class FaceQuality {
public:
  //----------------------------------------------------------------
  //brief: ��ʼ������
  //input: model_path, ����ģ���ļ����ڵ��ļ���·�����磺"E:\\model\\"
  //return: 0������-1�쳣
  //-----------------------------------------------------------------
  int Initial(const char* model_path);

  //----------------------------------------------------------------
  //brief: �޸��ж�Ϊ������ģ��ͼ�����ֵ
  //input: thresh��Ĭ��0.5��ʵ��ʱѡ��0.3����� >= thresh���������Ϊ
  //       ����ͼ�񣬷���Ϊģ��ͼ��
  //return: 0������-1�쳣
  //-----------------------------------------------------------------
  int Configure(const float& thresh = 0.5);

  //----------------------------------------------------------------
  //brief: ͼ�������ж�����
  //input: img, rgb, ԭͼ
  //input: landmarks, cv::Mat CV_32FC1, 1*136, x1,y1,x2,y2,...,x68,y68
  //output: label, ����0Ϊģ��ͼ�񣬵���1Ϊ����ͼ��
  //return: 0������-1�쳣
  //-----------------------------------------------------------------
  int Run(const cv::Mat& img, const cv::Mat& landmarks, int* label);

  //----------------------------------------------------------------
  //brief: ���캯������ʼ����Ա����
  //-----------------------------------------------------------------
  FaceQuality();

  //----------------------------------------------------------------
  //brief: �����������ͷų�Ա����
  //-----------------------------------------------------------------
  ~FaceQuality();

  //----------------------------------------------------------------
  //brief: �ͷź���
  //return: 0������-1�쳣
  //-----------------------------------------------------------------
  int Release();
private:
  void *quality_model_;

  // input: thresh, �жϵ�������ͼ����������ģ������ֵ��
  // ��� >= thresh���������Ϊ����������Ϊģ��
  float thresh_;
};

#endif
