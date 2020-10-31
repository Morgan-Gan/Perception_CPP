CXX := g++

MAJORNAME := waveface_identification
PLATFORM := centos64
DEVICE := gpu
VERSION_MAJOR := 2
VERSION_MINOR := 1
VERSION_REVISION := 1

# 计算模式设置
# CPU_ONLY := 1
ifeq ($(CPU_ONLY), 1)
	COMMON_FLAGS += -DCPU_ONLY
else
	CUDA_DIR := /usr/local/cuda
	CUDA_INCLUDE_DIR := $(CUDA_DIR)/include
	CUDA_LIB_DIR := $(CUDA_DIR)/lib64
	CUDA_LIBRARYS := cudart cublas curand cudnn
	COMMON_FLAGS += -DUSE_CUDNN 
endif

INCLUDE_DIRS := ./include ./3rdparty/include /usr/include/hdf5/serial $(CUDA_INCLUDE_DIR)
LIBRARY_DIRS := ./lib /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/x86_64-linux-gnu/hdf5/serial $(CUDA_LIB_DIR)
WARNINGS := -Wall -Wno-sign-compare
COMMON_FLAGS += -DNDEBUG -O2 -DUSE_OPENCV $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
CXXFLAGS += -MMD -MP -pthread -fPIC -std=c++0x -msse4.1 $(COMMON_FLAGS) $(WARNINGS)
NVCCFLAGS += -ccbin=$(CXX) -Xcompiler -fPIC $(COMMON_FLAGS)
LINKFLAGS += -pthread -fPIC -L./lib -lwavedl_centos64_gpu_v0.1.1 -lgpu_manager -lwavelogv0.0.0 -llkt5103_64 -Wl,-rpath='$$ORIGIN' $(COMMON_FLAGS) $(WARNINGS)
LDFLAGS += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir)) $(foreach library,$(LIBRARIES),-l$(library))
CXX_SRC := ./src/face_identification.cpp ./src/dog.cpp ./src/tracelog.cpp ./src/caffeFwProc.cpp ./src/face_segement.cpp
CXX_OBJS := ${CXX_SRC:.cpp=.o}
DEPS := ${CXX_OBJS:.o=.d}
LIB_NAME := lib$(MAJORNAME)_$(PLATFORM)_v$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_REVISION).so

.PHONY: lib clean install distribute

lib: $(CXX_OBJS)
	$(CXX) -shared -o $(LIB_NAME) $(CXX_OBJS) $(LINKFLAGS)

./src/%.o: ./src/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

clean:
	@ rm -rf $(CXX_OBJS) $(LIB_NAME) $(DEPS) ./distribute ./lib$(MAJORNAME)_$(PLATFORM)_v$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_REVISION).so

install:
	
	cp $(LIB_NAME) ./lib/
	cp include/face_identification.hpp ./demo/include/
	rm -r $(LIB_NAME)

distribute: lib
	@ rm -rf ./distribute
	@ mkdir ./distribute
	@ mkdir ./distribute/include
	@ mkdir ./distribute/lib
	@ cp include/face_identification.hpp ./distribute/include/
	@ cp $(LIB_NAME) ./distribute/lib
