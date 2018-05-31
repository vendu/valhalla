#! /bin/sh

CFGLAGS="-DVASDEBUG=1 -g"
CWARNFLAGS="-Wall -Wundef -Wextra"
INCLUDES="-I.. -I../.."

gcc $CFLAGS $CWARNFLAGS $INCLUDES  -O -o vm *.c ../vas/*.c

