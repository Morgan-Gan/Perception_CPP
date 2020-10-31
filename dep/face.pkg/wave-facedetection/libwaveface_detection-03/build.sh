#/bin/bash

cd libwaveface_detection
make clean
make
cd ..
cp libwaveface_detection/libwaveface_detection_centos64_gpu_v2.0.0.so example/libs
cp libwaveface_detection/models/* example/models

cd example
make clean
make

#add 2019-04-26 by ljk
echo "build unit-test"
cd ../unit-test  
make clean
make -j8
echo "build finish"

