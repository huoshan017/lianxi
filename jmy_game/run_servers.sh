#!/bin/sh
cd ./servers/bin/config_server
mkdir -p log
nohup ./config_server>/dev/null 2>&1 &
if [ $? -eq 0 ];then
	echo "config_server started"
else
	echo "config_server start failed"
fi
sleep 1

cd ../login_server
mkdir -p log
nohup ./login_server>/dev/null 2>&1 &
if [ $? -eq 0 ];then
	echo "login_server started"
else
	echo "login_server start failed"
fi
sleep 1

cd ../gate_server
mkdir -p log
nohup ./gate_server>/dev/null 2>&1 &
if [ $? -eq 0 ];then
	echo "gate_server started"
else
	echo "gate_server start failed"
fi
sleep 1

cd ../db_server
mkdir -p log
nohup ./db_server>/dev/null 2>&1 &
if [ $? -eq 0 ];then
	echo "db_server started"
else
	echo "db_server start failed"
fi
sleep 1

cd ../game_server
mkdir -p log
nohup ./game_server>/dev/null 2>&1 &
if [ $? -eq 0 ];then
	echo "game_server started"
else
	echo "game_server start failed"
fi
