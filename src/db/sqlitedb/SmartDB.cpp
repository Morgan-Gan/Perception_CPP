#include "SmartDB.h"

using namespace std;
using namespace SQLITE_DB;
using namespace common_cmmobj;

std::unordered_map<int, std::function<SqliteValue(sqlite3_stmt*, int)>> CSmartDB::m_valMap =
{ 
	{ std::make_pair(SQLITE_INTEGER, [](sqlite3_stmt* stmt,int index) {return sqlite3_column_int64(stmt, index);}) }
	,{ std::make_pair(SQLITE_FLOAT, [](sqlite3_stmt* stmt, int index) {return sqlite3_column_double(stmt, index);}) }
	,{ std::make_pair(SQLITE_BLOB, [](sqlite3_stmt* stmt, int index) {return string((const char*)sqlite3_column_blob(stmt, index));})}
	,{ std::make_pair(SQLITE_TEXT, [](sqlite3_stmt* stmt, int index) {return string((const char*)sqlite3_column_text(stmt, index));})}
	,{ std::make_pair(SQLITE_NULL, [](sqlite3_stmt* stmt, int index) {return nullptr;}) }
};

std::unordered_map<int, std::function<void(sqlite3_stmt *stmt, int index, JsonBuilder&)>> CSmartDB::m_builderMap =
{ 
	{ std::make_pair(SQLITE_INTEGER, [](sqlite3_stmt* stmt, int index, JsonBuilder& builder) {builder.Int64(sqlite3_column_int64(stmt, index)); }) },
	 { std::make_pair(SQLITE_FLOAT, [](sqlite3_stmt* stmt, int index, JsonBuilder& builder) {builder.Double(sqlite3_column_double(stmt, index)); }) },
	{ std::make_pair(SQLITE_BLOB, [](sqlite3_stmt* stmt, int index, JsonBuilder& builder) {builder.String((const char*)sqlite3_column_blob(stmt, index)); }) },
	 { std::make_pair(SQLITE_TEXT, [](sqlite3_stmt* stmt, int index, JsonBuilder& builder) {builder.String((const char*)sqlite3_column_text(stmt, index)); }) },
	{ std::make_pair(SQLITE_NULL, [](sqlite3_stmt* stmt, int index, JsonBuilder& builder) {builder.Null(); }) }
};

CSmartDB::CSmartDB(const string& strFileName):
m_dbHandle(nullptr),
m_statement(nullptr)
{
	Open(strFileName);
}

bool CSmartDB::Begin()
{
	return Excecute("begin transaction");
}

bool CSmartDB::RollBack()
{
	return Excecute("rollback transaction");
}

bool CSmartDB::Commit()
{
	return Excecute("commit transaction");
}

bool CSmartDB::Open(const string& strFileName)
{
	m_code = sqlite3_open(strFileName.data(), &m_dbHandle);
	return (SQLITE_OK == m_code);
}

bool CSmartDB::Close()
{
	if (nullptr == m_dbHandle)
	{
		return true;
	}
	
	sqlite3_finalize(m_statement);
	m_code = CloseDBHandle();
	m_dbHandle = nullptr;
	return (SQLITE_OK == m_code);
}

int CSmartDB::GetLastErrorCode()
{
	return m_code;
}

int CSmartDB::CloseDBHandle()
{
	int code = sqlite3_close(m_dbHandle);
	while (code == SQLITE_BUSY)
	{
		code = SQLITE_OK;
		sqlite3_stmt *stmt = sqlite3_next_stmt(m_dbHandle, NULL);
		
		if (nullptr == stmt)
		{
			break;
		}
		
		code = sqlite3_finalize(stmt);
		if (SQLITE_OK == code)
		{
			code = sqlite3_close(m_dbHandle);
		}
	}
	return code;
}

bool CSmartDB::Prepare(const std::string& sqlStr)
{
	m_code = sqlite3_prepare_v2(m_dbHandle, sqlStr.data(), -1, &m_statement, nullptr);
	if (SQLITE_OK != m_code)
	{
		return false;
	}
	return true;
}

int CSmartDB::BindParams(sqlite3_stmt* statement, int current)
{
	return SQLITE_OK;
}

bool CSmartDB::Excecute(const string& sqlStr)
{
	m_code = sqlite3_exec(m_dbHandle, sqlStr.data(), nullptr, nullptr,nullptr);
	return SQLITE_OK == m_code;
}

SqliteValue CSmartDB::GetValue(sqlite3_stmt* stmt, const int& index)
{
	int type = sqlite3_column_type(stmt, index);
	//根据列的类型取值
	auto it = m_valMap.find(type);
	if (it == m_valMap.end())
	{
		throw std::invalid_argument("can not find sqlite value type.");
	}
	
	return it->second(stmt, index);
}

bool CSmartDB::ExcecuteJson(const std::string& sqlStr, const char* json)
{
	//解析json串
	rapidjson::Document doc;
	if (doc.Parse<0>(json).HasParseError())
	{
		return false;
	}
	
	//解析SQL语句
	if (!Prepare(sqlStr))
	{
		return false;
	}
	
	//启用事务写数据
	return JsonTransaction(doc);
}

bool CSmartDB::JsonTransaction(const rapidjson::Document& doc)
{
	//事务启动
	Begin();
	
	//解析json对象
	for (size_t i = 0, size = doc.Size(); i < size; i++)
	{
		if (!ExcecuteJson(doc[i]))
		{
			RollBack();
			break;
		}
	}
	
	if (SQLITE_DONE != m_code)
	{
		return false;
	}
	
	Commit();
	return true;
}

bool CSmartDB::ExcecuteJson(const rapidjson::Value& val)
{
	using MemberIterType = rapidjson::Value::ConstMemberIterator;
	size_t i = 0;
	for (MemberIterType ite = val.MemberBegin();ite != val.MemberEnd(); i++)
	{
		//获取json值
		const char* key = val.GetKey(ite++);
		auto& t = val[key];
		
		//绑定json值
		BindJsonValue(t, i + 1);
	}
	
	m_code = sqlite3_step(m_statement);
	sqlite3_reset(m_statement);
	return SQLITE_DONE == m_code;
}

void CSmartDB::BindJsonValue(const rapidjson::Value& t, int index)
{
	auto type = t.GetType();
	if (type == rapidjson::kNullType)
	{
		m_code = sqlite3_bind_null(m_statement, index);
	}
	else if (type == rapidjson::kStringType)
	{
		m_code = sqlite3_bind_text(m_statement, index, t.GetString(), -1, SQLITE_STATIC);
	}
	else if (type == rapidjson::kNumberType)
	{
		BindNumber(t, index);
	}
	else
	{
		throw std::invalid_argument("can not find this type.");
	}
}

void CSmartDB::BindNumber(const rapidjson::Value& t, int index)
{
	if (t.IsInt() || t.IsUint())
	{
		m_code = sqlite3_bind_int(m_statement, index, t.GetInt());
	}
	else if (t.IsInt64() || t.IsUint64())
	{
		m_code = sqlite3_bind_int64(m_statement, index, t.GetInt64());
	}
	else
	{
		m_code = sqlite3_bind_double(m_statement, index, t.GetDouble());
	}
}

void CSmartDB::BuildJsonObject(JsonBuilder& jBuilder)
{
	int colCount = sqlite3_column_count(m_statement);
	
	jBuilder.StartArray();
	while (true)
	{
		m_code = sqlite3_step(m_statement);
		if (SQLITE_DONE == m_code)
		{
			break;
		}
		
		BuildJsonArray(colCount,jBuilder);
	}
	
	jBuilder.EndArray();
	sqlite3_reset(m_statement);
}

void CSmartDB::BuildJsonArray(const int& colCount, JsonBuilder& jBuilder)
{
	jBuilder.StartObject();
	for (int i = 0; i < colCount; ++i)
	{
		char* name = (char*)sqlite3_column_name(m_statement, i);
		
		jBuilder.String(name);
		BuildJsonValue(m_statement, i,jBuilder);
	}
	
	jBuilder.EndObject();
}

//创建json值
void CSmartDB::BuildJsonValue(sqlite3_stmt* stmt, int index, JsonBuilder& jBuilder)
{
	int type = sqlite3_column_type(stmt, index);
	auto it = m_builderMap.find(type);
	if (it == m_builderMap.end())
	{
		throw std::invalid_argument("can not find this type.");
	}
	
	it->second(stmt, index, jBuilder);
}