#ifndef CAFFE_FACE_DATA_LAYER_HPP_
#define CAFFE_FACE_DATA_LAYER_HPP_

#include <vector>

#include "caffe/blob.hpp"
#include "caffe/layer.hpp"
#include "caffe/data_transformer.hpp"
#include "caffe/proto/caffe.pb.h"

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

namespace caffe {

template <class Type> 
class Point_face{
public:
  Type x;
  Type y;
  Point_face(Type x_, Type y_){
    x = x_;
    y = y_;
  }
  Point_face(){
    x = 0;
    y = 0;
  }
};

class FaceCrop {
public:
  FaceCrop() {}
  ~FaceCrop() {}
  int Rotate(const cv::Mat img_src, const cv::Mat shape_src, 
    cv::Mat* img_dst, cv::Mat* shape_dst);
  int ShiYuanCrop(const cv::Mat img_src, const cv::Mat shape_src, const cv::Size size_dst,
    cv::Mat* img_dst);
  int FullFace(const cv::Mat img_src, const cv::Mat shape_src, const cv::Size size_dst,
    cv::Mat* img_dst);
  int CroppedFace(const cv::Mat img_src, const cv::Mat shape_src, const cv::Size size_dst,
    cv::Mat* img_dst);
  int Eye(const cv::Mat img_src, const cv::Mat shape_src, const cv::Size size_dst,
    cv::Mat* img_dst);
  int Nose(const cv::Mat img_src, const cv::Mat shape_src, const cv::Size size_dst,
    cv::Mat* img_dst);
  int Mouth(const cv::Mat img_src, const cv::Mat shape_src, const cv::Size size_dst,
    cv::Mat* img_dst);
  int Refinement(const cv::Mat img_src, const cv::Mat shape_src, const cv::Size size_dst, 
    cv::Mat* img_dst);
};

template <typename Dtype>
class FaceDataLayer : public Layer<Dtype> {
 public:
  explicit FaceDataLayer(const LayerParameter& param)
      : Layer<Dtype>(param), transform_param_(param.transform_param()) {}

  virtual void LayerSetUp(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top);
  virtual void Reshape(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top);
  virtual inline const char* type() const { return "FaceData"; }
  virtual inline int ExactNumBottomBlobs() const { return 0; }
  virtual inline int MinTopBlobs() const { return 1; }
  
  void SetBatchSize(int bs) { num_des_ = bs; }
  int GetImgH() { return height_des_; }
  int GetImgW() { return width_des_; }

  void AddMatVector(const std::vector<cv::Mat>& mat_vector,
      const std::vector<cv::Mat>& shape_vector);

 protected:
  virtual void Forward_cpu(const vector<Blob<Dtype>*>& bottom,const vector<Blob<Dtype>*>& top);
  virtual void Backward_cpu(const vector<Blob<Dtype>*>& top,
	  const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) {}
  Blob<Dtype> data_fullface_;
  Blob<Dtype> data_croppedface_;
  Blob<Dtype> data_eye_;
  Blob<Dtype> data_nose_;
  Blob<Dtype> data_mouth_;
  bool has_fullface_;
  bool has_croppedface_;
  bool has_eye_;
  bool has_nose_;
  bool has_mouth_;
  int num_des_;
  int channels_des_;
  int height_des_;
  int width_des_;
  TransformationParameter transform_param_;
  shared_ptr<DataTransformer<Dtype> > data_transformer_;
  FaceCrop face_crop_;
};

}  // namespace caffe

#endif  // CAFFE_MEMORY_DATA_LAYER_HPP_
