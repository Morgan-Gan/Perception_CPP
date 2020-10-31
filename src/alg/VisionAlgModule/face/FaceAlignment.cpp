#include "FaceAlignment.h"
#include "opencv2/opencv.hpp"
#include "boost/algorithm/string.hpp"

#include <sys/stat.h>  
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include "log4cxx/Loging.h"
#include "boost/BoostFun.h"
#include "SubTopic.h"
#include "endecode/Base64.h"
#include "comm/CommFun.h"
#include "CommData.h"
#include "comm/FuncInitLst.h"

using namespace Vision_FaceAlg;
using namespace std;
using namespace cv;
using std::max;
using std::min;

const string strAlignModel("./model/face_alignment_v0.2.8.rknn");

#define  LANDMARK 98
#define  IMGSIZE  112
float meanshape[] = { 24.320467,39.407368,24.597822,43.66881,24.880284,47.936487,25.243382,
	52.196175,25.782354,56.434764,26.573608,60.631519,27.678472,
	64.749893,29.152941,68.738546,31.036695,72.534362,33.319185,
	76.084689,35.94321,79.368436,38.831175,82.397179,41.939465,
	85.175336,45.257849,87.649128,48.861974,89.649954,52.817248,
	90.922258,56.989872,91.265181,61.07696,90.745386,64.902175,
	89.318177,68.371771,87.210931,71.575724,84.679057,74.576937,
	81.866016,77.35225,78.806712,79.851203,75.495769,82.007359,
	71.933469,83.771902,68.144983,85.138115,64.180878,86.139123,
	60.098245,86.830165,55.944581,87.269057,51.753722,87.525711,
	47.545077,87.690725,43.328216,87.841259,39.114135,32.340487,
	35.382014,36.644548,31.556096,41.15134,31.201726,45.713768,
	31.728684,50.033376,32.79944,50.015515,35.602285,45.700912,
	34.737936,41.135413,34.231989,36.650928,34.400147,61.752759,
	32.534544,66.289103,31.392916,70.778809,30.804891,75.304553,
	31.136594,79.44653,34.763642,75.279064,33.903044,70.764796,
	33.81038,66.284489,34.423792,61.756279,35.349327,55.999596,
	40.442686,56.046325,46.118932,56.075051,51.769817,56.20276,
	57.128235,49.309906,59.873409,52.834321,60.61046,56.414975,
	61.210938,59.770841,60.496012,63.071771,59.683768,36.786396,
	40.926331,39.431244,39.288467,42.532242,38.641046,45.859948,
	39.235842,48.477662,41.293659,45.594674,42.23948,42.534473,
	42.568611,39.525443,42.136936,63.498434,41.156329,66.279873,
	38.912694,69.869444,38.371096,72.73524,39.032976,75.212714,
	40.532725,72.76822,41.718537,70.06451,42.248524,66.70316,
	42.044711,44.114484,69.596668,48.769053,67.539055,54.222445,
	66.774374,56.557149,66.818011,58.934571,66.711822,64.100324,
	67.411218,68.47571,69.275359,65.459467,72.756651,61.441087,
	75.403441,56.612661,76.368705,51.591258,75.61003,47.352046,
	73.10145,45.042806,69.780587,50.646818,69.074181,56.578323,
	69.071735,62.233532,68.929724,67.587818,69.444846,62.467082,
	71.79437,56.563245,72.779855,50.420752,72.013924,42.799299,
	40.587724,69.695197,40.286312 };

float max_min[] = { 27.07073828, 29.18361271, 85.53304006, 88.44473259 };
extern char landmark_net[];
int AffineLandmark1(float* Landmark, const Mat& R, const Mat& C, float* LandmarkOut, bool isInv = false)

{
	if (Landmark == NULL || LandmarkOut == NULL)
	{
		return -1;
	}
	Mat A, T;
	if (isInv == false)
	{
		A = R.inv();

		T.create(1, 2, CV_32FC1);
		float *pData = (float*)T.data;
		float *pData1 = (float*)C.data;
		float* pData2 = (float*)A.data;
		pData[0] = (pData1[0] * pData2[0] + pData1[1] * pData2[2])*(-1.0);
		pData[1] = (pData1[0] * pData2[1] + pData1[1] * pData2[3])*(-1.0);
	}
	else
	{
		R.copyTo(A);
		C.copyTo(T);
	}
	float* pData = LandmarkOut;
	float* pData1 = Landmark;
	float* a = (float*)A.data;
	float* b = (float*)T.data;
	for (int i = 0; i < LANDMARK; i++)
	{
		pData[0] = pData1[0] * a[0] + pData1[1] * a[2] + b[0];
		pData[1] = pData1[0] * a[1] + pData1[1] * a[3] + b[1];
		pData += 2;
		pData1 += 2;
	}
	return 0;
}
int transform(float* landmarks, float* meanShape, Mat& T, Mat& C)
{
	if (landmarks == NULL || meanShape == NULL)
	{
		return -1;
	}
	float dest_mean_x = 0;
	float dest_mean_y = 0;
	float src_mean_x = 0;
	float src_mean_y = 0;
	float* pData = meanShape;
	for (int i = 0; i < LANDMARK; i++)
	{
		dest_mean_x += *pData;
		pData++;
		dest_mean_y += *pData;
		pData++;
	}
	dest_mean_x /= LANDMARK;
	dest_mean_y /= LANDMARK;
	pData = landmarks;
	for (int i = 0; i < LANDMARK; i++)
	{
		src_mean_x += *pData;
		pData++;
		src_mean_y += *pData;
		pData++;
	}
	src_mean_x /= LANDMARK;
	src_mean_y /= LANDMARK;
	int len = LANDMARK * 2;
	float *srcVec = new float[len];
	float *destVec = new float[len];
	pData = landmarks;
	float* pData1 = srcVec;
	for (int i = 0; i < LANDMARK; i++)
	{
		*pData1 = *pData - src_mean_x;
		pData++;
		pData1++;
		*pData1 = *pData - src_mean_y;
		pData++;
		pData1++;
	}
	pData = meanShape;
	pData1 = destVec;
	for (int i = 0; i < LANDMARK; i++)
	{
		*pData1 = *pData - dest_mean_x;
		pData++;
		pData1++;
		*pData1 = *pData - dest_mean_y;
		pData++;
		pData1++;
	}
	float sum1 = 0;
	float sum2 = 0.0;
	pData = srcVec;
	pData1 = destVec;
	for (int i = 0; i < len; i++)
	{
		sum1 += (*pData)*(*pData1);
		sum2 += (*pData)*(*pData);
		pData++;
		pData1++;
	}
	float a = sum1 / sum2;
	float b = 0;
	pData = srcVec;
	pData1 = destVec;
	for (int i = 0; i < LANDMARK; i++)
	{
		b += pData[0] * pData1[1] - pData[1] * pData1[0];
		pData += 2;
		pData1 += 2;
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
int bestFitRect(float* landmarks, float* meanShape, float*box, float* S0)
{
	if ((landmarks == NULL&&box == NULL) || meanShape == NULL || S0 == NULL)
	{
		return -1;
	}
	float rect[4];
	if (box == NULL)
	{
		float x_min = 100000;
		float y_min = 100000;
		float x_max = 0;
		float y_max = 0;
		float* pData = landmarks;
		for (int i = 0; i < LANDMARK; i++)
		{
			if (*pData<x_min)
			{
				x_min = *pData;
			}
			if (*pData > x_max)
			{
				x_max = *pData;
			}
			pData++;
			if (*pData<y_min)
			{
				y_min = *pData;
			}
			if (*pData > y_max)
			{
				y_max = *pData;
			}
			pData++;

		}
		rect[0] = x_min;
		rect[1] = y_min;
		rect[2] = x_max;
		rect[3] = y_max;
	}
	else
	{
		memcpy(rect, box, sizeof(float)* 4);
	}
	float boxCenter_x, boxCenter_y, boxWidth, boxHeight;
	boxCenter_x = (rect[0] + rect[2]) / 2;
	boxCenter_y = (rect[1] + rect[3]) / 2;
	boxWidth = rect[2] - rect[0];
	boxHeight = rect[3] - rect[1];
	float x_min = 10000;
	float y_min = 10000;
	float x_max = 0;
	float y_max = 0;
	float* pData = meanShape;
	for (int i = 0; i < LANDMARK; i++)
	{
		if (*pData<x_min)
		{
			x_min = *pData;
		}
		if (*pData > x_max)
		{
			x_max = *pData;
		}
		pData++;
		if (*pData<y_min)
		{
			y_min = *pData;
		}
		if (*pData > y_max)
		{
			y_max = *pData;
		}
		pData++;

	}
	float meanShapeWidth = x_max - x_min;
	float meanShapeHeight = y_max - y_min;
	float scaleWidth = boxWidth / meanShapeWidth;
	float	scaleHeight = boxHeight / meanShapeHeight;
	float scale = (scaleWidth + scaleHeight) / 2;
	int len = LANDMARK * 2;
	pData = meanShape;
	float* pData1 = S0;
	for (int i = 0; i < len; i++)
	{
		*pData1 = *pData*scale;
		pData++;
		pData1++;
	}
	pData = S0;
	x_min = 10000;
	y_min = 10000;
	x_max = 0;
	y_max = 0;
	for (int i = 0; i < LANDMARK; i++)
	{
		if (*pData<x_min)
		{
			x_min = *pData;
		}
		if (*pData > x_max)
		{
			x_max = *pData;
		}
		pData++;
		if (*pData<y_min)
		{
			y_min = *pData;
		}
		if (*pData > y_max)
		{
			y_max = *pData;
		}
		pData++;
	}
	float S0Center_x = (x_min + x_max) / 2;
	float S0Center_y = (y_min + y_max) / 2;
	float deta_x = boxCenter_x - S0Center_x;
	float deta_y = boxCenter_y - S0Center_y;
	pData1 = S0;
	for (int i = 0; i < LANDMARK; i++)
	{
		*pData1 += deta_x;
		pData1++;
		*pData1 += deta_y;
		pData1++;
	}
	return 0;
}
int RandomSRT(const Mat color, float*landmark, float*MeanShape, Mat& ImgVec, float* Land)
{
	if (MeanShape == NULL || landmark == NULL || color.data == NULL || Land == NULL)
	{
		return -1;
	}

	float* S0 = new float[LANDMARK * 2];
	int ret = bestFitRect(MeanShape, landmark, NULL, S0);
	if (ret)
	{
		delete[] S0;
		return ret;
	}
	Mat R, T;
	ret = transform(landmark, S0, R, T);
	if (ret)
	{
		delete[] S0;
		return ret;
	}
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
	for (int i = 0; i < IMGSIZE; i++)
	{

		for (int j = 0; j < IMGSIZE; j++)
		{
			float x1 = j * pData2[0] + i * pData2[2] + pData1[0];
			float y1 = j * pData2[1] + i * pData2[3] + pData1[1];
			int x = x1;
			int y = y1;
			if (x < 1 || x >= color.cols - 1 || y < 1 || y >= color.rows - 1)
			{
				ImageDst[i*ImgVec.step[0] + 3 * j] = 0;
				ImageDst[i*ImgVec.step[0] + 3 * j + 1] = 0;
				ImageDst[i*ImgVec.step[0] + 3 * j + 2] = 0;
			}
			else
			{
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
int GetAffineParam1(float*ShapesFrom, float* ShapeTo, Mat& T, Mat& C)
{
	if (ShapesFrom == NULL || ShapeTo == NULL)
	{
		return -1;
	}
	float dest_mean_x = 0;
	float dest_mean_y = 0;
	float src_mean_x = 0;
	float src_mean_y = 0;
	float* pData = ShapeTo;
	for (int i = 0; i < LANDMARK; i++)
	{
		dest_mean_x += *pData;
		pData++;
		dest_mean_y += *pData;
		pData++;
	}
	dest_mean_x /= LANDMARK;
	dest_mean_y /= LANDMARK;
	pData = ShapesFrom;
	for (int i = 0; i < LANDMARK; i++)
	{
		src_mean_x += *pData;
		pData++;
		src_mean_y += *pData;
		pData++;
	}
	src_mean_x /= LANDMARK;
	src_mean_y /= LANDMARK;
	int len = LANDMARK * 2;
	float *srcVec = new float[len];
	float *destVec = new float[len];
	pData = ShapesFrom;
	float* pData1 = srcVec;
	for (int i = 0; i < LANDMARK; i++)
	{
		*pData1 = *pData - src_mean_x;
		pData++;
		pData1++;
		*pData1 = *pData - src_mean_y;
		pData++;
		pData1++;
	}
	pData = ShapeTo;
	pData1 = destVec;
	for (int i = 0; i < LANDMARK; i++)
	{
		*pData1 = *pData - dest_mean_x;
		pData++;
		pData1++;
		*pData1 = *pData - dest_mean_y;
		pData++;
		pData1++;
	}
	float sum1 = 0;
	float sum2 = 0.0;
	pData = srcVec;
	pData1 = destVec;
	for (int i = 0; i < len; i++)
	{
		sum1 += (*pData)*(*pData1);
		sum2 += (*pData)*(*pData);
		pData++;
		pData1++;
	}
	float a = sum1 / sum2;
	float b = 0;
	pData = srcVec;
	pData1 = destVec;
	for (int i = 0; i < LANDMARK; i++)
	{
		b += pData[0] * pData1[1] - pData[1] * pData1[0];
		pData += 2;
		pData1 += 2;
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

static void printRKNNTensor(rknn_tensor_attr *attr) {
    printf("index=%d name=%s n_dims=%d dims=[%d %d %d %d] n_elems=%d size=%d fmt=%d type=%d qnt_type=%d fl=%d zp=%d scale=%f\n", 
            attr->index, attr->name, attr->n_dims, attr->dims[3], attr->dims[2], attr->dims[1], attr->dims[0], 
            attr->n_elems, attr->size, 0, attr->type, attr->qnt_type, attr->fl, attr->zp, attr->scale);
}

static unsigned char *load_model(const char *filename, int *model_size)
{
    FILE *fp = fopen(filename, "rb");
    if(fp == nullptr) {
	printf("fopen %s fail!\n", filename);
	return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    unsigned char *model = (unsigned char*)malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if(model_len != fread(model, 1, model_len, fp)) {
	printf("fread %s fail!\n", filename);
	free(model);
	return NULL;
    }
    *model_size = model_len;
    if(fp) {
	fclose(fp);
    }
    return model;
}

int FaceAlignment::Init(const Json& algCfg) 
{   
    int model_len = 0;
    unsigned char *model;
    model = load_model(strAlignModel.c_str(), &model_len);
	if (model == NULL) 
	{
		return -1;
	}

    int ret = rknn_init((rknn_context*)(&m_FaceAlignment), model, model_len, 0);
    if(ret < 0) 
	{
        printf("rknn_init fail! ret=%d\n", ret);
        return -1;
    }

	return 0;
}

int FaceAlignment::Alignment(const  cv::Mat &img,const  cv::Rect &bbox, cv::Mat* landmarks) 
{
	if (img.data == NULL || img.cols <= 0 || img.rows <= 0 || img.channels() != 3)
	{
		printf("image data is null or channels is not 3 !\n");		
 		return -1;
	}

	//resize img
	cv::Size dsize = cv::Size(img.cols, img.rows);
	cv::Mat cv_img_origin(dsize, CV_8UC3, (char*)img.data, 3 * img.cols);
	Mat frame;
	cv_img_origin.copyTo(frame);

	float scale = (int)min(bbox.height, bbox.width) / (float)((max_min[2] - max_min[0]));
	float* initlandmarks = new float[LANDMARK * 2];
	float* pData = initlandmarks;
	float* pData1 = meanshape;
	for (int i = 0; i < LANDMARK; i++)
	{
		*pData = (*pData1  - max_min[0]) * scale + bbox.x;
		pData1++;
		pData++;

		*pData = (*pData1 - max_min[1]) * scale + bbox.y;
		pData1++;
		pData++;
	}

	Mat image;
	float* lands = new float[LANDMARK * 2];
	int ret = RandomSRT(frame, initlandmarks, meanshape, image, lands);
	if (ret)
	{
		delete[] lands;
		delete[] initlandmarks;
		return ret;
	}

	Mat R, A ;
	ret = GetAffineParam1(initlandmarks, lands, R, A);
	if (ret)
	{
		delete[] lands;
		delete[] initlandmarks;
		return ret;
	}

    // Get Model Input Output Info
    // Load RKNN Model
    rknn_input_output_num io_num;
    ret = rknn_query(m_FaceAlignment, RKNN_QUERY_IN_OUT_NUM , &io_num, sizeof(io_num));
    if (ret != RKNN_SUCC) 
	{
        printf("rknn_query fail! ret=%d\n", ret);
        return -1;
    }

    rknn_tensor_attr input_attrs[io_num.n_input];
    memset(input_attrs, 0, sizeof(input_attrs));
    for (int i = 0; i < io_num.n_input; i++) 
	{
        input_attrs[i].index = i;
        ret = rknn_query(m_FaceAlignment, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC) 
		{
            printf("rknn_query fail! ret=%d\n", ret);
            return -1;
        }
    }

    rknn_tensor_attr output_attrs[io_num.n_output];
    memset(output_attrs, 0, sizeof(output_attrs));
    for (int i = 0; i < io_num.n_output; i++) 
	{
        output_attrs[i].index = i;
        ret = rknn_query(m_FaceAlignment, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC) 
		{
            printf("rknn_query fail! ret=%d\n", ret);
            return -1;
        }
    }

	//rknn extract feature 
	// Set Input Data
    Mat image_gray;
    cvtColor(image, image_gray, COLOR_BGR2GRAY);
    rknn_input inputs[1];
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].size = image_gray.cols*img.rows*image_gray.channels();
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    inputs[0].buf = image_gray.data;
    ret = rknn_inputs_set(m_FaceAlignment, io_num.n_input, inputs);
    if(ret < 0) 
	{
        printf("rknn_input_set fail! ret=%d\n", ret);
        return -1;
    }

    // Run
    rknn_output outputs[1];
    ret = rknn_run(m_FaceAlignment, nullptr);
      if(ret < 0) 
	{
        printf("rknn_run fail! ret=%d\n", ret);
        return -1;
    }

    // Get Output
    memset(outputs, 0, sizeof(outputs));
    outputs[0].want_float = 1;
    ret = rknn_outputs_get(m_FaceAlignment, 1, outputs, NULL);
    if(ret < 0) 
	{
        printf("rknn_outputs_get fail! ret=%d\n", ret);
        return -1;
    }

	int len = LANDMARK * 2;
	if (output_attrs[0].n_elems != len)
	{
		delete[] lands;
		delete[] initlandmarks;
		return -1;
	}
    for (int i = 0; i < output_attrs[0].n_elems; i++) 
	{
        float val = ((float*)(outputs[0].buf))[i];
        lands[i] = val + meanshape[i];          
    }
	rknn_outputs_release(m_FaceAlignment, 1, outputs);
	
	//rknn extract feature end	
	AffineLandmark1(lands, R, A, initlandmarks, false);
	*landmarks = cv::Mat(1, LANDMARK*2, CV_32FC1);
	memcpy((*landmarks).data, initlandmarks, sizeof(float)*len);

	delete[] lands;
	delete[] initlandmarks;

	return 0;
}

int FaceAlignment::GetMeanShape(cv::Mat* mean_landmarks) 
{
	*mean_landmarks = cv::Mat(1, LANDMARK*2, CV_32FC1);
	memcpy(mean_landmarks->data, meanshape, sizeof(float)*LANDMARK * 2);
	return 0;
}

int FaceAlignment::Release() 
{
	rknn_destroy(m_FaceAlignment);
   	return 0;
}
