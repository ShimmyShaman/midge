#!/bin/bash


echo
echo Compiling
export PKG_CONFIG_PATH='/usr/local/lib64/pkgconfig/'
g++ -o ./bin/midge `pkg-config --cflags glfw3` -lGL -lX11 -lpthread -lXrandr -lXi -ldl -I./src $(find . -type f -iregex ".*\.cpp") `pkg-config --static --libs glfw3`

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