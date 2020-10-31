#ifndef GPU_ALLOC
#define GPU_ALLOC

#include <iostream>
#include <thread>
#include <mutex>
#include <map>


struct MemoryLocation {
    unsigned int id_device;
    unsigned int memory;

    MemoryLocation(): id_device(-1),memory(0) {
    }

    MemoryLocation(const MemoryLocation &other) {
        this->id_device = other.id_device;
        this->memory = other.memory;
    }

    MemoryLocation & operator = (const MemoryLocation &other) {
        this->id_device = other.id_device;
        this->memory = other.memory; 
        return *this;
    }
};



// GPU manager 单例模式
class GpuManagerSingleton {
 private:
  static GpuManagerSingleton * instance_;
  GpuManagerSingleton() {}

  // 保留内存，避免分配内存和GPU内存刚刚好，避免GPU崩溃
  static const int keep_memory_;
  // 锁，
  static std::mutex mtx_;
  static std::mutex mtx2_;

  // 算法实例已经使用的内存，void* 表示是 各个算法模块的地址
  // MemoryLocation 表示该算法模块使用的GPU的ID号，以及使用的内存
  static std::map<void*, MemoryLocation> use_memory_;

  // int 第一个表示GPU的ID号， 第二个表示GPU剩余多少M
  static std::map<int, int> remain_memory_;

  // 获取GPU资源，并存放在remain_memory里面
  static int GetGPUMemory();

  // 添加 void* ,gpu id 
  int AddMemory(const void* instance_address,
                const unsigned int &gpu_id,
                const unsigned int &need_memory);
  
 public:
  static GpuManagerSingleton * GetInstance();
  // 删除 Void* 所对应的GPU
  void DeleteMemory(const void * instance_address);
  
  // 根据 内存大小获取GPU的ID号
  int GetGpuID(const void* instance_address,
               const unsigned int &need_memory);

  
  
};

#endif //GPU_ALLOC
