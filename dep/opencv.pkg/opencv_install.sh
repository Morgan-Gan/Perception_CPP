#!/bin/sh
BASE=`pwd`
BUILD_HOST=
OUTPUT_PATH=${BASE}/install

pre_clean()
{
    rm -rf compressed source install
}

make_dirs () 
{
    #为了方便管理，创建有关的目录
    cd ${BASE} && mkdir compressed install source -p
}

tar_package() 
{
    cd ${BASE}
    rm -rf ${BASE}/compressed/*
    cp -rf *.tar.gz ${BASE}/compressed
    cd compressed
    ls * > /tmp/list.txt
    for TAR in `cat /tmp/list.txt`
    do
        tar -xf $TAR -C  ../source
    done
    rm -rf /tmp/list.txt
}

make_opencv()
{
    cd ${BASE}/source/opencv-4.3.0
    mkdir build && cd build
    cmake -D CMAKE_BUILD_TYPE=Release -D OPENCV_GENERATE_PKGCONFIG=YES -D CMAKE_INSTALL_PREFIX=${OUTPUT_PATH}/opencv ..
    make && make install
}

after_clean()
{
    cd ${BASE}
    rm -rf compressed source
}

pre_clean

make_dirs

tar_package

make_opencv

after_clean