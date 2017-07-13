#!/bin/sh
cd ./servers/bin/config_server
mkdir -p log
nohup ./config_server>/dev/null 2>&1 &
sleep 1
cd ../login_server
mkdir -p log
nohup ./login_server>/dev/null 2>&1 &
sleep 1
cd ../gate_server
mkdir -p log
nohup ./gate_server>/dev/null 2>&1 &
sleep 1
cd ../db_server
mkdir -p log
nohup ./db_server>/dev/null 2>&1 &
sleep 1
cd ../game_server
mkdir -p log
nohup ./game_server>/dev/null 2>&1 &
