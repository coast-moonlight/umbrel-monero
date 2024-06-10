#!/bin/bash
gcc -O0 -Wall -o spectre ../common/common.c victim.c spectre.c -I../common
