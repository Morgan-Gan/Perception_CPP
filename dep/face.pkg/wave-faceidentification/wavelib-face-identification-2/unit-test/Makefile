CXX := g++

CPU_ONLY := 0
ifeq ($(CPU_ONLY), 1)
	COMMON_FLAGS += -DCPU_ONLY
else
	CUDA_DIR := /usr/local/cuda
	CUDA_INCLUDE_DIR := $(CUDA_DIR)/include
	CUDA_LIB_DIR := $(CUDA_DIR)/lib64
#	CUDA_LIBRARYS := cudart cublas curand cudnn
	COMMON_FLAGS += -DUSE_CUDNN
endif

INCLUDE_DIRS :=../include ./include ../3rdparty/include ./gtest_linux_x86-64/include /usr/include/hdf5/serial $(CUDA_INCLUDE_DIR)
LIBRARY_DIRS := ../lib ../3rdparty/lib ./gtest_linux_x86-64/lib64 /usr/lib64/atlas /usr/lib /usr/local/ffmpeg/lib /usr/lib/x86_64-linux-gnu /usr/lib/x86_64-linux-gnu/hdf5/serial  $(CUDA_LIB_DIR)
LIBRARIES += wavedl_centos64_gpu_v0.1.1  opencv_core opencv_core opencv_highgui opencv_video opencv_imgproc opencv_imgcodecs opencv_videoio swresample glog lkt5103_64 boost_system boost_filesystem boost_thread  gtest gcov $(CUDA_LIBRARYS)

WARNINGS := -Wall -Wno-sign-compare
COMMON_FLAGS += -DNDEBUG -DUSE_OPENCV $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
CXXFLAGS +=  -MMD -MP -pthread -lpthread -fprofile-arcs -ftest-coverage -fPIC -O0 -std=c++0x -msse4.1 $(COMMON_FLAGS) $(WARNINGS)
NVCCFLAGS += -ccbin=$(CXX) -Xcompiler -fPIC $(COMMON_FLAGS)
LINKFLAGS += -pthread -Wl,-rpath=../3rdparty/lib:../lib -pthread -fPIC $(COMMON_FLAGS) $(WARNINGS)
LDFLAGS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir)) $(foreach library,$(LIBRARIES),-l$(library))

CXX_SRC := ../src/face_identification.cpp ../src/dog.cpp ../src/tracelog.cpp ../src/caffeFwProc.cpp ../src/face_segement.cpp ./src/Test_Main.cpp ./src/Test_Case.cpp ./src/Test_Identification.cpp 
CXX_OBJS := ${CXX_SRC:.cpp=.o}
DEPS := ${CXX_OBJS:.o=.d}
BIN_NAME := unit_test.bin

.PHONY: test clean

test: $(CXX_OBJS)
	$(CXX) -o $(BIN_NAME) $(CXX_OBJS) $(LINKFLAGS) $(LDFLAGS)

./src/%.o: ./src/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

clean:
	rm -rf $(CXX_OBJS) $(BIN_NAME) $(DEPS)

