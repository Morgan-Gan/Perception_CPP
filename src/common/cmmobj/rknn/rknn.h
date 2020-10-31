#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <functional>

#include "rknn/rknn_api.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/videoio.hpp"

using namespace std;
using namespace cv;

const int GRID0 = 80;
const int GRID1 = 40;
const int GRID2 = 20;

const int nanchor = 3;                        //n anchor per yolo layer
const int nboxes_0 = GRID0 * GRID0 * nanchor;
const int nboxes_1 = GRID1 * GRID1 * nanchor;
const int nboxes_2 = GRID2 * GRID2 * nanchor;
const int nboxes_total = nboxes_0 + nboxes_1 + nboxes_2;

#pragma pack(push, 1)  
    typedef struct
    {
        float x,y,w,h;
    }box;

    typedef struct detection
    {
        box bbox;
        int classes;
        float *prob;
        float objectness;
        int sort_class;
    } detection;
#pragma pack(pop)

static Scalar colorArray[10] = 
{
    Scalar(139,   0,   0, 255),
    Scalar(139,   0, 139, 255),
    Scalar(  0,   0, 139, 255),
    Scalar(  0, 100,   0, 255),
    Scalar(139, 139,   0, 255),
    Scalar(209, 206,   0, 255),
    Scalar(  0, 127, 255, 255),
    Scalar(139,  61,  72, 255),
    Scalar(  0, 255,   0, 255),
    Scalar(255,   0,   0, 255),
};

static float expit(float x)
{
    return (float) (1.0 / (1.0 + exp(-x)));
}

static void free_detections(detection *dets, int n)
{
    int i;
    for(i = 0; i < n; ++i)
	{
        free(dets[i].prob);
    }
    free(dets);
}

static box get_yolo_box(float *x, float *biases, int n, int index, int i, int j, int lw, int lh, int netw, int neth, int stride)
{
	box b;
	b.x = (i + x[index + 0 * stride]) / lw;
	b.y = (j + x[index + 1 * stride]) / lh;
	b.w = (x[index + 2 * stride]) * biases[2 * n] / netw;
	b.h = (x[index + 3 * stride]) * biases[2 * n + 1] / neth;
	return b;
}

static float overlap(float x1,float w1,float x2,float w2)
{
	float l1=x1-w1/2;
	float l2=x2-w2/2;
	float left=l1>l2? l1:l2;
	float r1=x1+w1/2;
	float r2=x2+w2/2;
	float right=r1<r2? r1:r2;
	return right-left;
}

static float box_intersection(box a, box b)
{
    float w = overlap(a.x, a.w, b.x, b.w);
    float h = overlap(a.y, a.h, b.y, b.h);
    if(w < 0 || h < 0) return 0;
    float area = w*h;
    return area;
}

static float box_union(box a, box b)
{
    float i = box_intersection(a, b);
    float u = a.w*a.h + b.w*b.h - i;
    return u;
}

static float box_iou(box a, box b)
{
    return box_intersection(a, b)/box_union(a, b);
}

static int nms_comparator(const void *pa, const void *pb)
{
    detection a = *(detection *)pa;
    detection b = *(detection *)pb;
    float diff = 0;
    if(b.sort_class >= 0)
	{
        diff = a.prob[b.sort_class] - b.prob[b.sort_class];
    } 
	else 
	{
        diff = a.objectness - b.objectness;
    }

    if(diff < 0) return 1;
    else if(diff > 0) return -1;
    return 0;
}

static int do_nms_sort(detection *dets, int total, int classes, float thresh)
{
    int i, j, k;
    k = total-1;
    for(i = 0; i <= k; ++i)
	{
        if(dets[i].objectness == 0)
        {
            detection swap = dets[i];
            dets[i] = dets[k];
            dets[k] = swap;
            --k;
            --i;
        }
    }

    total = k+1;
    for(k = 0; k < classes; ++k)
	{
        for(i = 0; i < total; ++i)
		{
            dets[i].sort_class = k;
        }
        qsort(dets, total, sizeof(detection), nms_comparator);
        for(i = 0; i < total; ++i)
        {
            if(dets[i].prob[k] == 0) continue;
            box a = dets[i].bbox;
            for(j = i+1; j < total; ++j)
            {
                box b = dets[j].bbox;
                if (box_iou(a, b) > thresh)
                {
                    dets[j].prob[k] = 0;
                }
            }
        }
    }
	return total;
}

static void get_network_boxes(float *predictions, int netw, int neth, int GRID, int *masks, float *anchors, int box_off, int classes,float obj_thresh,detection *dets)
{
	int lw = GRID;
	int lh = GRID;
	int lwh = lw * lh;
	int lst_size = 1 + 4 + classes;  
	int index_start = 0;
	int index_end = 0;

	//x和y
    for (int n = 0; n < nanchor; n++) 
	{
		index_start = (n * lst_size + 0)* lwh;  
		index_end = index_start + 2 * lwh;
		for (int i = index_start; i < index_end; i++)
		{
			predictions[i] = ((1. / (1. + exp(-predictions[i]))) * 2. - 0.5);
		}
	}

	//w和h
	for (int n = 0; n < nanchor; n++) 
	{
		index_start = (n * lst_size + 2) * lwh; 
		index_end = index_start + 2 * lwh;
		for (int i = index_start; i < index_end; i++)
		{
			predictions[i] = pow((1. / (1. + exp(-predictions[i]))) * 2.,2);
		}
	}

	//obj做logistic
	for (int n = 0; n < nanchor; n++)
	{
		index_start = (n * lst_size + 4) * lwh;
		index_end = index_start + 1 * lwh;
		for (int i = index_start; i < index_end; i++)
		{  
			predictions[i] = 1. / (1. + exp(-predictions[i]));
		}
	}

	//class做logistic 
	for (int n = 0; n < nanchor; n++)
	{
		index_start = (n * lst_size + 5) * lwh;
		index_end = index_start + classes * lwh;
		for (int i = index_start; i < index_end; i++)
		{
			predictions[i] = 1. / (1. + exp(-predictions[i]));
		}
	}

	//dets将outpus重排列,dets[i]为第i个框,box顺序为先anchor再grid
    int count = box_off;
	for (int i = 0; i < lwh; i++)
	{
		int row = i / lw;
		int col = i % lw;
		for (int n = 0; n < nanchor; n++)
		{
			int box_index = n * lst_size * lwh + i; //box的x索引,ywh索引只要依次加上lw*lh
			int obj_index = box_index + 4 * lwh;
			float objectness = predictions[obj_index];
			if (objectness < obj_thresh)
            {
                continue;
            }

			dets[count].objectness = objectness;
			dets[count].classes = classes;
			dets[count].bbox = get_yolo_box(predictions, anchors, masks[n], box_index, col, row, lw, lh, netw, neth, lwh);
			for (int j = 0; j < classes; j++)
			{
				int class_index = box_index + (5 + j) * lwh;
				dets[count].prob[j] = objectness * predictions[class_index];
			}
			++count;
		}
	}
}

static int outputs_transform(rknn_output rknn_outputs[], int net_width, int net_height,int classes,float obj_thresh,float nms_thresh,detection *dets)
{
    float *output_0 = (float *)rknn_outputs[1].buf;  //80
	float *output_1 = (float *)rknn_outputs[2].buf;  //40
	float *output_2 = (float *)rknn_outputs[3].buf;  //20
	int masks_0[3] = {0, 1, 2};
	int masks_1[3] = {3, 4, 5};
	int masks_2[3] = {6, 7, 8};
	float anchors[18] = {10, 13, 16, 30, 33, 23, 30, 61, 62, 45, 59, 119, 116, 90, 156, 198, 373, 326};

	//输出xywh均在0-1范围内
	get_network_boxes(output_0, net_width, net_height, GRID0, masks_0, anchors, 0, classes,obj_thresh,dets);
	get_network_boxes(output_1, net_width, net_height, GRID1, masks_1, anchors, nboxes_0,classes,obj_thresh, dets);
	get_network_boxes(output_2, net_width, net_height, GRID2, masks_2, anchors, nboxes_0 + nboxes_1,classes,obj_thresh,dets);

    //抑制
    return  do_nms_sort(dets, nboxes_total, classes, nms_thresh);
}