CXX := g++

CUDA_DIR := /usr/local/cuda
CUDA_INCLUDE_DIR := $(CUDA_DIR)/include
CUDA_LIB_DIR := $(CUDA_DIR)/lib64
#CUDA_LIBRARYS := cudart cublas curand

INCLUDE_DIRS := ../libwaveface_detection/include ../libwaveface_detection/3rdparty/include ./include ./3rdparty/include ./gtest_linux_x86-64/include /usr/include/hdf5/serial $(CUDA_INCLUDE_DIR)
LIBRARY_DIRS :=  ../libwaveface_detection/3rdparty/lib ./gtest_linux_x86-64/lib64 ./libs  /usr/lib64/atlas /usr/local/lib /usr/lib/x86_64-linux-gnu/hdf5/serial $(CUDA_LIB_DIR)
LIBRARIES += wavedl_centos64_gpu_v0.1.1 opencv_core opencv_highgui opencv_imgproc opencv_imgcodecs glog gtest pthread gcov boost_system boost_filesystem boost_thread

WARNINGS := -Wall -Wno-sign-compare
COMMON_FLAGS += -DNDEBUG -O0 -DUSE_OPENCV $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
CXXFLAGS += -MMD -MP -pthread -fPIC -fprofile-arcs -ftest-coverage -O0 -std=c++0x -msse4.1 $(COMMON_FLAGS) $(WARNINGS)
NVCCFLAGS += -ccbin=$(CXX) -Xcompiler -fPIC $(COMMON_FLAGS)
LINKFLAGS += -pthread -fPIC $(COMMON_FLAGS) $(WARNINGS) -Wl,-rpath=./libs
LDFLAGS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir)) $(foreach library,$(LIBRARIES),-l$(library))

CXX_SRC := ../libwaveface_detection/src/face_detection.cpp ../libwaveface_detection/src/face_detection_cnn.cpp ./src/Test_Detect.cpp ./src/Test_Case.cpp ./src/Test_Main.cpp 
CXX_OBJS := ${CXX_SRC:.cpp=.o}
DEPS := ${CXX_OBJS:.o=.d}
BIN_NAME := unit-test.bin

.PHONY: test clean

test: $(CXX_OBJS)
	$(CXX) -o $(BIN_NAME) $(CXX_OBJS) $(LINKFLAGS) $(LDFLAGS)
	@echo $(LDFLAGS)

./src/%.o: ./src/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

clean:
	rm -rf $(CXX_OBJS) $(BIN_NAME) $(DEPS)

