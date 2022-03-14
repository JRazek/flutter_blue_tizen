# set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 3.13) 
project(Testing)
set(CMAKE_CXX_STANDARD 17)

#set these variables!
set(TIZEN_STUDIO_DIR "$ENV{HOME}/tizen-studio")
set(FLUTTER_TIZEN_DIR "/home/user/Tizen/projects/flutter-tizen")
####################

set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_C_COMPILER /usr/bin/clang)

set(CMAKE_CXX_COMPILER_TARGET armv7-a)
set(CMAKE_C_COMPILER_TARGET armv7-a)

add_executable(
    Main 
    main.cpp
)

target_compile_options(
    Main PUBLIC 
    -Ipch
    -g3
    -Wall
    -O0
    -fmessage-length=0
    --target=armv7-a
    -fPIC
    -march=armv7-a
    -mfloat-abi=softfp
    -D_DEBUG
    -ggdb
)

target_link_options(
    Main PUBLIC 
    -Ipch
    -g3
    -Wall
    -O0
    -fmessage-length=0
    --target=armv7-a
    -fPIC
    -march=armv7-a
    -static-libstdc++
    -Xlinker
    --unresolved-symbols=ignore-in-shared-libs
    --sysroot=${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core
    -Wl,-rpath,../lib/armel 
    # -Wl,-rpath,/home/owner/unit_tests_flutter_blue/lib/armel
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ccc-gcc-name arm-linux-gnueabi-g++")

SET(CMAKE_SYSROOT ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core)

target_include_directories(
    Main PUBLIC

    ${TIZEN_STUDIO_DIR}/tools/arm-linux-gnueabi-gcc-9.2/arm-tizen-linux-gnueabi/include/c++/9.2.0/
    ${TIZEN_STUDIO_DIR}/tools/arm-linux-gnueabi-gcc-9.2/arm-tizen-linux-gnueabi/include/c++/9.2.0/arm-tizen-linux-gnueabi/
    ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/network
    ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/dlog
    ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/appfw
    ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/appcore-agent

    ${FLUTTER_TIZEN_DIR}/embedding/cpp/include
    ${FLUTTER_TIZEN_DIR}/flutter/bin/cache/artifacts/engine/tizen-common/cpp_client_wrapper/include
    ${FLUTTER_TIZEN_DIR}/flutter/bin/cache/artifacts/engine/tizen-common/public
)

target_link_libraries(
    Main  

    pthread
    embedding_cpp
    dlog
    flutter_plugins
    flutter_engine
    flutter_tizen_tv
    protobuf-lite

    appcore-agent
    capi-appfw-app-common
    capi-appfw-application
    capi-appfw-app-control
    capi-appfw-app-control-uri
    capi-appfw-app-manager
    capi-appfw-package-manager
    ffi
)


target_link_directories(
    Main PUBLIC

    lib/armel
)

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
