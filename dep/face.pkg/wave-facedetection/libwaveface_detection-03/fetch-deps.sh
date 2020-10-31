#!/bin/bash

PLATFORM=$1
VERSION=$2

curl -u admin:password -O "http://193.168.1.117:8081/artifactory/development-local/wavedl/${PLATFORM}/${VERSION}/package.zip"

unzip package.zip
cp wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/lib/libwavedl_centos64_gpu_v0.1.1.so example/libs
cp wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/lib/libwavedl_centos64_gpu_v0.1.1.so unit-test/libs

rm -r libwaveface_detection/3rdparty/include/caffe
cp -r wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/include/caffe libwaveface_detection/3rdparty/include/

rm -r wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1
rm package.zip


cd unit-test  # download gtest add 2019-04-25
curl -uadmin:password -O "http://193.168.1.117:8081/artifactory/source-local/gtest-1.8.1.zip"
unzip gtest-1.8.1.zip 
rm -f gtest-1.8.1.zip
cd gtest-1.8.1
mkdir build 
cd build
cmake -DCMAKE_INSTALL_PREFIX=../../gtest_linux_x86-64 ..
make -j8 && make install
cd ..

