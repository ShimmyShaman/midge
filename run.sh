#!/bin/bash


echo
echo Compiling
g++ -o ./bin/midge -lX11 -I./src ./src/main.cpp

retval=$?
if [ $retval -ne 0 ]; then
    echo "Compilation Failed : $retval"
else
	echo Running
    ./bin/midge
    retval=$?
    echo "Program Exited: $retval"
fi