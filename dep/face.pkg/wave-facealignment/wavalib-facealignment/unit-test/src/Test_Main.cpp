#include <limits.h>
#include "gtest/gtest.h"
#include "Test_Alignment.h"

namespace{

TEST(WaveAlignment_NEAR,SameImgEqual){
  EXPECT_NEAR(0.0, TestWaveAlignment_Fun("./data/image_00000009.jpg","1.txt"),1.0);
  EXPECT_NEAR(0.0, TestWaveAlignment_Fun("./data/image_00000009.jpg","1.txt"),1.0);
  EXPECT_NEAR(0.0, TestWaveAlignment_Fun("./data/image_00000009.jpg","1.txt"),1.0);
}

}
