#!/bin/sh
prefixname=$1
platename=$2

sed -i "1c prefix=${prefixname}" ../3rdparty/pkgconfig/*.pc
sed -i "2c platetype=${platename}" ../3rdparty/pkgconfig/*.pc
