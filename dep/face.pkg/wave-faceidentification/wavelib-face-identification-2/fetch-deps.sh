#/bin/bash

curl -uadmin:password -O "http://193.168.1.117:8081/artifactory/development-local/wavedl/centos7-cuda9.1-cudnn7-amd64/0.1.1/package.zip"
unzip package.zip
rm -f package.zip

rm -rf 3rdparty/include/caffe
rm -f 3rdparty/lib/libwavedl_centos64_gpu_v0.1.1.so
rm -f lib/libwavedl_centos64_gpu_v0.1.1.so

cp -r wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/include/caffe  3rdparty/include/
cp wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/lib/libwavedl_centos64_gpu_v0.1.1.so 3rdparty/lib/
cp wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/lib/libwavedl_centos64_gpu_v0.1.1.so lib/


echo "download gtest"
cd unit-test  # build api-test
curl -uadmin:password -O "http://193.168.1.117:8081/artifactory/source-local/gtest-1.8.1.zip"
unzip gtest-1.8.1.zip 
rm -f gtest-1.8.1.zip
cd gtest-1.8.1
mkdir build 
cd build
cmake -DCMAKE_INSTALL_PREFIX=../../gtest_linux_x86-64 ..
make -j8 && make install
cd ../../..
