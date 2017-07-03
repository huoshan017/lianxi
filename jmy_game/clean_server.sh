#!/bin/sh
rm proto/db_field_struct.proto
rm servers/db_server/db_tables_define.h
rm servers/db_server/db_tables_func.*
rm servers/db_server/db_tables_struct.*
rm servers/game_server/csv_list_manager.h
rm -fr servers/game_server/csv
rm ./servers/bin/db_generator/db_generator
rm ./servers/bin/csv_generator/csv_generator
cd ./servers
make clean
