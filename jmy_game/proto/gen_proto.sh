#!/bin/sh
#export LD_LIBRARY_PATH=../servers/libjmy/thirdparty/lib/protobuf
mkdir -p src
../servers/thirdparty/bin/protoc --cpp_out=./src ./*.proto
