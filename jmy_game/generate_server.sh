#!/bin/sh

./generate_csv.sh

cd ./servers/db_generator
make
cd ../..
cd ./servers/bin/db_generator
./generate_files.sh

cd ../../../proto
./gen_proto.sh

cd ../servers
make
