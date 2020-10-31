// Face detection interface for DynamicFaceRecognition project
//
// Version:    1.0.0
// Code Style: Google C++ style
// Author:     zhijinlin
// Reviewer:   zhijinlin
// Date:       20190327
// All rights reserved @wavewisdom-bj

#include "face_detection.h"

#include "face_detection_cnn.h"

FaceDetection::FaceDetection() {
  detector = NULL;
  detector = new FaceDetector();
}

FaceDetection::~FaceDetection() {}

int FaceDetection::Initial(const char* model_path,
              const int& gpu_id,
              const int& min_face_size,
              const int& max_face_size,
              const float& score_thresh_1,
              const float& score_thresh_2,
              const float& score_thresh_3) {
  if (NULL == detector) {
    return -1;
  }
  int ret = -1;
  ret = ((FaceDetector*)detector)->Initial(model_path, min_face_size,
    score_thresh_3, gpu_id);
  return ret;
}

int FaceDetection::Configure(const int &min_face_size,
                const int &max_face_size,
                const float &score_thresh_1,
                const float &score_thresh_2,
                const float &score_thresh_3) {
  if (NULL == detector) {
    return -1;
  }
  ((FaceDetector*)detector)->SetMinFaceSize(min_face_size);
  ((FaceDetector*)detector)->SetThreshold(score_thresh_3);
  return 0;
}

int FaceDetection::Run(const cv::Mat &src_img,
          std::vector<FaceRectInfo>* face_rects) {
  if (NULL == detector) {
    return -1;
  }
  std::vector<Face> face_infos;
  face_infos = ((FaceDetector*)detector)->Detect(src_img);
  if (face_infos.size() <= 0) {
    return 0;
  }
  std::vector<FaceRectInfo> empty_vect;
  face_rects->swap(empty_vect);
  for (int i = 0; i < face_infos.size(); ++i) {
    FaceRectInfo face_item;
    face_item.bbox.x = face_infos[i].bbox.x1;
    face_item.bbox.y = face_infos[i].bbox.y1 +
                  (face_infos[i].bbox.y2-face_infos[i].bbox.y1) -
                  (face_infos[i].bbox.x2-face_infos[i].bbox.x1);
    face_item.bbox.width = face_infos[i].bbox.x2 - face_infos[i].bbox.x1;
    face_item.bbox.height = face_item.bbox.width;
    face_item.score = face_infos[i].score;
    face_rects->push_back(face_item);
  }
  return 0;
}

int FaceDetection::Release() {
  if (NULL == detector) {
    return -1;
  }
  return ((FaceDetector*)detector)->Release();
}


int FaceDetection::GetDeviceID() {
  if (NULL == detector) {
    return -1;
  }
  return ((FaceDetector*)detector)->GetDeviceID();
}


const int FaceDetection::GetNeedMemory() const {
  if (NULL == detector) {
    return -1;
  }
  return ((FaceDetector*)detector)->GetNeedMemory();
}


