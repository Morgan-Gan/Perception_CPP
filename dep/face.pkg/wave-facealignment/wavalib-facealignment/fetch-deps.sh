#/bin/bash

curl -uadmin:password -O "http://193.168.1.117:8081/artifactory/development-local/wavedl/centos7-cuda9.1-cudnn7-amd64/0.1.1/package.zip"
unzip package.zip
rm -f package.zip

rm -rf libwaveface_alignment/3rdparty/include/caffe
rm -f libwaveface_alignment/3rdparty/lib/libwavedl_centos64_gpu_v0.1.1.so
rm -rf demo1/3rdparty/include/caffe
rm -f demo1/3rdparty/lib/libwavedl_centos64_gpu_v0.1.1.so
rm -f demo1/3rdparty/lib/libwaveface_alignment_centos64_gpu_v0.1.1.so

cp -r wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/include/caffe  libwaveface_alignment/3rdparty/include/
cp -r wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/include/caffe  demo1/3rdparty/include/
cp wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/lib/libwavedl_centos64_gpu_v0.1.1.so libwaveface_alignment/3rdparty/lib/
cp wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/lib/libwavedl_centos64_gpu_v0.1.1.so demo1/3rdparty/lib/

rm -rf libwaveface_alignment/3rdparty/include/caffe
rm -f libwaveface_alignment/3rdparty/lib/libwavedl_centos64_gpu_v0.1.1.so
rm -rf unit-test/3rdparty/include/caffe
rm -f unit-test/3rdparty/lib/libwavedl_centos64_gpu_v0.1.1.so
rm -f unit-test/3rdparty/lib/libwaveface_alignment_centos64_gpu_v0.1.1.so

cp -r wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/include/caffe  libwaveface_alignment/3rdparty/include/
cp -r wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/include/caffe  unit-test/3rdparty/include/
cp wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/lib/libwavedl_centos64_gpu_v0.1.1.so libwaveface_alignment/3rdparty/lib/
cp wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1/lib/libwavedl_centos64_gpu_v0.1.1.so unit-test/3rdparty/lib/


echo "download gtest"
cd unit-test  # build unit-test
curl -uadmin:password -O "http://193.168.1.117:8081/artifactory/source-local/gtest-1.8.1.zip"
unzip gtest-1.8.1.zip 
rm -f gtest-1.8.1.zip
cd gtest-1.8.1
mkdir build 
cd build
cmake -DCMAKE_INSTALL_PREFIX=../../gtest_linux_x86-64 ..
make -j8 && make install
cd ../../..
