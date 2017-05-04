#include "mysql_config_loader.h"
#include "mysql_defines.h"
#include "mysql_connector.h"
#include "mysql_util.h"
#include "../common/util.h"
#include <cstring>

MysqlConfigLoader::MysqlConfigLoader(MysqlConnector* connector) : connector_(connector)
{
}

MysqlConfigLoader::~MysqlConfigLoader()
{
	clear();
}

bool MysqlConfigLoader::load(const MysqlDatabaseConfig& config)
{
	if (!connector_->create_db(config.dbname)) {
		LogError("create database %s failed", config.dbname);
		return false;
	}
	int i = 0;
	for (; i<config.tables_num; ++i) {
		if (!loadTable(config.tables_info[i])) {
			LogError("load table %s config failed", config.tables_info[i].name);
			return false;
		}
	}

	LogInfo("load database %s config success", config.dbname);
	return true;
}

void MysqlConfigLoader::clear()
{
}

bool MysqlConfigLoader::loadTable(const MysqlTableInfo& table_info)
{
	if (table_info.fields_num<=table_info.primary_key_index || table_info.primary_key_index<0) {
		LogError("primary_key_index %d invalid", table_info.primary_key_index);
		return false;
	}
	if (std::strcmp(table_info.fields_info[table_info.primary_key_index].name, table_info.primary_key) != 0) {
		LogError("primary_key(%d) is not same to index name", table_info.primary_key);
		return false;
	}

	const MysqlTableFieldInfo& primary_table_field = table_info.fields_info[table_info.primary_key_index];
	const char* primary_field_type = get_field_type(primary_table_field);
	const char* primary_field_create_flags = get_field_create_flags(primary_table_field);
	char* engine_type = (char*)"";
	if (table_info.engine_type == MYSQL_ENGINE_MYISAM)
		engine_type = (char*)"MyISAM";
	else
		engine_type = (char*)"InnoDB";

	std::snprintf(buf_, sizeof(buf_), "CREATE TABLE IF NOT EXISTS `%s` (`%s` %s %s, primary key(`%s`)) ENGINE=%s",
			table_info.name,
			table_info.primary_key,
			primary_field_type,
			primary_field_create_flags,
			table_info.primary_key,
			engine_type);

	if (!connector_->real_query(buf_, strlen(buf_))) {
		LogError("create table %s failed", table_info.name);
		return false;
	}
	
	int i = 0;
	for (; i<table_info.fields_num; ++i) {
		if (i == table_info.primary_key_index)
			continue;
		if (!add_field(table_info.name, table_info.fields_info[i])) {
			LogError("table %s add field %s failed", table_info.name, table_info.fields_info[i].name);
			return false;
		}
	}

	LogInfo("load table %s success", table_info.name);
	return true;
}

bool MysqlConfigLoader::dropTable(const char* table_name)
{
	std::snprintf(buf_, sizeof(buf_), "DROP TABLE %s", table_name);
	if (!connector_->real_query(buf_, strlen(buf_))) {
		LogError("drop table %s failed", table_name);
		return false;
	}
	LogInfo("droped table %s", table_name);
	return true;
}

const char* MysqlConfigLoader::get_field_type(const MysqlTableFieldInfo& field_info)
{
	static char buf[64];
	switch (field_info.field_type) {
	case MYSQL_FIELD_TYPE_TINYINT:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "TINYINT(%d)", MYSQL_FIELD_DEFAULT_LENGTH_TINYINT);
			else
				std::snprintf(buf, sizeof(buf), "TINYINT(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_SMALLINT:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "SMALLINT(%d)", MYSQL_FIELD_DEFAULT_LENGTH_SMALLINT);
			else
				std::snprintf(buf, sizeof(buf), "SMALLINT(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_MEDIUMINT:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "MEDIUMINT(%d)", MYSQL_FIELD_DEFAULT_LENGTH_MEDIUMINT);
			else
				std::snprintf(buf, sizeof(buf), "MEDIUMINT(%d)", field_info.type_length);	
		}
		break;
	case MYSQL_FIELD_TYPE_INT:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "INT(%d)", MYSQL_FIELD_DEFAULT_LENGTH_INT);
			else
				std::snprintf(buf, sizeof(buf), "INT(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_BIGINT:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "BIGINT(%d)", MYSQL_FIELD_DEFAULT_LENGTH_BIGINT);
			else
				std::snprintf(buf, sizeof(buf), "BIGINT(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_FLOAT:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "FLOAT(%d)", MYSQL_FIELD_DEFAULT_LENGTH_FLOAT);
			else
				std::snprintf(buf, sizeof(buf), "FLOAT(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_DOUBLE:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "DOUBLE(%d)", MYSQL_FIELD_DEFAULT_LENGTH_DOUBLE);
			else
				std::snprintf(buf, sizeof(buf), "DOUBLE(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_DATE:
		return "DATE";
	case MYSQL_FIELD_TYPE_DATETIME:
		return "DATETIME";
	case MYSQL_FIELD_TYPE_TIMESTAMP:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				return "TIMESTAMP";
				//std::snprintf(buf, sizeof(buf), "TIMESTAMP(%d)", MYSQL_FIELD_DEFAULT_LENGTH_TIMESTAMP);
			else
				std::snprintf(buf, sizeof(buf), "TIMESTAMP(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_YEAR:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "YEAR(%d)", MYSQL_FIELD_DEFAULT_LENGTH_YEAR);
			else
				std::snprintf(buf, sizeof(buf), "YEAR(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_CHAR:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "CHAR(%d)", MYSQL_FIELD_DEFAULT_LENGTH_CHAR);
			else
				std::snprintf(buf, sizeof(buf), "CHAR(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_VARCHAR:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "VARCHAR(%d)", MYSQL_FIELD_DEFAULT_LENGTH_VARCHAR);
			else
				std::snprintf(buf, sizeof(buf), "VARCHAR(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_TINYBLOB:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "TINYBLOB(%d)", MYSQL_FIELD_DEFAULT_LENGTH_TINYBLOB);
			else
				std::snprintf(buf, sizeof(buf), "TINYBLOB(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_TINYTEXT:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "TINYTEXT(%d)", MYSQL_FIELD_DEFAULT_LENGTH_TINYTEXT);
			else
				std::snprintf(buf, sizeof(buf), "TINYTEXT(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_MEDIUMBLOB:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "MEDIUMBLOB(%d)", MYSQL_FIELD_DEFAULT_LENGTH_MEDIUMBLOB);
			else
				std::snprintf(buf, sizeof(buf), "MEDIUMBLOB(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_MEDIUMTEXT:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "MEDIUMTEXT(%d)", MYSQL_FIELD_DEFAULT_LENGTH_MEDIUMTEXT);
			else
				std::snprintf(buf, sizeof(buf), "MEDIUMTEXT(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_BLOB:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "BLOB(%d)", MYSQL_FIELD_DEFAULT_LENGTH_BLOB);
			else
				std::snprintf(buf, sizeof(buf), "BLOB(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_TEXT:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "TEXT(%d)", MYSQL_FIELD_DEFAULT_LENGTH_TEXT);
			else
				std::snprintf(buf, sizeof(buf), "TEXT(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_LONGBLOB:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "LONGBLOB(%d)", MYSQL_FIELD_DEFAULT_LENGTH_LONGBLOB);
			else
				std::snprintf(buf, sizeof(buf), "LONGBLOB(%d)", field_info.type_length);
		}
		break;
	case MYSQL_FIELD_TYPE_LONGTEXT:
		{
			if (field_info.type_length == MYSQL_FIELD_DEFAULT_LENGTH)
				std::snprintf(buf, sizeof(buf), "LONGTEXT(%d)", MYSQL_FIELD_DEFAULT_LENGTH_LONGTEXT);
			else
				std::snprintf(buf, sizeof(buf), "LONGTEXT(%d)", field_info.type_length);
		}
		break;
	default:
		break;
	}
	return buf;
}

const char* MysqlConfigLoader::get_field_create_flags(const MysqlTableFieldInfo& field_info)
{
	struct CreateFlagsInfo {
		MysqlTableCreateFlag flag;
		const char* flag_str;
	};
	static CreateFlagsInfo s_flags_info[] = {
		{ MYSQL_TABLE_CREATE_ZEROFILL, "ZEROFILL" },
		{ MYSQL_TABLE_CREATE_UNSIGNED, "UNSIGNED" },
		{ MYSQL_TABLE_CREATE_AUTOINCREMENT, "AUTO_INCREMENT" },
		{ MYSQL_TABLE_CREATE_NULL, "NULL" },
		{ MYSQL_TABLE_CREATE_NOT_NULL, "NOT NULL" },
		{ MYSQL_TABLE_CREATE_DEFAULT, "DEFAULT" }
	};
	
	static char buf[2][128] = { {0}, {0} };
	buf[0][0] = '\0'; buf[1][0] = '\0';
	size_t i = 0;
	size_t s = sizeof(s_flags_info)/sizeof(s_flags_info[0]);
	int bi = -1;
	char* pbuf = (char*)"";
	for (; i<s; ++i) {
		if (field_info.create_flags & s_flags_info[i].flag) {
			bi += 1;
			if (bi >= (int)(sizeof(buf)/sizeof(buf[0]))) {
				bi = 0;
			}
			if (s_flags_info[i].flag == MYSQL_TABLE_CREATE_DEFAULT) {
				if (IS_MYSQL_INT_TYPE(field_info.field_type)) {
					std::snprintf(buf[bi], sizeof(buf[bi]), "%s %s 0", pbuf, s_flags_info[i].flag_str);
				} else if (IS_MYSQL_TEXT_TYPE(field_info.field_type)) {
					std::snprintf(buf[bi], sizeof(buf[bi]), "%s %s ''", pbuf, s_flags_info[i].flag_str);
				} else if (field_info.field_type == MYSQL_FIELD_TYPE_TIMESTAMP) {
					std::snprintf(buf[bi], sizeof(buf[bi]), "%s %s CURRENT_TIMESTAMP", pbuf, s_flags_info[i].flag_str);
				}
			} else {
				std::snprintf(buf[bi], sizeof(buf[bi]), "%s %s", pbuf, s_flags_info[i].flag_str);
			}
			pbuf = buf[bi];
		}
	}
	LogInfo("field %s create flags: %s", field_info.name, buf[bi]);
	return buf[bi];
}

const char* MysqlConfigLoader::get_field_index_type(const MysqlTableFieldInfo& field_info)
{
	struct IndexInfo {
		MysqlTableIndexType index;
		const char* index_str;
	};
	static IndexInfo s_index_info[] = {
		{ MYSQL_INDEX_TYPE_NONE, "" },
		{ MYSQL_INDEX_TYPE_NORMAL, "index" },
		{ MYSQL_INDEX_TYPE_UNIQUE, "unique" }
	};

	char* index_str = nullptr;
	size_t i = 0;
	for (; i<sizeof(s_index_info)/sizeof(s_index_info[0]); ++i) {
		if (s_index_info[i].index == field_info.index_type) {
			index_str = (char*)s_index_info[i].index_str;
			break;
		}
	}
	return index_str;
}

bool MysqlConfigLoader::add_field(const char* table_name, const MysqlTableFieldInfo& field_info)
{
	std::snprintf(buf_, sizeof(buf_), "DESCRIBE %s %s", table_name, field_info.name);
	if (!connector_->real_read_query(buf_, strlen(buf_))) {
		LogError("describe %s %s failed", table_name, field_info.name);
		return false;
	}

	MysqlConnector::Result& res = const_cast<MysqlConnector::Result&>(connector_->get_result());
	if (res.fetch()) {
		LogInfo("table %s field(%s) exists", table_name, field_info.name);
		return true;
	}

	const char* field_type_str = get_field_type(field_info);
	const char* field_create_str = get_field_create_flags(field_info);
	std::snprintf(buf_, sizeof(buf_), "ALTER TABLE `%s` ADD COLUMN `%s` %s %s", table_name, field_info.name, field_type_str, field_create_str);
	if (!connector_->real_query(buf_, strlen(buf_))) {
		LogError("add field(%s) to table(%s) failed", field_info.name, table_name);
		return false;
	}

	// create index
	const char* index_type_str = get_field_index_type(field_info);
	if (std::memcmp(index_type_str, "index", std::strlen("index")) == 0) {
		if (IS_MYSQL_TEXT_TYPE(field_info.field_type) || IS_MYSQL_BINARY_TYPE(field_info.field_type)) {
			int l = mysql_field_default_length(field_info.field_type);
			if (l < 0) {
				LogError("field_type(%d) default length invalid", field_info.field_type);
				return false;
			}
			std::snprintf(buf_, sizeof(buf_), "ALTER TABLE `%s` ADD INDEX %s_index ON %s(%d)",
					table_name, field_info.name, field_info.name, l);
		} else {
			std::snprintf(buf_, sizeof(buf_), "ALTER TABLE `%s` ADD INDEX %s_index ON %s",
					table_name, field_info.name, field_info.name);
		}
	} else if (std::memcmp(index_type_str, "unique", std::strlen("unique")) == 0) {
		std::snprintf(buf_, sizeof(buf_), "ALTER TABLE `%s` ADD UNIQUE ON %s", table_name, field_info.name);
	} else {
		LogError("not supported index type(%s)", index_type_str);
		return false;
	}
	if (!connector_->real_query(buf_, strlen(buf_))) {
		LogError("add field(%s) to table(%s) failed", field_info.name, table_name);
		return false;
	}
	
	LogInfo("table(%s) add field(%s) success", table_name, field_info.name);
	return true;
}

bool MysqlConfigLoader::remove_field(const char* table_name, const char* field_name)
{
	std::snprintf(buf_, sizeof(buf_), "ALTER TABLE `%s` DROP COLUMN `%s`", table_name, field_name);
	if (!connector_->real_query(buf_, strlen(buf_))) {
		LogError("remove field(%s) for table(%s) failed", field_name, table_name);
		return false;
	}
	LogInfo("table(%s) remove field(%s) success", table_name, field_name);
	return true;
}

bool MysqlConfigLoader::rename_field(const char* table_name, const char* old_field_name, const char* new_field_name)
{
	std::snprintf(buf_, sizeof(buf_), "ALTER TABLE `%s` CHANGE `%s` `%s`", table_name, old_field_name, new_field_name);
	if (!connector_->real_query(buf_, strlen(buf_))) {
		LogError("change table(%s) field name(%s) to name(%s)", table_name, old_field_name, new_field_name);
		return false;
	}
	LogInfo("table(%s) field name(%s) be changed to name(%s)", table_name, old_field_name, new_field_name);
	return true;
}

bool MysqlConfigLoader::modify_field_attr(const char* table_name, const MysqlTableFieldInfo& field_info)
{
	const char* field_type_str = get_field_type(field_info);
	const char* field_create_str = get_field_create_flags(field_info);
	std::snprintf(buf_, sizeof(buf_), "ALTER TABLE `%s` MODIFY `%s` %s %s", table_name, field_info.name, field_type_str, field_create_str);
	if (!connector_->real_query(buf_, strlen(buf_))) {
		LogError("modify table(%s) field(%s) attr failed", table_name, field_info.name);
		return false;
	}
	LogInfo("table(%s) field(%s) attr modified", table_name, field_info.name);
	return true;
}
