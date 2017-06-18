#!/bin/sh
#export LD_LIBRARY_PATH=../servers/libjmy/thirdparty/lib/protobuf
mkdir -p src
../servers/thirdparty/bin/protoc --cpp_out=./src ./*.proto
mkdir -p ../client/network_test/network_test/proto
../servers/thirdparty/bin/protoc --csharp_out=../client/network_test/network_test/proto ./common.proto ./error.proto
