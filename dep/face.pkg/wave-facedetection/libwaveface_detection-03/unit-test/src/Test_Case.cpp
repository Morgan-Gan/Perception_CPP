#include <limits.h>
#include "gtest/gtest.h"
#include "Test_Detect.h"


namespace{

TEST_F(WaveFaceInitTest,SuccRet){
	
	EXPECT_EQ(fd.Initial("./models",0,40, 1000, 0.7, 0.7, 0.7),0);
	fd.Release();
	EXPECT_EQ(fd.Initial("./models",1,40, 1000, 0.7, 0.7, 0.7),0);
	fd.Release();
	EXPECT_EQ(fd.Initial("./models",2,40, 1000, 0.7, 0.7, 0.7),0);
	fd.Release();
	EXPECT_EQ(fd.Initial("./models",3,40, 1000, 0.7, 0.7, 0.7),0);
	fd.Release();
}


TEST_F(WaveFaceDetectTest,ExecT){
	
	fd.Initial("./models",1,40, 1000, 0.7, 0.7, 0.7);
	EXPECT_EQ(test_exect.TestWaveFace_Detect(fd,"./images/1.jpg"),0);
	std::cout<<"WaveFaceDetectTest case 1 finish"<<std::endl;
	EXPECT_EQ(test_exect.TestWaveFace_Detect(fd,"./images/2.jpg"),0);
	std::cout<<"WaveFaceDetectTest case 2 finish"<<std::endl;
	EXPECT_EQ(test_exect.TestWaveFace_Detect(fd,"./images/4.jpg"),0);
	std::cout<<"WaveFaceDetectTest case 2 finish"<<std::endl;
	EXPECT_EQ(test_exect.TestWaveFace_Detect(fd,"./images/test11.jpg"),0); 
	std::cout<<"WaveFaceDetectTest case 4 finish"<<std::endl;
	fd.Release();
}


}
