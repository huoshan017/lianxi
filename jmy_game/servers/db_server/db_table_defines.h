#pragma once

#include "mysql_defines.h"

static const MysqlTableFieldInfo s_jmy_game_account_table_fields_info[] = {
	{ "id",				MYSQL_FIELD_TYPE_INT,		MYSQL_FIELD_DEFAULT_LENGTH,	MYSQL_INDEX_TYPE_NONE,		MYSQL_TABLE_CREATE_AUTOINCREMENT|MYSQL_TABLE_CREATE_UNSIGNED|MYSQL_TABLE_CREATE_NOT_NULL },
	{ "account",		MYSQL_FIELD_TYPE_VARCHAR,	32,							MYSQL_INDEX_TYPE_NORMAL,	MYSQL_TABLE_CREATE_NOT_NULL|MYSQL_TABLE_CREATE_DEFAULT },
	{ "create_time",	MYSQL_FIELD_TYPE_TIMESTAMP,	MYSQL_FIELD_DEFAULT_LENGTH, MYSQL_INDEX_TYPE_NONE,		MYSQL_TABLE_CREATE_NOT_NULL|MYSQL_TABLE_CREATE_DEFAULT },
	{ "last_login",		MYSQL_FIELD_TYPE_TIMESTAMP,	MYSQL_FIELD_DEFAULT_LENGTH, MYSQL_INDEX_TYPE_NONE,		MYSQL_TABLE_CREATE_NOT_NULL },
	{ "last_logout",	MYSQL_FIELD_TYPE_TIMESTAMP,	MYSQL_FIELD_DEFAULT_LENGTH,	MYSQL_INDEX_TYPE_NONE,		MYSQL_TABLE_CREATE_NOT_NULL },
};

static const MysqlTableInfo s_jmy_game_account_table_info = {
	"account",
	0,
	"id",
	s_jmy_game_account_table_fields_info,
	sizeof(s_jmy_game_account_table_fields_info)/sizeof(s_jmy_game_account_table_fields_info[0]),
	MYSQL_ENGINE_INNODB
};

static const MysqlTableFieldInfo s_jmy_game_player_table_fields_info[] = {
	{ "id",			MYSQL_FIELD_TYPE_INT,		MYSQL_FIELD_DEFAULT_LENGTH, MYSQL_INDEX_TYPE_NONE,		MYSQL_TABLE_CREATE_AUTOINCREMENT|MYSQL_TABLE_CREATE_UNSIGNED },
	{ "uid",		MYSQL_FIELD_TYPE_BIGINT,	MYSQL_FIELD_DEFAULT_LENGTH, MYSQL_INDEX_TYPE_NORMAL,	MYSQL_TABLE_CREATE_UNSIGNED|MYSQL_TABLE_CREATE_NOT_NULL },
	{ "account",	MYSQL_FIELD_TYPE_VARCHAR,	32,							MYSQL_INDEX_TYPE_NORMAL,	MYSQL_TABLE_CREATE_NOT_NULL },
	{ "nick_name",	MYSQL_FIELD_TYPE_VARCHAR,	32,							MYSQL_INDEX_TYPE_NONE,		MYSQL_TABLE_CREATE_DEFAULT },
	{ "level",		MYSQL_FIELD_TYPE_TINYINT,	MYSQL_FIELD_DEFAULT_LENGTH,	MYSQL_INDEX_TYPE_NONE,		MYSQL_TABLE_CREATE_DEFAULT },
	{ "exp",		MYSQL_FIELD_TYPE_INT,		MYSQL_FIELD_DEFAULT_LENGTH,	MYSQL_INDEX_TYPE_NONE,		MYSQL_TABLE_CREATE_DEFAULT },
};

static const MysqlTableInfo s_jmy_game_player_table_info = {
	"player",
	0,
	"id",
	s_jmy_game_player_table_fields_info,
	sizeof(s_jmy_game_player_table_fields_info)/sizeof(s_jmy_game_player_table_fields_info[0]),
	MYSQL_ENGINE_INNODB
};

static const MysqlTableInfo s_jmy_game_tables[] = {
	s_jmy_game_account_table_info,
	s_jmy_game_player_table_info
};

static const MysqlDatabaseConfig s_jmy_game_db_config = {
	"jmy_game",
	s_jmy_game_tables,
	sizeof(s_jmy_game_tables)/sizeof(s_jmy_game_tables[0])
};
