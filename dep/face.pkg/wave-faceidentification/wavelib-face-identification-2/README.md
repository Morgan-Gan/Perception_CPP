# 动态人脸识别库SDK 说明

---

*Update 2019年01月31日16:16*

- 修改内容：update dog.h dog.cpp 

## 提供文件

### 目录结构

``` 
.
├── 3rdparty
│   ├── include
│   │   └── caffe
│   └── lib
│       ├── libgpu_manager.so
│       ├── liblkt5103_64.so
│       ├── libwavedl_centos64_gpu_v0.1.1.a
│       ├── libwavedl_centos64_gpu_v0.1.1.so
│       └── libwavelogv0.0.0.so
├── demo
│   ├── data
│   │   ├── 00000_000001-何薇.jpg
│   │   ├── 00000_000002-勾多多.jpg
│   │   ├── 00000_00469_00035-何薇.jpg
│   │   ├── 00000_00469_00050-何薇.jpg
│   │   ├── 00000_01095_00040-勾多多.jpg
│   │   ├── 00000_01095_00045-勾多多.jpg
│   │   ├── 000025-许少辉.jpg
│   │   └── image_00000009.jpg
│   ├── include
│   │   └── face_identification.hpp
│   ├── Makefile
i│   └── src
│       └── test.cpp
├── include
│   ├── caffeFwProc.h
│   ├── DES.h
│   ├── dog.h
│   ├── face_identification.hpp
│   ├── face_segement.h
│   ├── lkt5103.h
│   ├── Local_common.h
│   ├── model.hpp
│   └── tracelog.h
├── lib
│   └── libwaveface_identification_centos64_v2.1.1.so
├── Makefile
├── models
│   ├── face_identification_linux64_v2.1.1-1.bin
│   └── face_identification_linux64_v2.1.1-2.bin
├── README.md
└── src
    ├── caffeFwProc.cpp
    ├── DES.cpp
    ├── dog.cpp
    ├── face_identification.cpp
    ├── face_segement.cpp
    └── tracelog.cpp

12 directories, 36 files

```

说明：

- `3rdparty` 第三方头文件和第三方库
- `demo` 中存放 测试动态人脸SDK
- `models` 算法模型文件，共 2 个，存放在 `models` 目录中（注意，文件名不能随意修改）
- `include`  封装动态人脸算法SDK头文件
- `src`  封装动态人脸算法SDK需要的源码文件
- `lib`  封装生成的动态链接库目录

---
## 使用环境

- centos系统  x86_64
- opencv3.0
- 3rdparty
- lib
- cuda9.0 
- cudnn7.1

---

## 算法功能

- 动态人脸识别算法提供sdk，完成人脸识别

---

## SDK 接口说明

> 此接口说明适用于 v2.1.0 及以后的版本

### 文件结构

- SDK 接口文件存放在 `demo/include` 目录下
- SDK 动态链接库存放在 `./lib` 目录下

---

## SDK算法模型介绍

### 训练集
- `VGGFACE2`: 9121人 2621470张
- `自建人证数据集`:  211963人 624773张
- `MS-1M`:  86526人 3906589张

### 测试集

- `data-30` : 测试视频297，约30人
- `register1000`: 注册集 1000人

### 准确率

- `face_identification_linux64_v2.1.1-1.bin`  完全拒识情况  正确识别率:92.6%
- `face_identification_linux64_v2.1.1-2.bin`  完全拒识情况  正确识别率:89.8%
- `SDK 模型准确率`  完全拒识情况  正确识别率:97.54%

---

## SDK 编译及测试

### 动态链接库编译

``` sh
 cd ./
 make clean
 make 
 make install
```
---

### 动态链接库测试

``` sh
 cd ./demo
 make clean
 make 
 ./test.2.0.0.bin
```
---

*All right reserved @wavewisdom-bj*




