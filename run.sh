#!/bin/bash

    #/home/daniel/cling/obj/lib/libcling.so  \
    #/home/daniel/cling/lib/libcling.so.5  \
    #/home/daniel/cling/obj/lib/libcling.so.5.0.0  \
    #-L/home/daniel/cling/obj/lib                                \

echo
echo Compiling
#g++ -Iglfw/glad/glad.h glfw/glad/glad.cpp -fPIC -o ./bin/glad.o

# export PKG_CONFIG_PATH='/usr/local/lib64/pkgconfig/'
# g++ -g -o ./bin/midge                                           \
#     `pkg-config --cflags glfw3`                                 \
#     -lGL -lX11 -lpthread -lXrandr -lXi -ldl                     \
#     -I./src                                                     \
#     -I/home/daniel/cling/src/include                            \
#     -I/home/daniel/cling/src/tools/cling/include                \
#     -I/home/daniel/cling/src/tools/clang/include                \
#     -I/home/daniel/cling/obj/include                            \
#     /home/daniel/cling/obj/lib/libcling.so                      \
#     /home/daniel/cling/obj/lib/libcling.so.5                    \
#     /home/daniel/cling/obj/lib/libLLVMSupport.a                 \
#     $(find . -type f -iregex ".*\.cpp")                         \
#     `pkg-config --static --libs glfw3`
export LD_LIBRARY_PATH='/home/daniel/cling/obj/lib;/usr/local/lib64'
export PKG_CONFIG_PATH='/usr/local/lib64/pkgconfig/'
g++ -g -fPIC src/main/main.cpp                           \
    src/opengl/glad/glad.cpp                          \
    -I./src                                           \
    -I./src/opengl/glad.h                             \
    -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl   \
    -I/home/daniel/cling/src/include                  \
    -I/home/daniel/cling/src/tools/cling/include      \
    -I/home/daniel/cling/src/tools/clang/include      \
    -I/home/daniel/cling/obj/include                  \
    /home/daniel/cling/obj/lib/libcling.so            \
    -o bin/midge               
    #`pkg-config --cflags glfw3` \                   
    #`pkg-config --libs glfw3`
    # ./dep/glad.o \

# export LD_LIBRARY_PATH='/home/daniel/cling/obj/lib;/home/daniel/midge/dep'
# g++ -fPIC src/main/main.cpp                           \
#     -I/home/daniel/cling/src/include                  \
#     -I/home/daniel/cling/src/tools/cling/include      \
#     -I/home/daniel/cling/src/tools/clang/include      \
#     -I/home/daniel/cling/obj/include                  \
#     /home/daniel/cling/obj/lib/libcling.so            \
#     -o bin/midge

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