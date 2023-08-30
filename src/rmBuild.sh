rm -rf cmake-build-debug
mkdir cmake-build-debug
cd cmake-build-debug
cmake .. -D VAR_DEBUG=1
make -j 4
