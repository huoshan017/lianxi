#!/bin/sh
export LD_LIBRARY_PATH=../servers/libjmy/thirdparty/lib/protobuf
../servers/libjmy/thirdparty/bin/protoc --cpp_out=./ ./common.proto
