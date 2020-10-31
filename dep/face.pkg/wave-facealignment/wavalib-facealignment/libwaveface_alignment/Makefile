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

LIBRARY_DIRS := /usr/local/lib/ ./3rdparty/lib /usr/lib /home/shiyuan/anaconda2/lib /usr/lib/x86_64-linux-gnu /usr/lib/x86_64-linux-gnu/hdf5/serial $(CUDA_LIB_DIR)
#LIBRARY_DIRS := ./libs /usr/lib /usr/lib64/mysql /usr/lib/x86_64-linux-gnu \
#/usr/lib64/atlas /usr/local/lib/ \
#./3rdparty/g2log/lib ./3rdparty/lib /usr/lib/x86_64-linux-gnu/hdf5/serial $(CUDA_LIB_DIR)

#LIBRARIES += wavedl_linux64_gpu_v0.2.0  opencv_core opencv_highgui opencv_imgproc glog gflags protobuf boost_system boost_filesystem m hdf5_hl hdf5 boost_thread stdc++ cblas atlas $(CUDA_LIBRARYS)
LIBRARIES += wavedl_centos64_gpu_v0.1.1  gpu_manager opencv_core opencv_highgui opencv_imgproc glog gflags protobuf boost_system boost_filesystem m hdf5_hl hdf5 boost_thread stdc++ $(CUDA_LIBRARYS)


WARNINGS := -Wall -Wno-sign-compare
COMMON_FLAGS += -DNDEBUG -O2 -DUSE_OPENCV $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
CXXFLAGS += -MMD -MP -pthread -fPIC -std=c++0x -msse4.1 $(COMMON_FLAGS) $(WARNINGS)
NVCCFLAGS += -ccbin=$(CXX) -Xcompiler -fPIC $(COMMON_FLAGS)
LINKFLAGS += -pthread -fPIC $(COMMON_FLAGS) $(WARNINGS)
LDFLAGS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir)) $(foreach library,$(LIBRARIES),-l$(library)) -Wl,-rpath=\$$ORIGIN

CXX_SRC :=  ./src/face_alignment.cpp  ./src/caffeFwProc_shiyuan.cpp ./src/model.cpp 
CXX_OBJS := ${CXX_SRC:.cpp=.o}
DEPS := ${CXX_OBJS:.o=.d}
LIB_NAME := libwaveface_alignment_centos64_gpu_v0.1.1.so

.PHONY: lib clean install

lib: $(CXX_OBJS)
	$(CXX) -shared -o $(LIB_NAME) $(CXX_OBJS) $(LINKFLAGS) $(LDFLAGS)

./src/%.o: ./src/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

clean:
	rm -rf $(CXX_OBJS) $(LIB_NAME) $(DEPS)

install:
	cp $(LIB_NAME) ../demo1/
	cp $(LIB_NAME) ../demo1/3rdparty/lib/
	cp include/face_alignment.h ../demo1/include/

