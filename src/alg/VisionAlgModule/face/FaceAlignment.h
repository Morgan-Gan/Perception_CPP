#pragma once
#include "comm/CommDefine.h"
#include "rknn/rknn.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

namespace Vision_FaceAlg
{
    class  FaceAlignment 
    {
    public:
        FaceAlignment() = default;
        
        ~FaceAlignment() = default;

        // initial model
        int Init(const Json& algCfg);

        // release resource
        int Release();

        // alignment with detect box
        int Alignment(const cv::Mat &img, const cv::Rect &bbox, cv::Mat* landmarks);

        //get mean shape
        int GetMeanShape(cv::Mat *mean_landmarks);

    private:
        rknn_context m_FaceAlignment;
    };
}