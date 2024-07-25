#!/bin/bash

#BUILD_TYPE=Release
BUILD_TYPE=Debug

ANDROID_NDK_PATH=~/opts/ndk/android-ndk-r18b

ROOT_PWD=$( cd "$( dirname $0 )" && cd -P "$( dirname "$SOURCE" )" && pwd )

BUILD_DIR=${ROOT_PWD}/build/build_rk3588_android

if [[ ! -d "${BUILD_DIR}" ]]; then
  mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}
cmake ../.. \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_SYSTEM_VERSION=23 \
    -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
    -DCMAKE_ANDROID_STL_TYPE=c++_static \
    -DCMAKE_ANDROID_NDK=${ANDROID_NDK_PATH} \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_INSTALL_PREFIX=${ROOT_PWD}/install/rockiva_rk3588_android
make
make install
cd -
