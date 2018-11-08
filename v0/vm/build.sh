#! /bin/sh

#CFGLAGS="-DVASDEBUG=1 -g"
#CWARNFLAGS="-Wall -Wundef -Wextra"
#INCLUDES="-I.. -I../.."

gcc -O0 -g -Wall -Wundef -Wextra -I../.. -I../../.. -o vm *.c ../../vas/*.c

