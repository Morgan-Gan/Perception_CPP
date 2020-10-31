#include "Test_Identification.h"

int TestFuns::TestWaveId_Exet(WaveID &obj,string imagepath)
	{
	   	cv::Rect faceRect;
		faceRect.x = 0;
		faceRect.y = 0;
		faceRect.width = 0;
		faceRect.height = 0;
		cv::Mat img = cv::imread(imagepath);
		cv::Mat landmarks;
		landmarks.create(1,136,CV_32FC1);
		int feature_dim = obj.GetFeatureDim();
		float* features = new float[feature_dim];
		obj.ExtractFeature(img,faceRect,landmarks,features);
		for(int i=0; i<feature_dim; i++)
		{
			if((features[i] > 1)||(features[i] < -1))
			{
				return -1;		
			}	
		}
		return 0;
	}

int TestCrop::TestWaveId_BigExet(WaveID &obj,string imagepath)
	{
	  std::cout << "into TestWaveId_BigExet: " << std::endl;
	  cv::Rect faceRect;
		faceRect.x = 222;
		faceRect.y = 373;
		faceRect.width = 321;
		faceRect.height = 321;
		cv::Mat img = cv::imread(imagepath);
		cv::Mat landmarks;
		landmarks.create(1,136,CV_32FC1);
		int feature_dim = obj.GetFeatureDim();
		float* features = new float[feature_dim];
		obj.ExtractFeature(img,faceRect,landmarks,features);
		
		return 0;
	}


int TestFuns::TestWaveId_Verify(WaveID &obj)
	{
		float *features_1 = new float[1024];
		float *features_2 = new float[1024];
		int N = 999;
		
		
		for(int i=0;i<10;i++)
		{
			srand(time(NULL));
			for(int j=0;j<1024;j++)
			{
				int k = rand()%2;
				features_1[j] = rand()%(N+1)/(float)(N+1);
				if( k == 0)
					features_1[j] = features_1[j]*(-1.0);
			}
			srand(time(NULL));
			for(int j=0;j<1024;j++)
			{
				int k = rand()%2;
				features_2[j] = rand()%(N+1)/(float)(N+1);
				if( k == 0)
					features_2[j] = features_2[j]*(-1.0);
			}
			double fscore = obj.FaceVerify(features_1,features_2);
			if((fscore > 1) ||(fscore < 0))
				return -1;
		}

		return 0;
	}


// TestCase事件-----测试初始化函数
void WaveInitTest :: SetUp() 
	{
		std::cout << "WaveID Test Init Test SetUP" << std::endl;
	}

void WaveInitTest :: TearDown()
	{
		std::cout << "WaveID Test Init Test TearDown" << std::endl;
	}
	


// TestSuit事件-----测试提取特征函数
void WaveExtractFeatureTest:: SetUp()
	{
		std::cout << "WaveID Test ExtractFeature  Test SetUP" << std::endl;	
	}
void WaveExtractFeatureTest:: TearDown() 
	{
		std::cout << "WaveID Test ExtractFeature  Test TearDown" << std::endl;
	}
	
// TestSuit事件-----测试Crop
void WaveExtractCropTest:: SetUp()
	{
		std::cout << "WaveID Test ExtractFeature  Test SetUP" << std::endl;	
	}
void WaveExtractCropTest:: TearDown() 
	{
		std::cout << "WaveID Test ExtractFeature  Test TearDown" << std::endl;
	}
	
	
	
// TestSuit 事件测试----特征对比函数
void WaveFaceVerifyTest :: SetUp()
         {
              std::cout << "WaveID Test FaceVerify  Test SetUP" << std::endl;	   
         }
void WaveFaceVerifyTest :: TearDown()
         {
          	std::cout << "WaveID Test FaceVerify Test SetUP" << std::endl;	
         }
	 		


