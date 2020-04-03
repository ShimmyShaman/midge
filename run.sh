#!/bin/bash


echo
echo Compiling
g++ -o ./bin/midge -lX11 ./src/main.cpp

echo Running
./bin/midge
#echo pass | sudo -S ./bin/factai
