#!/usr/bin/env bash
set -epu pipefail

# check if dir build not exists
if [ ! -d "build" ]; then
  mkdir build
  cmake -B build -S . -DCMAKE_INSTALL_PREFIX="" -DCMAKE_BUILD_TYPE=Debug
  ln -snf build/compile_commands.json .
fi

make -C build
DESTDIR="." make install -C build
#ctest --test-dir build

export ASAN_OPTIONS=detect_leaks=0
#gdb -q -ex "set confirm off" -ex "set pagination off" -ex "file ./build/bin/reproduce" -ex "run './build/bin/printrandom'" ./build/bin/reproduce
./build/bin/reproduce ./build/bin/printrandom
