#! /bin/sh

CFGLAGS="-DVASDEBUG=1 -g"
CWARNFLAGS="-Wall -Wundef -Wextra"
INCLUDES="-I.. -I../.."

gcc -g -Wall -Wundef -Wextra -I.. -I../..  -O -o vm *.c ../vas/*.c

