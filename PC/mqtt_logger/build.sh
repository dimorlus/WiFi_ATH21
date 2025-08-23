#!/usr/bin/env bash
rm -rf build_lnx
cmake -S . -B build_lnx -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux
cmake --build build_lnx -j$(nproc)