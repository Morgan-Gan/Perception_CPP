CXX := g++
VERSION_MAJOR := 2
VERSION_MINOR := 0
VERSION_REVISION := 0
CUDA_DIR := /usr/local/cuda
CUDA_INCLUDE_DIR := $(CUDA_DIR)/include
CUDA_LIB_DIR := $(CUDA_DIR)/lib64
CUDA_LIBRARYS := cudart cublas curand

INCLUDE_DIRS := ./include ./3rdparty/include /usr/include/hdf5/serial $(CUDA_INCLUDE_DIR)
LIBRARY_DIRS := ./3rdparty/lib /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/x86_64-linux-gnu/hdf5/serial $(CUDA_LIB_DIR)
LIBRARIES = 

WARNINGS := -Wall -Wno-sign-compare
COMMON_FLAGS += -DNDEBUG -O0 -DUSE_OPENCV $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
CXXFLAGS += -MMD -MP -pthread -fPIC -std=c++0x -Wall -msse4.1 $(COMMON_FLAGS) $(WARNINGS)
NVCCFLAGS += -ccbin=$(CXX) -Xcompiler -fPIC $(COMMON_FLAGS)
LINKFLAGS += -pthread -fPIC $(COMMON_FLAGS) $(WARNINGS) 
LDFLAGS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir)) $(foreach library,$(LIBRARIES),-l$(library))

CXX_SRC := ./src/face_detection.cpp ./src/face_detection_cnn.cpp
CXX_OBJS := ${CXX_SRC:.cpp=.o}
DEPS := ${CXX_OBJS:.o=.d}
LIB_NAME := libwaveface_detection_centos64_gpu_v$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_REVISION).so
MODEL_NAME := face_detection_centos64_v$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_REVISION).wave 
.PHONY: lib clean distribute  install

lib: $(CXX_OBJS)
	$(CXX) -shared -o $(LIB_NAME) $(CXX_OBJS) $(LINKFLAGS)

./src/%.o: ./src/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -g -o $@

clean:
	rm -rf $(CXX_OBJS) $(LIB_NAME) $(DEPS)

DISTRIBUTE_PATH := libwaveface_detection_release
distribute: lib
	@rm -rf ../$(DISTRIBUTE_PATH)/include
	@rm -rf ../$(DISTRIBUTE_PATH)/libs
	@rm -rf ../$(DISTRIBUTE_PATH)/models
	@mkdir ../$(DISTRIBUTE_PATH)/include 
	@mkdir ../$(DISTRIBUTE_PATH)/libs
	@mkdir ../$(DISTRIBUTE_PATH)/models
	@cp -rf include/face_detection.h ../$(DISTRIBUTE_PATH)/include 
	@cp -rf $(LIB_NAME) ../$(DISTRIBUTE_PATH)/libs
	@cp -rf models/$(MODEL_NAME) ../$(DISTRIBUTE_PATH)/models

install:
	cp $(LIB_NAME) ../demo/libs/
	cp include/face_detection.h ../demo/include/
	cp models/* ../demo/models/

