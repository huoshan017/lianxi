#!/bin/sh
cd ./servers/bin/config_server
nohup ./config_server &
cd ../login_server
nohup ./login_server &
cd ../gate_server
nohup ./gate_server &
cd ../db_server
nohup ./db_server &
cd ../game_server
nohup ./game_server &
