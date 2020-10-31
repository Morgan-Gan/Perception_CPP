#include <iostream>
#include "face_detection.h"
#include "helpers.h"
#include <thread>
#include "face_identification.hpp"


class ThreadGuard {
  std::thread& t_;
public:
  explicit ThreadGuard(std::thread &t): t_(t) {}
  ~ThreadGuard() {
    if (t_.joinable()) {
      t_.join();
    }
  }
  ThreadGuard(ThreadGuard const &) = delete;
  ThreadGuard& operator = (ThreadGuard const &) = delete;
};

void ThreadFun(int nn, char* image_path1, char* image_path2) {
  int count = 0;
  const int RUN_TIMES = 10;
  const int DETECT_TIMES = 100;
	FaceDetection fd;
  fd.Initial("./models/", 1, 40, 1000, 0.7, 0.7, 0.7);
  std::vector<FaceRectInfo> faces;
  while (count < RUN_TIMES) {
    ++count;
    std::cout << nn << "---->  count: " << count << std::endl;
	  Timer timer;
    std::cout << nn << "---->  " << fd.GetDeviceID() << std::endl;
	  timer.Start();
    for (int j = 0; j < DETECT_TIMES; ++j) {
      cv::Mat img;
      if (j % 2 == 0) {
        img = cv::imread(image_path1);
      } else {
        img = cv::imread(image_path2);
      }
//      fd.Configure(80, 1000, 0.7, 0.7, 0.7);
      fd.Run(img, &faces);
      std::cout << "face size: " << faces.size() << std::endl;
    }
	  std::cout << nn << "---->Elapsed time (ms): "
        << timer.Stop() / float(DETECT_TIMES) << std::endl;
  }
  cv::Mat img = cv::imread(image_path1);
  for (int i = 0; i < faces.size(); ++i) {
    int x = faces[i].bbox.x;
    int y = faces[i].bbox.y;
    int width = faces[i].bbox.width;
    int height = faces[i].bbox.height;
    cv::rectangle(img, cv::Rect(x, y, width, height), cv::Scalar(0, 255, 0), 2);
  }
  cv::imwrite("result.jpg", img);
  fd.Release();
}


int main(int argc, char** argv) {

//  ::google::InitGoogleLogging("");
  ThreadFun(1, argv[1], argv[2]);




  // std::thread t1(ThreadFun, 0);
  // std::thread t2(ThreadFun, 1);
  // 
  // std::thread t3(ThreadFun, 2);
  // std::thread t4(ThreadFun, 3);
  // std::thread t5(ThreadFun, 4);
  // std::thread t6(ThreadFun, 5);
  // std::thread t7(ThreadFun, 6);
  // std::thread t8(ThreadFun, 7);

  // // t3.detach();
  // // t4.detach();
  // // t5.detach();
  // // t6.detach();
  // // t7.detach();
  // // t8.join();

  // // ThreadGuard tg1(t1);
  // ThreadGuard tg2(t2);
  // ThreadGuard tg3(t3);
  // ThreadGuard tg4(t4);
  // ThreadGuard tg5(t5);
  // ThreadGuard tg6(t6);
  // ThreadGuard tg7(t7);
  // ThreadGuard tg8(t8);
  
  // 参数依次：创建的线程ID, 线程参数，调用函数，传入的参数
  // pthread_t t_id1;
  // pthread_t t_id2;
  // int indexes[2];
  // indexes[0] = 0;
  // indexes[1] = 1;
  // int ret = pthread_create(&t_id1, NULL, ThreadFun, (void *)&(indexes[0]));
  // if (ret != 0) {
  //   std::cout << "pthread_create error: error_code = " << ret << std::endl;
  // }
  // ret = pthread_create(&t_id2, NULL, ThreadFun, (void *)&(indexes[1]));
  // if (ret != 0) {
  //   std::cout << "pthread_create error: error_code = " << ret << std::endl;
  // }
  // pthread_exit(NULL);
  return 0;
}
