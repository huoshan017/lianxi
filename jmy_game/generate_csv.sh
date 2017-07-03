#!/bin/sh
cd servers/csv_generator
make
cd ../bin/csv_generator
./csv_generator ../game_server/csv

mv csv_list_manager.h ../../game_server/
mkdir -p ../../game_server/csv
mv *.h ../../game_server/csv
