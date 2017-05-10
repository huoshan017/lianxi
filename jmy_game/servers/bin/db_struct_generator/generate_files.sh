#!/bin/sh
./db_struct_generator
mv db_struct_structs.h db_struct_defines.h db_struct_funcs.* ../../db_server
