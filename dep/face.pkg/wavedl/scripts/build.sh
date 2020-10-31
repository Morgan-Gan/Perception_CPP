#!/bin/bash

export LD_LIBRARY_PATH=/usr/local/lib

mv Makefile.config.example Makefile.config

#-- Start building
make -j64

make test

make runtest

