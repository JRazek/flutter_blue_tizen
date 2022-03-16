#!/bin/bash
ADDRESS=10.0.0.5
PORT=2002
DEST_PATH=/opt/usr/globalapps/com.example.flutter_blue_tizen_example
# DEST_PATH=/home/owner/unit_tests_flutter_blue/bin

./upload.sh
sdb shell gdbserver 127.0.0.1:$PORT $DEST_PATH/bin/Main &
gdb-multiarch --eval-command="target remote tcp:$ADDRESS:$PORT"