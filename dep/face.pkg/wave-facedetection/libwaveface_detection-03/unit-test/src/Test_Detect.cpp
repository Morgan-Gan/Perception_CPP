#include "Test_Detect.h"

int TestFuns::TestWaveFace_Detect(FaceDetection &fd,string imagepath)
	{
		std::vector<FaceRectInfo> faces;
		cv::Mat img = cv::imread(imagepath);
		int ret = fd.Run(img, &faces);
		if(ret == -1)
			return -1;
		if(faces.size() == 0)
			return -1;
		
		return 0;
	}



// TestCase事件-----测试初始化函数
void WaveFaceInitTest :: SetUp() 
	{
		std::cout << "WaveID Test Init Test SetUP" << std::endl;
	}

void  WaveFaceInitTest :: TearDown()
	{
		std::cout << "WaveID Test Init Test TearDown" << std::endl;
	}
	


// TestSuit事件-----测试人脸检测函数
void WaveFaceDetectTest:: SetUp()
	{
		std::cout << "WaveID Test ExtractFeature  Test SetUP" << std::endl;	
	}
void WaveFaceDetectTest:: TearDown() 
	{
		std::cout << "WaveID Test ExtractFeature  Test TearDown" << std::endl;
	}
	



