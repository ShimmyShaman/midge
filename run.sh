#!/bin/bash


echo
echo Compiling
g++ -o ./bin/midge -lX11 -lpthread -I./src $(find . -type f -iregex ".*\.cpp") 

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