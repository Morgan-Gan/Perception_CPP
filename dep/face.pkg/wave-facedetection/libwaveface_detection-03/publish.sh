#!/bin/bash

PACKAGE_DIR_NAME=face_detection-centos7_cuda9.1_cudnn7_amd64-2.0.0
TARGET_PATH=face_detection/centos7-cuda9.1-cudnn7-amd64/2.0.0

mkdir -p $PACKAGE_DIR_NAME/lib
cp libwaveface_detection/libwaveface_detection_centos64_gpu_v2.0.0.so $PACKAGE_DIR_NAME/lib
cp -r libwaveface_detection/models $PACKAGE_DIR_NAME/
cp -r libwaveface_detection/include $PACKAGE_DIR_NAME/

# Package ...
zip -r  package.zip $PACKAGE_DIR_NAME
rm -r $PACKAGE_DIR_NAME

