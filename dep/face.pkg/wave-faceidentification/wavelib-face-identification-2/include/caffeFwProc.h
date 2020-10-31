#ifndef FACE_CAFFEPROC_HPP_
#define FACE_CAFFEPROC_HPP_
#include <glog/logging.h>
#include "Local_common.h"
#include "face_segement.h"
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
#include "boost/algorithm/string.hpp"
#include "caffe/caffe.hpp"
using namespace cv;
using namespace caffe;  // NOLINT(build/namespaces)
using std::max;
using std::min;
#ifndef NP_ERROR_NONE
#define NP_ERROR_NONE				  0     //正常
#endif
#ifndef NP_ERROR_LICENSE
#define	NP_ERROR_LICENSE		     -1		// 授权错误
#endif
#ifndef NP_ERROR_INIT
#define NP_ERROR_INIT				 -2     //初始化错误
#endif
#ifndef NP_ERROR_HANDLE
#define NP_ERROR_HANDLE				 -3		// 句柄错误
#endif
#ifndef NP_ERROR_MEMORY
#define NP_ERROR_MEMORY			     -4     //内存错误
#endif
#ifndef NP_ERROR_PARAM
#define NP_ERROR_PARAM				 -5     //参数错误
#endif
#ifndef NP_ERROE_CALL
#define NP_ERROE_CALL                -6     //函数调用失败
#endif
#ifdef NP_ERROE_IMAGE
#define NP_ERROE_IMAGE               -7
#endif
int ForwardInit(void** net, const char *model, const char *weight, int bgpu);
int ForwardProcess(Net<float> *caffe_net, Net<float> *caffe_net1, WV_FACE_ParamIn *inPara, StructOutParam *outPara);
int ForwardUinit();
#endif