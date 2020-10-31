#/bin/bash

cd libwaveface_alignment    #build .so
make clean
make -j8 && make install
cp libwaveface_alignment_centos64_gpu_v0.1.1.so ../unit-test/3rdparty/lib/
cd ..

cd demo1
make clean
make -j8
cd ..

echo "build unit-test"
cd unit-test  # build api-test
make clean
make -j8
cd ..
echo "build finish"

