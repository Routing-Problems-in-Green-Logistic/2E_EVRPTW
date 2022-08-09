rm -r cmake-build-debug
mkdir cmake-build-debug
#cp throw.gdb cmake-build-debug/throw.gdb
cp dist cmake-build-debug/
cd cmake-build-debug
cmake ..
make -j 6
