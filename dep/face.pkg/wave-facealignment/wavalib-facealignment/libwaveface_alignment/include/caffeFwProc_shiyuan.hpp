#ifndef _WV_CAFFEPROC_H_
#define _WV_CAFFEPROC_H_
#pragma once
#include "caffe/caffe.hpp"

int caffeALLForwardProcess1(void *net, char* rgb, int height, int width, float**pFeature1, 	int& pfeaturelen1, float**pFeature2, int& pfeaturelen2);
#endif
