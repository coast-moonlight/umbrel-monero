#!/bin/bash
gcc -o sender sender.c ../common/common.c -I../common
gcc -o receiver receiver.c ../common/common.c -I../common
