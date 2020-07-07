cd /home/docker/project
mkdir -p build-cross
cd build-cross
cmake -DCMAKE_TOOLCHAIN_FILE=../docker/cross-win/toolchain.cmake ..
make $@
