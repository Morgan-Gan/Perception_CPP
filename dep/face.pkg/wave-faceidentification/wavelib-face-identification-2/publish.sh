#/bin/bash

PUBLISH_DIR_NAME=face_identification-centos7_cuda9.1_cudnn7_amd64-2.1.1

mkdir -p $PUBLISH_DIR_NAME/lib
mkdir -p $PUBLISH_DIR_NAME/include

cp lib/libwaveface_identification_centos64_v2.1.1.so $PUBLISH_DIR_NAME/lib
cp -r models $PUBLISH_DIR_NAME
cp -r include/face_identification.hpp $PUBLISH_DIR_NAME/include

# Package ...
rm -rf  wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1
zip -r  package.zip $PUBLISH_DIR_NAME
rm -r $PUBLISH_DIR_NAME
