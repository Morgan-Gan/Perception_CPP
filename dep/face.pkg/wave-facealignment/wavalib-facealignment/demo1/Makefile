CXX := g++

CPU_ONLY := 0
ifeq ($(CPU_ONLY), 1)
	COMMON_FLAGS += -DCPU_ONLY
else
	CUDA_DIR := /usr/local/cuda
	CUDA_INCLUDE_DIR := $(CUDA_DIR)/include
	CUDA_LIB_DIR := $(CUDA_DIR)/lib64
	CUDA_LIBRARYS := cudart cublas curand cudnn
	COMMON_FLAGS += -DUSE_CUDNN
endif

INCLUDE_DIRS := ./include ./3rdparty/include /home/shiyuan/anaconda2/include  /usr/include/hdf5/serial $(CUDA_INCLUDE_DIR)
#INCLUDE_DIRS := ./ ./include ./3rdparty/hikcom/include ./3rdparty/include \
#/usr/include/mysql /usr/include/hdf5/serial ./3rdparty/vlc/include \
#./3rdparty/g2log/include $(CUDA_INCLUDE_DIR)

LIBRARY_DIRS := ./3rdparty/lib /usr/local/lib/ /usr/lib64/atlas /usr/lib /usr/local/ffmpeg/lib /usr/lib/x86_64-linux-gnu /usr/lib/x86_64-linux-gnu/hdf5/serial $(CUDA_LIB_DIR)
#LIBRARY_DIRS :=./ ./libs /usr/lib /usr/lib64/mysql /usr/lib/x86_64-linux-gnu \
#/usr/lib64/atlas /usr/local/lib/ \
#./3rdparty/g2log/lib ./3rdparty/lib /usr/lib/x86_64-linux-gnu/hdf5/serial $(CUDA_LIB_DIR)

#LIBRARIES += waveface_alignment_linux64_gpu_v0.1.1  wavedl_linux64_gpu_v0.2.0   wavelogv0.0.0 opencv_core opencv_highgui opencv_imgproc  opencv_imgcodecs glog gflags protobuf boost_system boost_filesystem m hdf5_hl hdf5 boost_thread stdc++ cblas atlas $(CUDA_LIBRARYS)
LIBRARIES += waveface_alignment_centos64_gpu_v0.1.1 wavedl_centos64_gpu_v0.1.1 gpu_manager wavelogv0.0.0 opencv_core opencv_highgui opencv_video opencv_imgproc opencv_imgcodecs opencv_videoio swresample glog gflags protobuf boost_system boost_filesystem m hdf5_hl hdf5 boost_thread stdc++ cblas atlas $(CUDA_LIBRARYS)

WARNINGS := -Wall -Wno-sign-compare
COMMON_FLAGS += -DNDEBUG -O2 -DUSE_OPENCV $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
CXXFLAGS += -MMD -MP -pthread -fPIC -std=c++0x -msse4.1 $(COMMON_FLAGS) $(WARNINGS)
NVCCFLAGS += -ccbin=$(CXX) -Xcompiler -fPIC $(COMMON_FLAGS)
LINKFLAGS += -Wl,-rpath,./3rdparty/lib -pthread -fPIC $(COMMON_FLAGS) $(WARNINGS)
LDFLAGS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir)) $(foreach library,$(LIBRARIES),-l$(library))


CXX_SRC := ./src/test.cpp
CXX_OBJS := ${CXX_SRC:.cpp=.o}
DEPS := ${CXX_OBJS:.o=.d}
BIN_NAME := test.bin

.PHONY: test clean

test: $(CXX_OBJS)
	$(CXX) -o $(BIN_NAME) $(CXX_OBJS) $(LINKFLAGS) $(LDFLAGS)

./src/%.o: ./src/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

clean:
	rm -rf $(CXX_OBJS) $(BIN_NAME) $(DEPS)

