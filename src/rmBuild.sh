rm -r cmake-build-debug
mkdir cmake-build-debug
mkdir cmake-build-debug/resultados
mkdir cmake-build-debug/resultados/solPrint
mkdir cmake-build-debug/resultados/solCompleta
#cp throw.gdb cmake-build-debug/throw.gdb
cp dist cmake-build-debug/
cd cmake-build-debug
cmake ..
make -j 6
