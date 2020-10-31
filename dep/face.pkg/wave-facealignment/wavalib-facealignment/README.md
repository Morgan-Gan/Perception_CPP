### ========================
## 工程目录
```
.  
├── demo1 #测试demo的编译及运行  
│   ├── 3rdparty #第三方依赖库的存放位置  
│   │   ├── include #第三方依赖头文件存放位置  
│   │   │   └── caffe #精简版的caffe头文件等  
│   │   │       ├── layers  
│   │   │       ├── proto  
│   │   │       ├── test  
│   │   │       └── util  
│   │   └── lib  
│   ├── data #测试数据  
│   ├── include #需要包含的头文件存放位置  
│   ├── logs #打印日志存放位置  
│   ├── models #测试model模型  
│   └── src #测试代码的源码  
└── libwaveface_alignment #封装库的编译及运行  
    ├── 3rdparty #第三方依赖库的存放位置  
    │   ├── include #第三方依赖头文件存放位置  
    │   │   └── caffe #精简版的caffe头文件等  
    │   │       ├── layers  
    │   │       ├── proto  
    │   │       ├── test  
    │   │       └── util  
    │   └── lib  
    ├── include #需要包含的头文件存放位置  
    ├── models #测试model模型  
    └── src #封装的源码  
```

### ========================
### 上手指南
提供更新版本的人脸关键点定位算法的头文件、封装库编译源码，以及测试调用脚本demo和模型文件。
### 运行环境
本编译开发环境基于centos64
### ========================
### 版本库封装步骤
> * 1.$ cd libwaveface_alignment/
> * 2.$ make clean
> * 3.$ make
> * 4.$ make install

### =========================
### 版本库测试demo
> * 1.$ make clean
> * 2.$ make
> * 3.$ ./test.2.0.0.bin 
> * 4.$ ./demo1/目录中的1.bmp文件就是demo测试结果

### =========================
### 版本说明
##### 版本waveface_alignment_v0.1.1:
其中libwaveface_alignment_centos64_gpu_v0.1.1.so->依赖于libwavedl_centos64_gpu_v0.1.1.so，使用深度学习方法封装的关键点定位算法，性能较之前有所提高。
##### 版本waveface_alignment_v0.1.3:
新添加小模型版本waveface_alignment_v0.1.3,libwaveface_alignment_centos64_gpu_v0.1.3.so->依赖于libwavedl_centos64_gpu_v0.1.1.so，该版本较0.1.1版本，模型更小，速度更快，针对移动端低功耗平台部署。


