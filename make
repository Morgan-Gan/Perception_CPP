#! /bin/sh
case $1 in
    "clean")  
        rm -rf ./build/*
        ;;
    "build") 
        cd build 
        cmake ..
        make
        make install
        ;;
    "make") 
        cd build 
        make
        ;;
    "run")
        cd build/bin
        ./Ubiquitousperception
        ;;
    "debug")
        cd build/bin
        ;;
    *)  
        ./make "build"
    ;;
esac