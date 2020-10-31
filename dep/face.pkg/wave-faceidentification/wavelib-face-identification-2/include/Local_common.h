#ifndef __NP_LOCALCOMMON_H_INCLUDE__
#define __NP_LOCALCOMMON_H_INCLUDE__
#pragma once 
#include "caffe/caffe.hpp"
using namespace caffe;
#ifndef BYTE
typedef unsigned char       BYTE;
#endif
typedef struct _OutParam
{
	float *pdata;  //��������
	int dim;         //����ά��
	float *pdata1;  //��������
	int dim1;         //����ά��
}StructOutParam;
//
typedef enum _WV_IMG_TYPE{ WV_BGR = 0, WV_RGB }WV_IMG_TYPE;
typedef	struct _WV_Image
{
	int		width;			//ͼ����
	int		height;			//ͼ����
	int		nWidthStep;		//��Ҫ��4�ֽڶ���
	int		nChannel;		//ͼ��ͨ����
	int		nType;			// ��ʾͼ�����ݵ�����WV_IMG_TYPE 
	unsigned char* pData;
}WV_Image;

typedef struct WV_FacePoint
{
	float x;//�ؼ���������
	float y;//�ؼ���������
}WV_FacePoint;
typedef struct  WV_FaceRectOut
{
	short left;//���������󶥵�������
	short top;//���������󶥵�������
	short right;//���������ҵ׶˺�����
	short bottom;//���������ҵ׶�������
}WV_FaceRect;
typedef struct _WV_FacePoints
{
	int  nFacePointNum;//�����ؼ�����Ŀ
	WV_FacePoint pPoints[68];//�ؼ�����Ϣ����
	WV_FaceRect  pFaceRect;//����λ��
}WV_FacePoints;
typedef struct _WV_FACE_ParamIn
{
	WV_Image nImg;			//����ͼ��
	WV_FacePoints *nFacePoint;//��������������Ϣ ��nFacePoint==null  ������ͼ������ ������������Ϣ����
}WV_FACE_ParamIn;

typedef struct _WV_FACE_ParamOut
{
	BYTE*  pFeature; //����
	int  nFeatlen;  //��������(��byteΪ��λ)
}WV_FACE_ParamOut;
#endif
