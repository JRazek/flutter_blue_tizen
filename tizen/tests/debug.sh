#!/bin/bash
ADDRESS=10.0.0.6
PORT=2002

./upload.sh
sdb shell gdbserver 127.0.0.1:$PORT /home/owner/unit_tests_flutter_blue/bin/Main &
gdb-multiarch --eval-command="target remote tcp:$ADDRESS:$PORT"