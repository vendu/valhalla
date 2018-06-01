#! /bin/sh

CFGLAGS="-DVASDEBUG=1 -g"
CWARNFLAGS="-Wall -Wundef -Wextra"
INCLUDES="-I.. -I../.."

gcc -DVASDEBUG=1 -g -Wall -Wundef -Wextra -I.. -I../..  -O -o vm *.c ../vas/*.c

