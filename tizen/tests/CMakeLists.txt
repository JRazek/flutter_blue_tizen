# set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 3.13) 
project(Testing)
set(CMAKE_CXX_STANDARD 17)

#set these variables!
set(TIZEN_STUDIO_DIR "$ENV{HOME}/tizen-studio")
set(FLUTTER_TIZEN_DIR "/home/user/Tizen/projects/flutter-tizen")
####################

set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)

set(SOURCE_FILES main.cpp)

set(CMAKE_CXX_COMPILER_TARGET armv7-a)
set(CMAKE_C_COMPILER_TARGET armv7-a)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I\"pch\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --target=armv7-a")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE} -gcc-toolchain ${TIZEN_STUDIO_DIR}/tools/arm-linux-gnueabi-gcc-9.2/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ccc-gcc-name arm-linux-gnueabi-g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xlinker")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --unresolved-symbols=ignore-in-shared-libs")


add_executable(Main ${SOURCE_FILES})


target_compile_options(Main PUBLIC "-I\"pch\"")
target_compile_options(Main PUBLIC "-g3")
target_compile_options(Main PUBLIC "-D_DEBUG")
target_compile_options(Main PUBLIC "-Wall")
target_compile_options(Main PUBLIC "-O0")
target_compile_options(Main PUBLIC "-fmessage-length=0")
target_compile_options(Main PUBLIC "-std=c++17")
target_compile_options(Main PUBLIC "--target=armv7-a")
target_compile_options(Main PUBLIC "-fPIC")
target_compile_options(Main PUBLIC "-march=armv7-a")
target_compile_options(Main PUBLIC "-mfloat-abi=softfp")


SET(CMAKE_SYSROOT ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core)

target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/tools/arm-linux-gnueabi-gcc-9.2/arm-tizen-linux-gnueabi/include/c++/9.2.0/)
target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/tools/arm-linux-gnueabi-gcc-9.2/arm-tizen-linux-gnueabi/include/c++/9.2.0/arm-tizen-linux-gnueabi/)
target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include)
target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/network)
target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/nnstreamer)
target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/dlog)
target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/appfw)
target_include_directories(Main PUBLIC ${TIZEN_STUDIO_DIR}/platforms/tizen-6.5/wearable/rootstraps/wearable-6.5-device.core/usr/include/appcore-agent)

target_include_directories(Main PUBLIC ${FLUTTER_TIZEN_DIR}/embedding/cpp/include)
target_include_directories(Main PUBLIC ${FLUTTER_TIZEN_DIR}/flutter/bin/cache/artifacts/engine/tizen-common/cpp_client_wrapper/include)
target_include_directories(Main PUBLIC ${FLUTTER_TIZEN_DIR}/flutter/bin/cache/artifacts/engine/tizen-common/public)


set(
    lib_paths

    lib/armel
    /home/owner/unit_tests_flutter_blue/lib/armel
)
set(
    libraries

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



target_link_directories(Main PUBLIC ${lib_paths})


target_link_libraries(Main ${libraries})


SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
