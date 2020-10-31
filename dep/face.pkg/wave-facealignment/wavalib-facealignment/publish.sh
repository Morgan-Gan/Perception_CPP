#/bin/bash

PUBLISH_DIR_NAME=face_alignment-centos7_cuda9.1_cudnn7_amd64-0.1.1

mkdir -p $PUBLISH_DIR_NAME/lib
mkdir -p $PUBLISH_DIR_NAME/include

cp libwaveface_alignment/include/face_alignment.h  $PUBLISH_DIR_NAME/include/
cp -r libwaveface_alignment/models $PUBLISH_DIR_NAME
cp libwaveface_alignment/libwaveface_alignment_centos64_gpu_v0.1.1.so  $PUBLISH_DIR_NAME/lib/

# Package ...
rm -rf  wavedl-centos7_cuda9.1_cudnn7_amd64-0.1.1
zip -r  package.zip $PUBLISH_DIR_NAME
rm -r $PUBLISH_DIR_NAME
