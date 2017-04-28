#!/bin/sh
./db_struct_generator
mv db_struct_struct.h db_struct_defines.h ../../db_server
