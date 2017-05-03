#!/bin/sh
cd ./proto
./gen_proto.sh
cd ..
cd ./servers/db_struct_generator
make
cd ../..
cd ./servers/bin/db_struct_generator
./generate_files.sh
cd ../../..
cd ./servers
make
