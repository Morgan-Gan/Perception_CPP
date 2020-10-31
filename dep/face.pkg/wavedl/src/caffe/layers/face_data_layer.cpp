#include <opencv2/core/core.hpp>
#include <vector>
#include "caffe/layers/face_data_layer.hpp"
#include "caffe/data_transformer.hpp"

#include <math.h>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>


namespace caffe {

int FaceCrop::Rotate(const cv::Mat img_src, const cv::Mat shape_src,
    cv::Mat* img_dst, cv::Mat* shape_dst){
    int ladmarks_num = shape_src.cols / 2;
    cv::Point2f le, re, nose, lm, rm;
    if (ladmarks_num == 68){
      le.x = (shape_src.at<float>(0, 72) + shape_src.at<float>(0, 74) + shape_src.at<float>(0, 76) + shape_src.at<float>(0, 78) + shape_src.at<float>(0, 80) + shape_src.at<float>(0, 82)) / 6.0;
      re.x = (shape_src.at<float>(0, 84) + shape_src.at<float>(0, 86) + shape_src.at<float>(0, 88) + shape_src.at<float>(0, 90) + shape_src.at<float>(0, 92) + shape_src.at<float>(0, 94)) / 6.0;
      nose.x = shape_src.at<float>(0, 60);
      lm.x = shape_src.at<float>(0, 96);
      rm.x = shape_src.at<float>(0, 108);
      le.y = (shape_src.at<float>(0, 73) + shape_src.at<float>(0, 75) + shape_src.at<float>(0, 77) + shape_src.at<float>(0, 79) + shape_src.at<float>(0, 81) + shape_src.at<float>(0, 83)) / 6.0;
      re.y = (shape_src.at<float>(0, 85) + shape_src.at<float>(0, 87) + shape_src.at<float>(0, 89) + shape_src.at<float>(0, 91) + shape_src.at<float>(0, 93) + shape_src.at<float>(0, 95)) / 6.0;
      nose.y = shape_src.at<float>(0, 61);
      lm.y = shape_src.at<float>(0, 97);
      rm.y = shape_src.at<float>(0, 109);
    }else if (ladmarks_num == 5){
      le.x = shape_src.at<float>(0,0);
      re.x = shape_src.at<float>(0,2);
      nose.x = shape_src.at<float>(0,4);
      lm.x = shape_src.at<float>(0,6);
      rm.x = shape_src.at<float>(0,8);
      le.y = shape_src.at<float>(0,1);
      re.y = shape_src.at<float>(0,3);
      nose.y = shape_src.at<float>(0,5);
      lm.y = shape_src.at<float>(0,7);
      rm.y = shape_src.at<float>(0,9);
    }

    float centor_x = (le.x + re.x + nose.x + lm.x + rm.x) / 5;
    float centor_y = (le.y + re.y + nose.y + lm.y + rm.y) / 5;

    //float centor_x = img_src.cols / 2.0;
    //float centor_y = img_src.rows / 2.0;

    cv::Point2f pt_centor(centor_x, centor_y);
    double theta = atan((re.y - le.y) / (re.x - le.x));
    //int row_dst = img_src.rows * cos(fabs(theta)) + img_src.cols * sin(fabs(theta));
    //int col_dst = img_src.rows * sin(fabs(theta)) + img_src.cols * cos(fabs(theta));
    double angle = theta * 180 / 3.141592654;

    cv::Mat rotate_mat = cv::getRotationMatrix2D(pt_centor, angle, 1.0);

    cv::Mat shape_src_mat = cv::Mat::ones(3, ladmarks_num, CV_64FC1);
    for (int i = 0; i < ladmarks_num; ++i){
      shape_src_mat.at<double>(0, i) = shape_src.at<float>(0, 2*i);
      shape_src_mat.at<double>(1, i) = shape_src.at<float>(0, 2*i+1);
    }
    cv::Mat shape_dst_mat = cv::Mat::ones(2, ladmarks_num, CV_64FC1);
    shape_dst_mat = rotate_mat * shape_src_mat;
    shape_dst->create(1, ladmarks_num*2, CV_32FC1);
    for (int i = 0; i < ladmarks_num; ++i){
      //std::cout <<  shape_dst_mat.at<double>(0, i) << " " << shape_dst_mat.at<double>(1, i) << std::endl;
      //shape_dst->at<float>(0, 2*i) = shape_dst_mat.at<double>(0, i) - centor_x + col_dst / 2.0;
      //shape_dst->at<float>(0, 2*i+1) = shape_dst_mat.at<double>(1, i) - centor_y + row_dst / 2.0;
      shape_dst->at<float>(0, 2*i) = shape_dst_mat.at<double>(0, i);
      shape_dst->at<float>(0, 2*i+1) = shape_dst_mat.at<double>(1, i);
    }
    cv::warpAffine(img_src, *img_dst, rotate_mat, cv::Size(img_src.cols, img_src.rows));
    //std::cout << img_dst->rows << " " << img_dst->cols << std::endl;

    return 0;
  }

  int FaceCrop::ShiYuanCrop(const cv::Mat img_src, const cv::Mat shape_src, const cv::Size size_dst,
    cv::Mat* img_dst){
    cv::Point2f le, re, nose, lm, rm;
    if (shape_src.cols == 136){
      le.x = (shape_src.at<float>(0, 72) + shape_src.at<float>(0, 74) + shape_src.at<float>(0, 76) + shape_src.at<float>(0, 78) + shape_src.at<float>(0, 80) + shape_src.at<float>(0, 82)) / 6.0;
      re.x = (shape_src.at<float>(0, 84) + shape_src.at<float>(0, 86) + shape_src.at<float>(0, 88) + shape_src.at<float>(0, 90) + shape_src.at<float>(0, 92) + shape_src.at<float>(0, 94)) / 6.0;
      nose.x = shape_src.at<float>(0, 60);
      lm.x = shape_src.at<float>(0, 96);
      rm.x = shape_src.at<float>(0, 108);
      le.y = (shape_src.at<float>(0, 73) + shape_src.at<float>(0, 75) + shape_src.at<float>(0, 77) + shape_src.at<float>(0, 79) + shape_src.at<float>(0, 81) + shape_src.at<float>(0, 83)) / 6.0;
      re.y = (shape_src.at<float>(0, 85) + shape_src.at<float>(0, 87) + shape_src.at<float>(0, 89) + shape_src.at<float>(0, 91) + shape_src.at<float>(0, 93) + shape_src.at<float>(0, 95)) / 6.0;
      nose.y = shape_src.at<float>(0, 61);
      lm.y = shape_src.at<float>(0, 97);
      rm.y = shape_src.at<float>(0, 109);
    }else if (shape_src.cols == 10){
      le.x = shape_src.at<float>(0,0);
      re.x = shape_src.at<float>(0,2);
      nose.x = shape_src.at<float>(0,4);
      lm.x = shape_src.at<float>(0,6);
      rm.x = shape_src.at<float>(0,8);
      le.y = shape_src.at<float>(0,1);
      re.y = shape_src.at<float>(0,3);
      nose.y = shape_src.at<float>(0,5);
      lm.y = shape_src.at<float>(0,7);
      rm.y = shape_src.at<float>(0,9);
    }
    
    float centor_x = (le.x + re.x + nose.x + lm.x + rm.x) / 5;
    float centor_y = (le.y + re.y + nose.y + lm.y + rm.y) / 5;

    double dis_eye = sqrt((re.y - le.y)*(re.y - le.y) + (re.x - le.x)*(re.x - le.x));
    double dis_em = sqrt(((re.y + le.y) / 2 - (rm.y + lm.y) / 2)*((re.y + le.y) / 2 - (rm.y + lm.y) / 2) + ((re.x + le.x) / 2 - (rm.x + lm.x) / 2)*((re.x + le.x) / 2 - (rm.x + lm.x) / 2));
    double dis = fmax(dis_eye, dis_em);

    int x_min = fmax(0, floor(centor_x - dis*1.3));
    int x_max = fmin(img_src.cols, ceil(centor_x + dis*1.3));
    int y_min = fmax(0, floor(centor_y - dis*1.3));
    int y_max = fmin(img_src.rows, ceil(centor_y + dis*1.3));

    cv::Mat img_crop = img_src(cv::Rect(x_min, y_min, x_max - x_min, y_max - y_min));
    cv::resize(img_crop, *img_dst, size_dst);
    return 0;
  }

  int FaceCrop::FullFace(const cv::Mat img_src, const cv::Mat shape_src, const cv::Size size_dst,
    cv::Mat* img_dst){
    int x_min = 10000;
    int y_min = 10000;
    int x_max = 0;
    int y_max = 0;

    for (int i = 0; i < shape_src.cols / 2; i++){
      float x = shape_src.at<float>(0, 2 * i);
      float y = shape_src.at<float>(0, 2 * i + 1);
      if (x < x_min){
        x_min = x;
      }
      if (x > x_max){
        x_max = x;
      }
      if (y < y_min){
        y_min = y;
      }
      if (y > y_max){
        y_max = y;
      }
    }
    float lambda_centor = 1.0 / 3.0;
    float centor_x = (x_min + x_max) / 2.0;
    float centor_y = y_min + lambda_centor * (y_max - y_min);
    float dis = fmax((y_max - y_min)*(1 - lambda_centor), (x_max - x_min) / 2.0);

    x_min = fmax(0, int(centor_x - dis));
    x_max = fmin(img_src.cols - 1, int(centor_x + dis));
    y_min = fmax(0, int(centor_y - dis));
    y_max = fmin(img_src.rows - 1, int(centor_y + dis));

    cv::Mat img_crop = img_src(cv::Rect(x_min, y_min, x_max - x_min, y_max - y_min));
    cv::resize(img_crop, *img_dst, size_dst);
    return 0;
  }

  int FaceCrop::CroppedFace(const cv::Mat img_src, const cv::Mat shape_src, const cv::Size size_dst,
    cv::Mat* img_dst){
    int x_min = 10000;
    int y_min = 10000;
    int x_max = 0;
    int y_max = 0;
  
    for (int i = 0; i < shape_src.cols / 2; i++){
      float x = shape_src.at<float>(0, 2 * i);
      float y = shape_src.at<float>(0, 2 * i + 1);
      if (x < x_min){
        x_min = x;
      }
      if (x > x_max){
        x_max = x;
      }
      if (y < y_min){
        y_min = y;
      }
      if (y > y_max){
        y_max = y;
      }
    }
    float lambda_centor = 1.0 / 2.0;
    float centor_x = (x_min + x_max) / 2.0;
    float centor_y = y_min + lambda_centor * (y_max - y_min);
    float dis = fmax((y_max - y_min)*(1 - lambda_centor), (x_max - x_min) / 2.0);
  
    x_min = fmax(0, int(centor_x - dis));
    x_max = fmin(img_src.cols - 1, int(centor_x + dis));
    y_min = fmax(0, int(centor_y - dis));
    y_max = fmin(img_src.rows - 1, int(centor_y + dis));
  
    cv::Mat img_crop = img_src(cv::Rect(x_min, y_min, x_max - x_min, y_max - y_min));
    cv::resize(img_crop, *img_dst, size_dst);
    return 0;
  }

  int FaceCrop::Eye(const cv::Mat img_src, const cv::Mat shape_src, const cv::Size size_dst,
    cv::Mat* img_dst){
    cv::Mat eye_shape = cv::Mat::zeros(1, 44, shape_src.type());
    for (int i = 0; i < 20; i++){
      eye_shape.at<float>(0, i) = shape_src.at<float>(0, i + 34);
    }
    for (int i = 20; i < 44; i++){
      eye_shape.at<float>(0, i) = shape_src.at<float>(0, i + 52);
    }
  
    int x_min = 10000;
    int y_min = 10000;
    int x_max = 0;
    int y_max = 0;
  
    for (int i = 0; i < eye_shape.cols / 2; i++){
      float x = eye_shape.at<float>(0, 2 * i);
      float y = eye_shape.at<float>(0, 2 * i + 1);
      if (x < x_min){
        x_min = x;
      }
      if (x > x_max){
        x_max = x;
      }
      if (y < y_min){
        y_min = y;
      }
      if (y > y_max){
        y_max = y;
      }
    }
    float lambda_centor = 1.0 / 2.0;
    float centor_x = (x_min + x_max) / 2.0;
    float centor_y = y_min + lambda_centor * (y_max - y_min);
    float dis = fmax((y_max - y_min)*(1 - lambda_centor), (x_max - x_min) / 2.0);
  
    x_min = fmax(0, int(centor_x - dis));
    x_max = fmin(img_src.cols - 1, int(centor_x + dis));
    y_min = fmax(0, int(centor_y - dis));
    y_max = fmin(img_src.rows - 1, int(centor_y + dis));
  
    cv::Mat img_crop = img_src(cv::Rect(x_min, y_min, x_max - x_min, y_max - y_min));
    cv::resize(img_crop, *img_dst, size_dst);
    return 0;
  }

  int FaceCrop::Nose(const cv::Mat img_src, const cv::Mat shape_src, const cv::Size size_dst,
    cv::Mat* img_dst){
    cv::Mat nose_shape = cv::Mat::zeros(1, 82, shape_src.type());
    for (int i = 0; i < 82; i++){
      nose_shape.at<float>(0, i) = shape_src.at<float>(0, i + 54);
    }
  
    int x_min = 10000;
    int y_min = 10000;
    int x_max = 0;
    int y_max = 0;
  
    for (int i = 0; i < nose_shape.cols / 2; i++){
      float x = nose_shape.at<float>(0, 2 * i);
      float y = nose_shape.at<float>(0, 2 * i + 1);
      if (x < x_min){
        x_min = x;
      }
      if (x > x_max){
        x_max = x;
      }
      if (y < y_min){
        y_min = y;
      }
      if (y > y_max){
        y_max = y;
      }
    }
    float lambda_centor = 1.0 / 2.0;
    float centor_x = (x_min + x_max) / 2.0;
    float centor_y = y_min + lambda_centor * (y_max - y_min);
    float dis = fmax((y_max - y_min)*(1 - lambda_centor), (x_max - x_min) / 2.0);
  
    x_min = fmax(0, int(centor_x - dis));
    x_max = fmin(img_src.cols - 1, int(centor_x + dis));
    y_min = fmax(0, int(centor_y - dis));
    y_max = fmin(img_src.rows - 1, int(centor_y + dis));
  
    cv::Mat img_crop = img_src(cv::Rect(x_min, y_min, x_max - x_min, y_max - y_min));
    cv::resize(img_crop, *img_dst, size_dst);
    return 0;
  }

  int FaceCrop::Mouth(const cv::Mat img_src, const cv::Mat shape_src, const cv::Size size_dst,
    cv::Mat* img_dst){
    cv::Mat mouth_shape = cv::Mat::zeros(1, 40, shape_src.type());
    for (int i = 0; i < 40; i++){
      mouth_shape.at<float>(0, i) = shape_src.at<float>(0, i + 96);
    }
  
    int x_min = 10000;
    int y_min = 10000;
    int x_max = 0;
    int y_max = 0;
  
    for (int i = 0; i < mouth_shape.cols / 2; i++){
      float x = mouth_shape.at<float>(0, 2 * i);
      float y = mouth_shape.at<float>(0, 2 * i + 1);
      if (x < x_min){
        x_min = x;
      }
      if (x > x_max){
        x_max = x;
      }
      if (y < y_min){
        y_min = y;
      }
      if (y > y_max){
        y_max = y;
      }
    }
    float lambda_centor = 1.0 / 2.0;
    float centor_x = (x_min + x_max) / 2.0;
    float centor_y = y_min + lambda_centor * (y_max - y_min);
    float dis = fmax((y_max - y_min)*(1 - lambda_centor), (x_max - x_min) / 2.0);
  
    x_min = fmax(0, int(centor_x - dis));
    x_max = fmin(img_src.cols - 1, int(centor_x + dis));
    y_min = fmax(0, int(centor_y - dis));
    y_max = fmin(img_src.rows - 1, int(centor_y + dis));
  
    cv::Mat img_crop = img_src(cv::Rect(x_min, y_min, x_max - x_min, y_max - y_min));
    cv::resize(img_crop, *img_dst, size_dst);
    return 0;
  }

  int FaceCrop::Refinement(const cv::Mat img_src, const cv::Mat landmarks, 
    const cv::Size size_dst, cv::Mat* img_dst){
    // Target image's width must be equal to height 
    if (size_dst.width == 0 || size_dst.height == 0) {
      std::cout << "Error: target image width can't be Zero" << std::endl;
      return -1;
    }
  
    cv::Mat landmarks_five_src = cv::Mat::zeros(5, 2, CV_32FC1);
    cv::Mat landmarks_five_dst = cv::Mat::zeros(5, 2, CV_32FC1);
    if (landmarks.cols == 136){
      landmarks_five_src.at<float>(0, 0) = (landmarks.at<float>(0, 72) + landmarks.at<float>(0, 74) + landmarks.at<float>(0, 76) + landmarks.at<float>(0, 78) + landmarks.at<float>(0, 80) + landmarks.at<float>(0, 82)) / 6.0;
      landmarks_five_src.at<float>(0, 1) = (landmarks.at<float>(0, 73) + landmarks.at<float>(0, 75) + landmarks.at<float>(0, 77) + landmarks.at<float>(0, 79) + landmarks.at<float>(0, 81) + landmarks.at<float>(0, 83)) / 6.0;
      landmarks_five_src.at<float>(1, 0) = (landmarks.at<float>(0, 84) + landmarks.at<float>(0, 86) + landmarks.at<float>(0, 88) + landmarks.at<float>(0, 90) + landmarks.at<float>(0, 92) + landmarks.at<float>(0, 94)) / 6.0;
      landmarks_five_src.at<float>(1, 1) = (landmarks.at<float>(0, 85) + landmarks.at<float>(0, 87) + landmarks.at<float>(0, 89) + landmarks.at<float>(0, 91) + landmarks.at<float>(0, 93) + landmarks.at<float>(0, 95)) / 6.0;
      landmarks_five_src.at<float>(2, 0) = landmarks.at<float>(0, 60);
      landmarks_five_src.at<float>(2, 1) = landmarks.at<float>(0, 61);
      landmarks_five_src.at<float>(3, 0) = landmarks.at<float>(0, 96);
      landmarks_five_src.at<float>(3, 1) = landmarks.at<float>(0, 97);
      landmarks_five_src.at<float>(4, 0) = landmarks.at<float>(0, 108);
      landmarks_five_src.at<float>(4, 1) = landmarks.at<float>(0, 109);
    }
    else if (landmarks.cols == 10){
      landmarks_five_src.at<float>(0, 0) = landmarks.at<float>(0, 0);
      landmarks_five_src.at<float>(0, 1) = landmarks.at<float>(0, 1);
      landmarks_five_src.at<float>(1, 0) = landmarks.at<float>(0, 2);
      landmarks_five_src.at<float>(1, 1) = landmarks.at<float>(0, 3);
      landmarks_five_src.at<float>(2, 0) = landmarks.at<float>(0, 4);
      landmarks_five_src.at<float>(2, 1) = landmarks.at<float>(0, 5);
      landmarks_five_src.at<float>(3, 0) = landmarks.at<float>(0, 6);
      landmarks_five_src.at<float>(3, 1) = landmarks.at<float>(0, 7);
      landmarks_five_src.at<float>(4, 0) = landmarks.at<float>(0, 8);
      landmarks_five_src.at<float>(4, 1) = landmarks.at<float>(0, 9);
    }
    else {
      std::cout << "Error: the number of landmarks must be 5 or 68" << std::endl;
      return -2;
    }
  
    // Scaling the template when image dst width is not equal to 96
    float scale = 1.0;
    if ( 96 != size_dst.width){
      scale = size_dst.width / 96.0;
    }
  
    landmarks_five_dst.at<float>(0, 0) = 30 * scale;
    landmarks_five_dst.at<float>(0, 1) = 46 * scale;
    landmarks_five_dst.at<float>(1, 0) = 65 * scale;
    landmarks_five_dst.at<float>(1, 1) = 46 * scale;
    landmarks_five_dst.at<float>(2, 0) = 48 * scale;
    landmarks_five_dst.at<float>(2, 1) = 66 * scale;
    landmarks_five_dst.at<float>(3, 0) = 33 * scale;
    landmarks_five_dst.at<float>(3, 1) = 87 * scale;
    landmarks_five_dst.at<float>(4, 0) = 62 * scale;
    landmarks_five_dst.at<float>(4, 1) = 87 * scale;
  
    // Solving the similarity transformation parameter using the least square method 
    //     by invoking opencv cv::sove and cv::warpAffine
    //     Ax = B (x has 4 parameters corresponding to 4 degree of similarity transformation)
    cv::Mat A = cv::Mat::zeros(10, 4, CV_32FC1);
    cv::Mat B = cv::Mat::zeros(10, 1, CV_32FC1);
    for (int i = 0; i < 5; ++i){
      A.at<float>(2 * i, 0) = landmarks_five_src.at<float>(i, 0);
      A.at<float>(2 * i, 1) = -landmarks_five_src.at<float>(i, 1);
      A.at<float>(2 * i, 2) = 1;
      A.at<float>(2 * i, 3) = 0;
      A.at<float>(2 * i + 1, 0) = landmarks_five_src.at<float>(i, 1);
      A.at<float>(2 * i + 1, 1) = landmarks_five_src.at<float>(i, 0);
      A.at<float>(2 * i + 1, 2) = 0;
      A.at<float>(2 * i + 1, 3) = 1;
      B.at<float>(2 * i, 0) = landmarks_five_dst.at<float>(i, 0);
      B.at<float>(2 * i + 1, 0) = landmarks_five_dst.at<float>(i, 1);
    }
    cv::Mat X = cv::Mat::zeros(4, 1, CV_32FC1);
    cv::solve(A, B, X, cv::DECOMP_SVD);
    cv::Mat M = cv::Mat::zeros(2, 3, CV_32FC1);
    M.at<float>(0, 0) = X.at<float>(0, 0);
    M.at<float>(0, 1) = -X.at<float>(1, 0);
    M.at<float>(0, 2) = X.at<float>(2, 0);
    M.at<float>(1, 0) = X.at<float>(1, 0);
    M.at<float>(1, 1) = X.at<float>(0, 0);
    M.at<float>(1, 2) = X.at<float>(3, 0);
  
    cv::warpAffine(img_src, *img_dst, M, size_dst);

    return 0;
  }


//Face Data Layer
template <typename Dtype>
void FaceDataLayer<Dtype>::LayerSetUp(const vector<Blob<Dtype>*>& bottom,
     const vector<Blob<Dtype>*>& top) {
  const int num_top = top.size();
  const FaceDataParameter& param = this->layer_param_.face_data_param();
  const int num_shape = param.shape_size();
  //for (int i = 0; i < param.shape(0).dim_size(); ++i){
	//  std::cout << param.shape(0).dim(i) << std::endl;
  //}
  if (num_shape > 0) {
	  num_des_ = int(param.shape(0).dim(0));
	  channels_des_ = int(param.shape(0).dim(1));
	  height_des_ = int(param.shape(0).dim(2));
	  width_des_ = int(param.shape(0).dim(3));
	  for (int i = 0; i < num_top; ++i) {
		  top[i]->Reshape(param.shape(0));
	  }
  }
  has_fullface_ = param.fullface();
  if (has_fullface_) {
    data_fullface_.Reshape(param.shape(0));
  }
  has_croppedface_ = param.croppedface();
  if (has_croppedface_) {
    data_croppedface_.Reshape(param.shape(0));
  }
  has_eye_ = param.eye();
  if (has_eye_) {
    data_eye_.Reshape(param.shape(0));
  }
  has_nose_ = param.nose();
  if (has_nose_) {
    data_nose_.Reshape(param.shape(0));
  }
  has_mouth_ = param.mouth();
  if (has_mouth_) {
    data_mouth_.Reshape(param.shape(0));
  }

  data_transformer_.reset(new DataTransformer<Dtype>(transform_param_, caffe::TEST));
}

template <typename Dtype>
void FaceDataLayer<Dtype>::Reshape(const vector<Blob<Dtype>*>& bottom,
     const vector<Blob<Dtype>*>& top) {
  const int num_top = top.size();
  for (int i = 0; i < num_top; ++i) {
      top[i]->Reshape(num_des_, channels_des_, height_des_, width_des_);
    }

}

template <typename Dtype>
void FaceDataLayer<Dtype>::AddMatVector(const vector<cv::Mat>& mat_vector,
      const vector<cv::Mat>& shape_vector) {
  if (mat_vector.size() != num_des_){
    std::cout << "the size of vector must be equal to num_des_" << std::endl;
    return;
  }
  std::vector<cv::Mat> img_vector_rotate;
  std::vector<cv::Mat> shape_vector_rotate;
  //std::cout << " rotate " << std::endl;
  for (int i =0; i< num_des_;++i) {
    cv::Mat img_rotate;
    cv::Mat shape_rotate;
    face_crop_.Rotate(mat_vector[i], shape_vector[i], &img_rotate, &shape_rotate);
    img_vector_rotate.push_back(img_rotate);
    shape_vector_rotate.push_back(shape_rotate);
    //show results
    //cv::Mat img_rotate_copy = img_rotate.clone();
    //for (int j = 0; j < 136; j=j+2) {
    //  //std::cout << shape_vector[i].at<float>(0, j) << " " << shape_vector[i].at<float>(0, j+1) << std::endl;
    //  cv::circle(img_rotate_copy, cv::Point2f(shape_rotate.at<float>(0, j), shape_rotate.at<float>(0, j+1)), 1, cv::Scalar(255, 255, 0), 1);
    //}
    //cv::imshow("windows", img_rotate_copy);
    //cv::waitKey();
    //cv::imshow("windows", mat_vector[i]);
    //cv::waitKey();
    }
  //std::cout << " crop " << std::endl;
  if (has_fullface_) {
    vector<cv::Mat> mat_fullface;
    mat_fullface.clear();
    for (int i =0; i< num_des_;++i){
      cv::Mat img_fullface;
      if (has_eye_) {
        face_crop_.ShiYuanCrop(img_vector_rotate[i], shape_vector_rotate[i], cv::Size(width_des_, height_des_), &img_fullface);
      }else{
        face_crop_.Refinement(mat_vector[i], shape_vector[i], cv::Size(width_des_, height_des_), &img_fullface);
      }
      
      mat_fullface.push_back(img_fullface);
	    //cv::imshow("img_fullface", img_fullface);
	    //cv::waitKey();
    }
    data_fullface_.Reshape(num_des_, channels_des_, height_des_, width_des_);
    data_transformer_->Transform(mat_fullface, &data_fullface_);
  }
  if (has_croppedface_) {
    vector<cv::Mat> mat_croppedface;
    mat_croppedface.clear();
    for (int i =0; i< num_des_;++i){
      cv::Mat img_croppedface;
      face_crop_.CroppedFace(img_vector_rotate[i], shape_vector_rotate[i], cv::Size(width_des_, height_des_), &img_croppedface);
      mat_croppedface.push_back(img_croppedface);
	    //cv::imshow("img_croppedface", img_croppedface);
	    //cv::waitKey();
    }
    data_croppedface_.Reshape(num_des_, channels_des_, height_des_, width_des_);
    data_transformer_->Transform(mat_croppedface, &data_croppedface_);
  }
  if (has_eye_) {
    vector<cv::Mat> mat_eye;
    mat_eye.clear();
    for (int i =0; i< num_des_;++i){
      cv::Mat img_eye;
      face_crop_.Eye(img_vector_rotate[i], shape_vector_rotate[i], cv::Size(width_des_, height_des_), &img_eye);
      mat_eye.push_back(img_eye);
	    //cv::imshow("img_eye", img_eye);
	    //cv::waitKey();
    }
    data_eye_.Reshape(num_des_, channels_des_, height_des_, width_des_);
    data_transformer_->Transform(mat_eye, &data_eye_);
  }
  if (has_nose_) {
    vector<cv::Mat> mat_nose;
    mat_nose.clear();
    for (int i =0; i< num_des_;++i){
      cv::Mat img_nose;
      face_crop_.Nose(img_vector_rotate[i], shape_vector_rotate[i], cv::Size(width_des_, height_des_), &img_nose);
      mat_nose.push_back(img_nose);
	    //cv::imshow("img_nose", img_nose);
	    //cv::waitKey();
    }
    data_nose_.Reshape(num_des_, channels_des_, height_des_, width_des_);
    data_transformer_->Transform(mat_nose, &data_nose_);
  }
  if (has_mouth_) {
    vector<cv::Mat> mat_mouth;
    mat_mouth.clear();
    for (int i =0; i< num_des_;++i){
      cv::Mat img_mouth;
      face_crop_.Mouth(img_vector_rotate[i], shape_vector_rotate[i], cv::Size(width_des_, height_des_), &img_mouth);
      mat_mouth.push_back(img_mouth);
	    //cv::imshow("img_mouth", img_mouth);
	    //cv::waitKey();
    }
    data_mouth_.Reshape(num_des_, channels_des_, height_des_, width_des_);
    data_transformer_->Transform(mat_mouth, &data_mouth_);
  }
}

template <typename Dtype>
void FaceDataLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top) {
  int top_index = 0;
  if (has_fullface_){
    top[top_index]->ShareData(data_fullface_);
    top_index ++;
  }
  if (has_croppedface_){
    top[top_index]->ShareData(data_croppedface_);
    top_index ++;
  }
  if (has_eye_){
    top[top_index]->ShareData(data_eye_);
    top_index ++;
  }
  if (has_nose_){
    top[top_index]->ShareData(data_nose_);
    top_index ++;
  }
  if (has_mouth_){
    top[top_index]->ShareData(data_mouth_);
    top_index ++;
  }
}

INSTANTIATE_CLASS(FaceDataLayer);
REGISTER_LAYER_CLASS(FaceData);

}  // namespace caffe
