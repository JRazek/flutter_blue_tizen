cmake_minimum_required(VERSION 3.13) 
project(Testing)
set(CMAKE_CXX_STANDARD 17)

#set these variables!
set(TIZEN_STUDIO_DIR "$ENV{HOME}/tizen-studio")
set(FLUTTER_TIZEN_DIR "/home/user/Tizen/projects/flutter-tizen")
####################

set(CMAKE_C_COMPILER ${TIZEN_STUDIO_DIR}/tools/arm-linux-gnueabi-gcc-9.2/bin/arm-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER ${TIZEN_STUDIO_DIR}/tools/arm-linux-gnueabi-gcc-9.2/bin/arm-linux-gnueabi-g++)


set(SOURCE_FILES main.cpp)
add_executable(Main ${SOURCE_FILES})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I\"pch\"")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DEBUG")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -c")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++17")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target arm-tizen-linux-gnueabi")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ccc-gcc-name arm-linux-gnueabi-g++")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=softfp")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE} -gcc-toolchain ${TIZEN_STUDIO_DIR}/tools/arm-linux-gnueabi-gcc-9.2/")
SET(CMAKE_SYSROOT ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core)

target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include)
target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/network)
target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/nnstreamer)
target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/dlog)
target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/appfw)
target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/appcore-agent)

target_include_directories(Main PUBLIC ${FLUTTER_TIZEN_DIR}/embedding/cpp/include)
target_include_directories(Main PUBLIC ${FLUTTER_TIZEN_DIR}/flutter/bin/cache/artifacts/engine/tizen-common/cpp_client_wrapper/include)
target_include_directories(Main PUBLIC ${FLUTTER_TIZEN_DIR}/flutter/bin/cache/artifacts/engine/tizen-common/public)
target_include_directories(Main PUBLIC ${FLUTTER_TIZEN_DIR}/flutter/bin/cache/artifacts/engine/tizen-common/public)

set(
    lib_paths

    lib
    ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/lib
)
set(
    libraries
    
    flutter_engine
    flutter_tizen_tv
    flutter_plugins
    embedding_cpp
    dlog
    pthread

    appcore-agent
    capi-appfw-app-common
    capi-appfw-application
    capi-appfw-app-control
    capi-appfw-app-control-uri
    capi-appfw-app-manager
    capi-appfw-package-manager
    ffi
)


target_link_directories(Main PUBLIC ${lib_paths})
target_link_libraries(Main ${libraries})

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)