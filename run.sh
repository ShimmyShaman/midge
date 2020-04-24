#!/bin/bash

echo
echo Compiling

# # export PKG_CONFIG_PATH='/usr/local/lib64/pkgconfig/'
# # g++ -g -o ./bin/midge                                           \
# #     `pkg-config --cflags glfw3`                                 \
# # -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl   \
# #     -I./src                                                     \
# #     -I/home/daniel/cling/src/include                            \
# #     -I/home/daniel/cling/src/tools/cling/include                \
# #     -I/home/daniel/cling/src/tools/clang/include                \
# #     -I/home/daniel/cling/obj/include                            \
# #     /home/daniel/cling/obj/lib/libcling.so                      \
# #     /home/daniel/cling/obj/lib/libcling.so.5                    \
# #     /home/daniel/cling/obj/lib/libLLVMSupport.a                 \
# #     $(find . -type f -iregex ".*\.cpp")                         \
# #     `pkg-config --static --libs glfw3`
# # export PKG_CONFIG_PATH='/usr/local/lib64/pkgconfig/'
# # -I/home/jason/cling/src/include                  \
# # -I/home/jason/cling/src/tools/cling/include      \
# # -I/home/jason/cling/src/tools/clang/include      \     #/usr/local/lib64'
# export LD_LIBRARY_PATH='/home/jason/cling/obj/lib'
#     -I/home/daniel/cling/src/include                            \
#     -I/home/daniel/cling/src/tools/cling/include                \
#     -I/home/daniel/cling/src/tools/clang/include                \
# /home/jason/cling/obj/lib/libcling.so            \
# /home/jason/cling/obj/lib/libLLVMSupport.a                 \
# -L/home/jason/cling/inst/bin                 \
export LD_LIBRARY_PATH='/home/jason/cling/inst/lib'
g++ src/main/main.cpp                             \
-Isrc                                             \
-I/home/jason/cling/inst/include                  \
/home/jason/cling/obj/lib/libcling.so             \
-o bin/midge

# # export LD_LIBRARY_PATH='/home/daniel/cling/obj/lib;/home/daniel/midge/dep'
# # g++ -fPIC src/main/main.cpp                           \
# #     -I/home/daniel/cling/src/include                  \
# #     -I/home/daniel/cling/src/tools/cling/include      \
# #     -I/home/daniel/cling/src/tools/clang/include      \
# #     -I/home/daniel/cling/obj/include                  \
# #     /home/daniel/cling/obj/lib/libcling.so            \
# #     -o bin/midge

# g++                                                       \
#     src/vulkan.cpp                                     \
#     -lvulkan \
#     -o bin/vulkanexp
retval=$?
if [ $retval -ne 0 ]; then
    echo "Compilation Failed : $retval"
else
    echo Running
    echo "#######################"
    ./bin/midge
    # chmod u=rwx ./bin/vulkanexp
    # ./bin/vulkanexp
    retval=$?
    echo "#######################"
    echo "Program Exited: $retval"
fi