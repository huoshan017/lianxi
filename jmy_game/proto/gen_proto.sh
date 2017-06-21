#!/bin/sh
#export LD_LIBRARY_PATH=../servers/libjmy/thirdparty/lib/protobuf
mkdir -p src
../servers/thirdparty/bin/protoc --cpp_out=./src ./*.proto
../servers/thirdparty/bin/protoc --csharp_out=../client/network_test/network_test ./common.proto ./error.proto
