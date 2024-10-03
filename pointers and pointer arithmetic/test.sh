#!/bin/bash

# Trigger all your test cases with this script

make all

# ./vr2017 ./binary_files/01.bin 0xFF 0xAA 0x11
./vr2017 ./binary_files/02.bin 0xFF 0xAA 0x11



make clean