#!/bin/bash

    #/home/daniel/cling/obj/lib/libcling.so  \
    #/home/daniel/cling/lib/libcling.so.5  \
    #/home/daniel/cling/obj/lib/libcling.so.5.0.0  \
    #-L/home/daniel/cling/obj/lib                                \

echo
echo Compiling
export LD_LIBRARY_PATH='/home/daniel/cling/obj/lib'
export PKG_CONFIG_PATH='/usr/local/lib64/pkgconfig/'
g++ -g -o ./bin/midge                                           \
    `pkg-config --cflags glfw3`                                 \
    -lGL -lX11 -lpthread -lXrandr -lXi -ldl                     \
    -I./src                                                     \
    -I/home/daniel/cling/src/include                            \
    -I/home/daniel/cling/src/tools/cling/include                \
    -I/home/daniel/cling/obj/include                            \
    /home/daniel/cling/obj/lib/libcling.so                      \
    /home/daniel/cling/obj/lib/libcling.so.5                    \
    $(find . -type f -iregex ".*\.cpp")                         \
    `pkg-config --static --libs glfw3`

retval=$?
if [ $retval -ne 0 ]; then
    echo "Compilation Failed : $retval"
else
	echo Running
    echo "#######################"
    ./bin/midge
    retval=$?
    echo "#######################"
    echo "Program Exited: $retval"
fi