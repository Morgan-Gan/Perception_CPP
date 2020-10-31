#ifndef FACE_ALIGNMENT_HPP_
#define FACE_ALIGNMENT_HPP_

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class  FaceAlignment {
public:
  FaceAlignment() :
    face_alignment_(0),gpu_id_(-1){
  }
  
  ~FaceAlignment(){
  }

  // initial model
  int Initial(const char* modelpath,int gpu_id);
	//void SetModeCpu();
	//void SetModeGpu(const int gpu_id);
  // release resource
  int Release();
  // alignment with detect box
  int Alignment(const cv::Mat &img, const cv::Rect &bbox, cv::Mat* landmarks);
  //get mean shape
  int GetMeanShape(cv::Mat *mean_landmarks);
  //get gpu_id ,if id>=0,mode=GPU,else mode=CPU
  const int GetDeviceID();
  //get model need memory
  const int GetNeedMemory();
private:
	void* face_alignment_;
        int gpu_id_;
};

#endif
