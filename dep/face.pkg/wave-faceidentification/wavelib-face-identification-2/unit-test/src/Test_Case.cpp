#include <limits.h>
#include "gtest/gtest.h"
#include "Test_Identification.h"

namespace{

TEST_F(WaveInitTest,SuccRet){
	sem_t sem;
	obj.SetModeCPU();	
	EXPECT_EQ(obj.Initial("../models",&sem,0),0);
	obj.Release();
	EXPECT_EQ(obj.Initial("../models",&sem,1),0);
	obj.Release();
	EXPECT_EQ(obj.Initial("../models",&sem,2),0);
	obj.Release();
	EXPECT_EQ(obj.Initial("../models",&sem,3),0);
	obj.SetModeGPU(3);
	obj.Release();
}


TEST_F(WaveExtractFeatureTest,ExecT){
	sem_t sem;	
	obj.Initial("../models",&sem,1);
	EXPECT_EQ(test_exect.TestWaveId_Exet(obj,"../data/00000_000001.jpg"),0);
	std::cout<<"WaveExtractFeatureTest case 1 finish"<<std::endl;
	EXPECT_EQ(test_exect.TestWaveId_Exet(obj,"../data/00000_000002.jpg"),0);
	std::cout<<"WaveExtractFeatureTest case 2 finish"<<std::endl;
	EXPECT_EQ(test_exect.TestWaveId_Exet(obj,"../data/000025.jpg"),0);
	std::cout<<"WaveExtractFeatureTest case 3 finish"<<std::endl;
	EXPECT_EQ(test_exect.TestWaveId_Exet(obj,"../data/00000_01095_00040.jpg"),0); 
	std::cout<<"WaveExtractFeatureTest case 4 finish"<<std::endl;
	obj.Release();
}

TEST_F(WaveExtractCropTest,ExectCrop){
	sem_t sem;	
	obj.Initial("../models",&sem,1);
	EXPECT_EQ(test_crop.TestWaveId_BigExet(obj,"../data/100037_100037.jpg"),0);
	obj.Release();
}


TEST_F(WaveFaceVerifyTest,TestWaveId_Verify){
	EXPECT_EQ(test_exect.TestWaveId_Verify(obj),0);

}

}
