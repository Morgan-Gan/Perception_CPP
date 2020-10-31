#include "face_identification.hpp"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <opencv2/core/core.hpp>    
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include "gtest/gtest.h"
using std::string;

class TestFuns{
public:
TestFuns() {}
	int TestWaveId_Exet(WaveID &obj,string imagepath);
	int TestWaveId_Verify(WaveID &obj);	
};

class TestCrop{
public:
TestCrop() {}
	int TestWaveId_BigExet(WaveID &obj,string imagepath);
	
};

// TestCase事件-----测试初始化函数
class WaveInitTest : public testing::Test
{
public:
	virtual void SetUp();

	virtual void TearDown();
	WaveID obj;
};


// TestSuit事件-----测试提取特征函数
class WaveExtractFeatureTest : public testing::Test 
{
public:
	virtual void SetUp();
	virtual void TearDown();
	WaveID obj;
	TestFuns test_exect;
};


// TestSuit事件-----测试TestCrop
class WaveExtractCropTest : public testing::Test 
{
public:
	virtual void SetUp();
	virtual void TearDown();
	WaveID obj;
	TestCrop test_crop;
};



// TestSuit 事件测试----特征对比函数
class WaveFaceVerifyTest : public testing::Test {
public:
	virtual void SetUp();
  virtual void TearDown();
	WaveID obj;

  TestFuns test_exect;
};


