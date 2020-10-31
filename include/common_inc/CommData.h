#pragma once
#include <string>
#include <vector>
#include <map>
#include "rknn/rknn.h"

static std::map<string, string> AlgCfgMap =
{
	{std::make_pair("0","detect1")},
	{std::make_pair("1","action1")},
	{std::make_pair("2","face1")}
};

static std::map<string, string> DetectLabsMap =
{
	{std::make_pair("0","人")},{std::make_pair("1","自行车")},{std::make_pair("2","小车")},{std::make_pair("3","摩托车")},{std::make_pair("4","巴士")},
	{std::make_pair("5","火车")},{std::make_pair("6","卡车")},{std::make_pair("7","船")},{std::make_pair("8","交通灯")},{std::make_pair("9","消防栓")},
	{std::make_pair("10","停止标志")},{std::make_pair("11","猫")},{std::make_pair("12","狗")},{std::make_pair("13","背包")},{std::make_pair("14","雨伞")},
    {std::make_pair("15","手提包")},{std::make_pair("16","手提箱")},{std::make_pair("17","刀")},{std::make_pair("18","笔记本电脑")},{std::make_pair("19","手机")},
	{std::make_pair("20","火")}
};

static std::map<string,string> ActionLabsMap =
{
    {std::make_pair("0","抽烟")},{std::make_pair("1","闯入")},{std::make_pair("2","摔倒")},{std::make_pair("3","徘徊")},{std::make_pair("4","攀爬")},{std::make_pair("5","打架")}
};

static bool DetectInference(cv::Mat& img,const detection* dets,int total,int classes,float thresh,const std::string& strTask)
{
    bool bHaveTarget = false;
	for(int i = 0;i < total;i++)
	{
		if(0 == dets[i].objectness) 
        {
            continue;
        }

        int s32Cls = -1;
        int s32TopCls = -1;
		float fTopScore = 0.0;
		for(int j = 0;j < classes;j++)
		{
            if(dets[i].prob[j] <= thresh)
            {
                continue;
            }

            //获取最大评分和最大可能类别
            if (fTopScore < dets[i].prob[j])
            {
                fTopScore = dets[i].prob[j];
                s32TopCls = j;
            }

            if(0 > s32Cls)
            {
                s32Cls = j;
            }
		}

        if(0 != strTask.compare(to_string(s32TopCls)) || 0.0 >= fTopScore)
        {
            continue;
        }

        //画框
        box b = dets[i].bbox;
        int x1 = (b.x - b.w / 2.) * img.cols;
        int x2 = (b.x + b.w / 2.) * img.cols;
        int y1 = (b.y - b.h / 2.) * img.cols - (img.cols - img.rows) / 2;
        int y2 = (b.y + b.h / 2.) * img.cols - (img.cols - img.rows) / 2;

        stringstream strStream;
        strStream << DetectLabsMap[strTask].data() << " : " << fTopScore;

        cv::rectangle(img, Point(x1, y1), Point(x2, y2), colorArray[s32Cls % 10], 3);
        cv::putText(img, strStream.str().c_str(), Point(x1, y1 - 12), 1, 2, Scalar(0, 255, 0, 255));

        bHaveTarget = true;
	}

	return bHaveTarget;
}