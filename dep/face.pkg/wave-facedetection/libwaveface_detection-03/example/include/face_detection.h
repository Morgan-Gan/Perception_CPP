// Face detection interface for DynamicFaceRecognition project
//
// Version:    1.0.0
// Code Style: Google C++ style
// Author:     wanghanchao
// Reviewer:   zhijinlin
// Date:       20181025
// All rights reserved @wavewisdom-bj

#ifndef FACE_DETECTION_HPP_
#define FACE_DETECTION_HPP_

#include <opencv2/opencv.hpp>

/**
 * @brief Face detection box.
 */
struct FaceRect {
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
    : x(x_), y(y_), width(width_), height(height_) {}

  FaceRect & operator = (const FaceRect &other) {
    x = other.x;
    y = other.y;
    width = other.width;
    height = other.height;
    return *this;
  }

  FaceRect(const cv::Rect &rect)
   : x(rect.x), y(rect.y), width(rect.width), height(rect.height) {}
    FaceRect & operator=(const cv::Rect &other){
    x = other.x;
    y = other.y;
    width = other.width;
    height = other.height;
    return *this;
  }

  cv::Rect GetRect() {
    return cv::Rect(x, y, width, height);
  }

  const cv::Rect GetRect() const {
    return cv::Rect(x, y, width, height);
  }
};

/**
 * @brief Face detection information.
 */
struct FaceRectInfo {
  // Face box of detected.
  FaceRect bbox;

  // Confidence of the detected face.
  float score;

  cv::Mat landmarks;
  FaceRectInfo(): bbox(), score(0.0f) {}

  FaceRectInfo(const FaceRect &face_rects,
               const float &score_,
               const cv::Mat &landmarks_): bbox(face_rects), score(score_) {
    landmarks=landmarks_.clone();
  }

  FaceRectInfo& operator = (const FaceRectInfo & other) {
    bbox = other.bbox;
    score = other.score;
    landmarks=other.landmarks.clone();
    return *this;
  }
};

/**
 * @brief Face detection
 */
class FaceDetection {
 public:
  FaceDetection();
  ~FaceDetection();

  /**
   * @brief Initialize algorithm.
   *
   * @param model_path input the path of the model.
   * @param gpu_id input specified gpu device id, default is -1.
   * @param min_face_size Input the mimimum face detected, default is 80.
   * @param max_face_size input the maximum face detected, default is 300.
   * @param score_thresh_1 input stage 1 threshold, default is 0.7.
   * @param score_thresh_2 input stage 2 threshold, default is 0.7.
   * @param score_thresh_3 input stage 3 threshold, default is 0.7.
   * @return 0 is successful, otherwise is failed.
   */
  int Initial(const char* model_path,
              const int& gpu_id = -1,
              const int& min_face_size = 80,
              const int& max_face_size = 300,
              const float& score_thresh_1 = 0.7,
              const float& score_thresh_2 = 0.7,
              const float& score_thresh_3 = 0.7);

  /**
   * @brief Configure the parameters of algorithm.
   *
   * @param min_face_size input the mimimum face detected, default is 80.
   * @param max_face_size input the maximum face detected, default is 300.
   * @param score_thresh_1 input stage 1 threshold, default is 0.7.
   * @param score_thresh_2 input stage 2 threshold, default is 0.7.
   * @param score_thresh_3 input stage 3 threshold, default is 0.7.
   * @return 0 is successful, otherwise is failed.
   */
  int Configure(const int &min_face_size = 80,
                const int &max_face_size = 300,
                const float &score_thresh_1 = 0.7,
                const float &score_thresh_2 = 0.7,
                const float &score_thresh_3 = 0.7);

  /**
   * @brief Perform face detection.
   *
   * @param src_img input a image.
   * @param face_rects output detected information.
   * @return 0 is successful, otherwise is failed.
   */
  int Run(const cv::Mat &src_img,
          std::vector<FaceRectInfo>* face_rects);

  /**
   * @brief Release the resource.
   *
   * @return 0 is successful, otherwise is failed.
   */
  int Release();

  /**
   * @brief Get the id of the gpu device in use.
   *
   * @return the id of the gpu device in use, -1 represents CPU only mode,
             negative number represents GPU device id.
   */
  int GetDeviceID();

  /**
   * @brief Get the amount of memory the current algorithm needs to use.
   *
   * @return the amount of memory (Unit: MB).
   */
  const int GetNeedMemory() const;

 private:
  void* detector;
};

#endif  //FACE_DETECTION_HPP_

