#!/bin/bash

DEST_PATH=/opt/usr/globalapps/com.example.flutter_blue_tizen_example

mkdir -p build
rm -rf ./build/*
cd ./build
cmake ..
make VERBOSE=1
cd ..
sdb wait-for-device
sdb root on
sdb shell mkdir -p $DEST_PATH/bin/
sdb push build/Main $DEST_PATH/bin/runner

if [[ $1 != '' ]] ; then
    sdb push lib $DEST_PATH/lib
fi