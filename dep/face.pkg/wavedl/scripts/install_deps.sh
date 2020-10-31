#!/bin/bash

yum install epel-release -y
yum update -y
yum install wget -y

yum install unzip -y
yum install make -y

#-- Install tools
wget https://github.com/Kitware/CMake/archive/v3.5.1.zip
unzip v3.5.1.zip
cd CMake-3.5.1
./configure
make -j16
make install
source /etc/profile
cd ..
rm -rf CMake-3.5.1

yum install autoconf automake libtool -y

### Install caffe dependences
yum install gflags-devel glog-devel atlas-devel hdf5-devel lmdb-devel leveldb-devel snappy-devel 

cp /lib64/atlas/libsatlas.so /lib64/atlas/libcblas.so
cp /lib64/atlas/libsatlas.so /lib64/atlas/libatlas.so

#-- Install Protobuf
wget https://github.com/protocolbuffers/protobuf/archive/v2.6.1.zip
unzip v2.6.1.zip
cd protobuf-2.6.1
wget https://github.com/google/googletest/archive/release-1.5.0.zip
unzip release-1.5.0.zip
mv googletest-release-1.5.0 gtest
./autogen.sh
./configure
make -j64
make install
cd ..
rm -rf protobuf-2.6.1

#-- Install Boost
wget https://dl.bintray.com/boostorg/release/1.65.1/source/boost_1_65_1.zip
unzip boost_1_65_1.zip
cd boost_1_65_1
./bootstrap.sh
./b2 install
cd ..
rm -rf boost_1_65_1

#-- Install OpenCV
wget https://github.com/opencv/opencv/archive/3.0.0.zip
unzip 3.0.0.zip
cd opencv-3.0.0
mkdir -p 3rdparty/ippicv/downloads/linux-8b449a536a2157bcad08a2b9f266828b
wget http://sourceforge.net/projects/opencvlibrary/files/3rdparty/ippicv/ippicv_linux_20141027.tgz
mv ippicv_linux_20141027.tgz 3rdparty/ippicv/downloads/linux-8b449a536a2157bcad08a2b9f266828b
mkdir build
cd build
cmake -DBUILD_TESTS=OFF -DWITH_CUDA=OFF -DCMAKE_BUILD_TYPE=release ..
make -j64
make install
cd ..
rm -rf opencv-3.0.0

