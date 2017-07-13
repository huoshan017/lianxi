#!/bin/sh

for s in config_server login_server gate_server db_server game_server
do
	killall $s
	if [ $? -eq 0 ];then
		echo "$s stoped"
	else
		echo "$s stop failed"
	fi
done
