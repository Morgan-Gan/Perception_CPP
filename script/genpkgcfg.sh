#!/bin/sh
prefixname=$1
platename=$2

sed -i "1c prefix=${prefixname}" ../bin/pkgconfig/*.pc
sed -i "2c platetype=${platename}" ../bin/pkgconfig/*.pc