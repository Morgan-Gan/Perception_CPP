#include <cstring>
#include <fstream>
#include <iostream>
#include <chrono>
#include <time.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "face_quality.h"
#include <glog/logging.h>
using namespace std;

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

int main(int argc, char* argv[])
{
  google::InitGoogleLogging(argv[0]);
  FaceQuality face_quality;
  std::cout<<"111st"<<std::endl;
  face_quality.Initial("./models/");

  face_quality.Configure();

    
  cv::Mat img = cv::imread("data/face_quality_test.jpg");
  if (img.empty()) {
    std::cout << " Input Image is empty" << std::endl;
    return -1;
  }
  if (img.empty() == false) {
    cv::Mat shape_src(1, 136, CV_32FC1);
    ifstream fin;
    fin.open("data/face_quality_test.txt", ios::in);
    string line;
    float x = 0.0, y = 0.0;
    for (int i = 0; i < 68 && getline(fin, line); ++i) {
      istringstream instr(line);
      instr >> i;
      instr >> x;
      instr >> y;
      shape_src.at<float>(0, 2 * i) = x;
      shape_src.at<float>(0, 2 * i + 1) = y;
    }

    high_resolution_clock::time_point beginTime = high_resolution_clock::now();

    int label = -1;
    int re_quality = face_quality.Run(img, shape_src, &label);

    high_resolution_clock::time_point endTime = high_resolution_clock::now();
    milliseconds timeInterval = std::chrono::duration_cast<milliseconds>(endTime - beginTime);
    cout << "time: " << timeInterval.count() << "ms\n";
    std::cout << "label:" << label << std::endl;

  }

  face_quality.Release();
  return 0;
}