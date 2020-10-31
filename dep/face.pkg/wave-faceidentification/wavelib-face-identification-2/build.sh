#/bin/bash


make clean
make -j8
make install   # build .so

cd benchmark  #build benchmark
make clean
make -j8
cd ..

echo "build api-test"

cd unit-test
make clean
make -j8
cd ..
echo "build finish"




