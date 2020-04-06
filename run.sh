#!/bin/bash


echo
echo Compiling
g++ -o ./bin/midge -lX11 -I./src ./src/main.cpp

echo Running
./bin/midge
#echo pass | sudo -S ./bin/factai
