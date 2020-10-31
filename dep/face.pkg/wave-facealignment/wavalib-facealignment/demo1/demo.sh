cd ../libwaveface_alignment
make clean
make
make install
cd ../demo1
make clean
make
./test.bin
