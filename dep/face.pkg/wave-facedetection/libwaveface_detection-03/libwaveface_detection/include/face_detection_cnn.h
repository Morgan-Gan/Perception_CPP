#ifndef FACE_BOXES_H_
#define FACE_BOXES_H_

#include <caffe/caffe.hpp>
#ifdef USE_OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif  // USE_OPENCV
#include <algorithm>
#include <iomanip>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>


using caffe::Net;
using caffe::Blob;
using std::string;
using std::vector;
using std::shared_ptr;

// 人脸框
struct BBox {
  float x1;
  float y1;
  float x2;
  float y2;
  cv::Rect GetRect() const;
  BBox GetSquare() const;
};

// 人脸相关信息，人脸框，回归信息，人脸分数，特征点
struct Face {
  BBox bbox;
  float score;
  
  static void Bboxes2Squares(std::vector<Face>& faces);
};


class FaceDetector {
 public:
  FaceDetector();
  ~FaceDetector();
  int Initial(const string &path,
              const int& min_face_size = 40,
              const float &threshold = 0.9,
              const int &gpu_id = -1);

  std::vector<Face> Detect(const cv::Mat& img);
  
  int Release();
 private:
  void SetMean(const string& mean_value);

  void WrapInputLayer(std::vector<cv::Mat>* input_channels);

  void Preprocess(const cv::Mat& img,
                  std::vector<cv::Mat>* input_channels);
public:
  // 获取人脸检测的阈值
  inline const float &Threshold() const {
    return threshold_;
  }
  inline float & Threshold() {
    return threshold_;
  }
  // 设置人脸检测的阈值
  inline void SetThreshold(const float threshold) {
    this->threshold_ = threshold;
  }

  inline void SetMinFaceSize(const int min_face_size) {
    this->min_face_size_ = min_face_size;
  }

  inline int GetDeviceID() {
    return gpu_id_;
  }

  inline int GetNeedMemory() {
    return need_memory_;
  }
 private:
  shared_ptr<Net<float> > net_;
  cv::Size input_geometry_;
  int num_channels_;
  int min_face_size_;
  float threshold_;
  int gpu_id_;
  int need_memory_;
  cv::Mat mean_;
};

#endif  // FACE_BOXES_H_

