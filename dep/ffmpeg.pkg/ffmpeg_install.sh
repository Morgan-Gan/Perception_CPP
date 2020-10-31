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

download_package() 
{
    cd ${BASE}
    wget http://download.videolan.org/videolan/x265/x265_3.2.1.tar.gz
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

make_x264()
{
    cd ${BASE}/source/x264*
    ./configure --enable-shared --prefix=${OUTPUT_PATH}/x264
    make && make install
}

make_x265() 
{
    # 其他编译选项可以 通过 在 cmake ../source 以后 ccmake ../source 可以查看 ( ccmake 可以通过 sudo apt-get install cmake-curses-gui  进行安装 )
    cd ${BASE}/source/x265*/source

    # 获取 工具链所在位置 下面的操作为的是在 CMakeLists.txt 中插入下面内容
    GCC_FULL_PATH=`whereis ${BUILD_HOST}gcc | awk -F: '{ print $2 }' | awk '{print $1}'` # 防止多个结果
    GCC_DIR=`dirname ${GCC_FULL_PATH}/`
    sed -i "1i\set( CMAKE_SYSTEM_NAME Linux  )"                         CMakeLists.txt
    sed -i "2a\set( CMAKE_SYSTEM_PROCESSOR ARM  )"                      CMakeLists.txt
    sed -i "2a\set( CMAKE_C_COMPILER ${GCC_DIR}/${BUILD_HOST}gcc  )"    CMakeLists.txt
    sed -i "2a\set( CMAKE_CXX_COMPILER ${GCC_DIR}/${BUILD_HOST}g++  )"  CMakeLists.txt
    sed -i "2a\set( CMAKE_FIND_ROOT_PATH ${GCC_DIR} )"                  CMakeLists.txt
    sed -i "2a\set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fPIC )"         CMakeLists.txt             
    cmake ../source

    # 指定安装路径
    sed -i "1i\set( CMAKE_INSTALL_PREFIX "${OUTPUT_PATH}/x265"  )"     cmake_install.cmake
    make && make install
}

make_ffmpeg()
{
    cd ${BASE}/source/ffmpeg-4.2.3
    CFLAGS="-L${OUTPUT_PATH}/x264 -L${OUTPUT_PATH}/x265" ./configure --enable-gpl --enable-shared --enable-libx264 --enable-libx265 --prefix=${OUTPUT_PATH}/ffmpeg
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

make_x264

make_x265

make_ffmpeg

after_clean