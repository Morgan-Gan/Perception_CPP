#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <algorithm>
#include <iomanip>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include "comm/CommDefine.h"
#include "rknn/rknn.h"

namespace Vision_FaceAlg
{
    /**
     * @brief Face detection box.
     */
    struct FaceRect 
    {
        // The x-coordinate of the face box
        int x;
        // The y-coordinate of the face box
        int y;
        // The width of the face box
        int width;
        // The height of the face box
        int height;

        FaceRect(): x(0), y(0), width(0), height(0) {}

        FaceRect(const int &x_,
                const int &y_,
                const int &width_,
                const int &height_)
            : x(x_), y(y_), width(width_), height(height_) 
        {

        }

        FaceRect & operator = (const FaceRect &other) 
        {
            x = other.x;
            y = other.y;
            width = other.width;
            height = other.height;
            return *this;
        }

        FaceRect(const cv::Rect &rect) : 
        x(rect.x), 
        y(rect.y), 
        width(rect.width), 
        height(rect.height) 
        {

        }
        
        FaceRect & operator=(const cv::Rect &other)
        {
            x = other.x;
            y = other.y;
            width = other.width;
            height = other.height;
            return *this;
        }

        cv::Rect GetRect() 
        {
            return cv::Rect(x, y, width, height);
        }

        const cv::Rect GetRect() const 
        {
            return cv::Rect(x, y, width, height);
        }
    };

    /**
     * @brief Face detection information.
     */
    struct FaceRectInfo 
    {
        // Face box of detected.
        FaceRect bbox;

        // Confidence of the detected face.
        float score;

        cv::Mat landmarks;
        FaceRectInfo(): bbox(), score(0.0f) 
        {

        }

        FaceRectInfo(const FaceRect &face_rects,
                const float &score_,
                const cv::Mat &landmarks_): bbox(face_rects), score(score_) 
        {
            landmarks=landmarks_.clone();
        }

        FaceRectInfo& operator = (const FaceRectInfo & other) 
        {
            bbox = other.bbox;
            score = other.score;
            landmarks=other.landmarks.clone();
            return *this;
        }
    };

    /**
     * @brief Face detection
     */
    class FaceDetection 
    {
        using MsgBusShrPtr = std::shared_ptr<common_template::MessageBus>;
        using AlgModelMapType = std::map<std::string,rknn_context>;
        using ResultReportTopicMapType = std::map<std::string,int>;
        using ResultReportTopicLstType = std::vector<std::string>;

    public:
        FaceDetection();
        ~FaceDetection();

        bool Init(const Json& algCfg);

        void Run(const std::string& strIp,const std::string& strCameCode,const cv::Mat& matImg);
        
    private:

        bool Preprocess(const cv::Mat& srcImg,cv::Mat& dstImg,int& s32Width,int& s32Height);

        void OutputTrans(rknn_output outputs[],const cv::Mat& srcImg,const cv::Mat& dstImg,const int& s32Width,const int& s32Height,bool bResize);

        void Release();

        // 获取人脸检测的阈值
        inline float & GetThreshold() 
        {
            return m_fThreshold;
        }

        // 设置人脸检测的阈值
        inline void SetThreshold(const float threshold) 
        {
            m_fThreshold = threshold;
        }

    private:  
        float m_fThreshold;
        AlgModelMapType m_mapAlgModel;
    };

    class DetectOutput 
    {
    public:
        DetectOutput(int mbox_num, int image_w, int image_h) 
        {
            mbox_num_ = mbox_num;
            image_w_ = image_w;
            image_h_ = image_h;
        }
        ~DetectOutput() {}

    int Output(const float* loc_data, const float* conf_data,std::vector<std::vector<float> > *faceinfos, const float face_thresh=0.80);

    private:
        int DecodeBBoxes(const float *t_bboxes, const float *p_bboxes,float *b_bboxes);

        static bool SortScorePairDescend(const std::pair<int, float>& pair1,const std::pair<int, float>& pair2);

        int GetFeatureMapSize2Cov(const int input_w, const int input_h,const int kernel_s, const int stride_s, 
        const int padding_s,int *output_w, int *output_h);
        
        int GetFeatureMapSize2Pol(const int input_w, const int input_h,
            const int kernel_s, const int stride_s, const int padding_s,
            int *output_w, int *output_h);

        int GenPriorbox(const int image_w, const int image_h, const int feamap_w,
            const int feamap_h, const std::vector<int>& min_sizes, const float step,
            float** prior_data, int* prior_data_length, float** variances_data,
            int* variance_data_length);

        int GenAllPriorboxs(float** prior_data);

        int GetMaxScoreIndex(const float *scores, const float thresh, const int top_k,
            std::vector<std::pair<int, float> >* index_score_vec);

        int IntersectBBox(const float *bbox0, const float *bbox1, float *bbox_inter);

        float BBoxSize(const float *bbox);

        float JaccardOverlap(const float *bbox0, const float *bbox1);

        int ApplyNMSFast(const float *b_bboxes, const float *confs,
            const float conf_thresh, const float nms_thresh, const int top_k,
            std::vector<int> *indices);

    private:
        int mbox_num_;
        int image_w_;
        int image_h_;
    };
}