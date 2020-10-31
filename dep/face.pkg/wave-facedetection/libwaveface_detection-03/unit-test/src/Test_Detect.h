#include <iostream>
#include "face_detection.h"
#include "helpers.h"
#include "face_identification.hpp"
#include <opencv2/core/core.hpp>    
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>

#include "gtest/gtest.h"
using std::string;

class TestFuns{
public:
TestFuns() {}
	int TestWaveFace_Detect(FaceDetection &fd,string imagepath);
	
};

// TestCase事件-----测试初始化函数
class WaveFaceInitTest : public testing::Test
{
public:
	virtual void SetUp();
	virtual void TearDown();
	
	FaceDetection fd;
};


// TestSuit事件-----测试人间检测函数
class WaveFaceDetectTest : public testing::Test 
{
public:
	virtual void SetUp();
	virtual void TearDown();
	FaceDetection fd;
	TestFuns test_exect;
};



