#! /bin/sh

gcc -DVASDEBUG=1 -g -Wall -Wundef -I.. -I../.. -O -o vm *.c ../vas/*.c

