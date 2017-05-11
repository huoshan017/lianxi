#!/bin/sh
./db_generator
mv ./db_field_struct.proto ../../../proto
mv ./*.h ./*.cpp ../../db_server
