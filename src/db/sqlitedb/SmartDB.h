#pragma once
#include <string>
#include <unordered_map>
#include <iostream>
#include <memory>
#include "comm/NonCopyable.h"
#include "comm/Variant.h"
#include "sqlite3/sqlite3.h"
#include "json/JsonCpp.h"

namespace SQLITE_DB
{
	using SqliteValue = common_template::Variant<double, int, std::uint32_t, sqlite3_int64, char*, const char*, void*/*blob*/, std::string, std::nullptr_t>;

	using JsonBuilder = common_cmmobj::CJsonCpp;

	template<int...>
	struct IndexTuple {};
		
	template<int N, int... Indexes>
	struct MakeIndexes : MakeIndexes<N - 1, N - 1, Indexes...> {};
						
	template<int... indexes>
	struct MakeIndexes<0, indexes...>
	{
		using type = IndexTuple<indexes...>;
	};
	
	class CSmartDB : public common_template::NonCopyable
	{
	public:
		CSmartDB() = default;
		~CSmartDB() {Close();}
		
		/**
		* 功能：创建或打开数据库
		* 如数据库不存在，则创建并打开
		*/
		explicit CSmartDB(const std::string& strFileName);	
		
		/*功能：打开数据库*/
		bool Open(const std::string& strFileName);

		/*功能：释放资源，关闭数据库*/
		bool Close();

		/*
		 * 功能：不带占位符，执行SQL，不带返回结果,如 insert、update、delete等
		 * @param[in] sqlStr:SQL语句,不带占位符
		 * @return bool,成功返回true,失败返回false
		 */
		bool Excecute(const std::string& sqlStr);
		
		/*
		 *功能：带占位符,执行SQL,不带返回结果，如 insert、update、delete等
		 * @param[in] sqlStr:SQL语句,可能带占位符"?"
		 * @param[in] args: 参数列表，用来填充占位符
		 * @return bool,成功返回true,失败返回false
		 */
		template<typename... Args>
		bool Excecute(const std::string& strSql, Args&&... args);
		
		/*
		*功能:执行sql,返回函数执行的一个值，执行简单的汇聚函数,如select count(x),select max(*)等,返回结果可能有多种类型,返回value类型，在外面通过get函数去
		* @param[in]: query:sql语句，用来填充占位符"?"
		* @param[in]: args:参数列表，用来填充占位符
		* @return R:返回结果值，失败则返回无效值
		*/
		template<typename R = sqlite_int64, typename... Args> R ExecuteScalar(const std::string& sqlStr, Args&&... args)
		{
			if (!Prepare(sqlStr))	
			{
				return GetErrorVal<R>();
			}
			
			//绑定sql脚本中的参数
			if (SQLITE_OK != BindParams(m_statement, 1, std::forward<Args>(args)...))
			{
				return GetErrorVal<R>();
			}
			
			m_code = sqlite3_step(m_statement);
			if (SQLITE_ROW != m_code)
			{
				return GetErrorVal<R>();
			}
			
			SqliteValue val = GetValue(m_statement, 0);
			R result = val.Get<R>();
			sqlite3_reset(m_statement);
			return result;
		}
		
		/* 
		* 功能：tuple的执行接口
		*/
		template<typename Tuple>
		bool ExcecuteTuple(const std::string& sqlStr, Tuple&& t);
		
		/*功能： json接口*/
		bool ExcecuteJson(const std::string& sqlStr, const char* json);
		
		/*json查询接口*/
		template<typename... Args>
		void Query(const std::string&, rapidjson::Document&,Args&&... args);
		
	private:
		/*
		* 功能：事务接口
		*/
		bool Begin();
		bool RollBack();
		bool Commit();

		/*获取最后的错误码*/
		int GetLastErrorCode();
		
		/*关闭数据库句柄*/
		int CloseDBHandle();

		/*
		* 功能：解释和保存SQL,可能带占位符
		* @param[in] query: SQL语句,可能带占位符“？”
		* @return bool,成功返回true,失败返回false
		*/
		bool Prepare(const std::string& sqlStr);
		
		/*取列的值*/
		SqliteValue GetValue(sqlite3_stmt* stmt, const int& index);

		/*绑定json值*/
		void BindJsonValue(const rapidjson::Value& t, int index);
		
		/*绑定*/
		void BindNumber(const rapidjson::Value& t, int index);
		
		/*创建json对象*/
		void BuildJsonObject(JsonBuilder& jBuilder);
		
		/*创建json值*/
		void BuildJsonValue(sqlite3_stmt* stmt, int index, JsonBuilder& jBuilder);
		
		/*创建json对象列表*/
		void BuildJsonArray(const int& colCount, JsonBuilder& jBuilder);
		
		/*json执行*/
		bool ExcecuteJson(const rapidjson::Value& val);
		
		/*功能: 通过json串写到数据库中*/
		bool JsonTransaction(const rapidjson::Document& doc);
		
	private:
		/*功能:绑定参数*/
		int BindParams(sqlite3_stmt* statement, int current);
		
		template<typename T, typename... Args>
		int BindParams(sqlite3_stmt* statement, int current, T&& first, Args&&... args)
		{
			BindValue(statement, current, first);
			if (SQLITE_OK != m_code)
			{
				return m_code;
			}
		
			BindParams(statement, current + 1, std::forward<Args>(args)...);
			return m_code;
		}

		template<typename... Args>
		bool PrepareStatement(const std::string& query, Args&&... args);

		/*返回无效值*/
		template<typename T>
		typename std::enable_if<std::is_arithmetic<T>::value, T>::type GetErrorVal() 
		{
			return T(-9999);
		}
		
		template<typename T>
		typename std::enable_if<!std::is_arithmetic<T>::value, T>::type GetErrorVal() 
		{
			return "";
		}
		
		template<typename T>
		typename std::enable_if<std::is_floating_point<T>::value>::type BindValue(sqlite3_stmt* statement, int current, T t)
		{
			m_code = sqlite3_bind_double(statement, current, std::forward<T>(t));
		}
		
		template<typename T>
		typename std::enable_if<std::is_integral<T>::value>::type BindValue(sqlite3_stmt* statement, int current, T t)
		{
			BindIntValue(statement, current, t);
		}
		
		template<typename T>
		typename std::enable_if<std::is_same<std::string, T>::value>::type BindValue(sqlite3_stmt* statement, int current, const T& t)
		{
			m_code = sqlite3_bind_text(statement, current, t.data(), t.length(), SQLITE_TRANSIENT);
		}
		
		template<typename T>
		typename std::enable_if<std::is_same<std::nullptr_t, T>::value>::type BindValue(sqlite3_stmt* statement, int current, const T& t)
		{
			m_code = sqlite3_bind_null(statement, current);
		}
		
		template<typename T>
		typename std::enable_if<std::is_same<char*, T>::value || std::is_same<const char*, T>::value>::type BindValue(sqlite3_stmt* statement, int current, const T& t)
		{
			m_code = sqlite3_bind_text(statement, current, t, strlen(t) + 1, SQLITE_TRANSIENT);
		}
		
		template<typename T>
		typename std::enable_if<std::is_same<T, int64_t>::value || std::is_same<T, uint64_t>::value>::type BindIntValue(sqlite3_stmt* statement, int current, T t)
		{
			m_code = sqlite3_bind_int64(statement, current, std::forward<T>(t));
		}
		
		template<typename T>
		typename std::enable_if<!std::is_same<T, int64_t>::value && !std::is_same<T, uint64_t>::value>::type BindIntValue(sqlite3_stmt* statement, int current, T t)
		{
			m_code = sqlite3_bind_int(statement, current, std::forward<T>(t));
		}
			
		/*
		功能：批量操作接口,必须先调用Prepare接口
		* @param[in]:可变参数
		* @return bool,成功返回true,失败返回false
		*/
		template<typename... Args>
		bool ExcecuteArgs(Args&&... args);
		
		/*
		* 
		*/
		template<int... Indexes, class Tuple>
		bool ExcecuteTuple(IndexTuple<Indexes...>&& in, Tuple&& t);
		
	private:
		int m_code;
		sqlite3* m_dbHandle;
		sqlite3_stmt* m_statement;
		static std::unordered_map<int, std::function<SqliteValue(sqlite3_stmt*, int)>> m_valMap;

		static std::unordered_map<int, std::function<void(sqlite3_stmt *stmt, int index, JsonBuilder&)>> m_builderMap;
	};
	
	template<typename... Args>
	inline	bool CSmartDB::Excecute(const std::string& strSql,Args&&... args)
	{
		if (!Prepare(strSql))
		{
			return false;
		}
		
		return ExcecuteArgs(std::forward<Args>(args)...);
	}
	
	template<typename... Args>
	inline	bool CSmartDB::ExcecuteArgs(Args&&... args)
	{
		if (SQLITE_OK != BindParams(m_statement, 1, std::forward<Args>(args)...))
		{
			return false;
		}
		
		m_code = sqlite3_step(m_statement);
		
		sqlite3_reset(m_statement);
		return m_code == SQLITE_DONE;
	}
	
	template<typename Tuple>
	inline bool CSmartDB::ExcecuteTuple(const std::string& sqlStr, Tuple&& t)
	{
		if (!Prepare(sqlStr))
		{
			return false;
		}
		return ExcecuteTuple(typename MakeIndexes<std::tuple_size<Tuple>::value>::type(), std::forward<Tuple>(t));
	}
	
	template<int... Indexes, class Tuple>
	inline bool CSmartDB::ExcecuteTuple(IndexTuple<Indexes...>&& in, Tuple&& t)
	{
		if(SQLITE_OK != BindParams(m_statement,1,std::get<Indexes>(std::forward<Tuple>(t))...))
		{
			return false;	
		}
		
		m_code = sqlite3_step(m_statement);
		sqlite3_reset(m_statement);
		return SQLITE_DONE == m_code;
	}
	
	template<typename... Args>
	inline bool CSmartDB::PrepareStatement(const std::string& query, Args&&... args)
	{
		return Prepare(query);
	}

	template<typename... Args>
	inline void CSmartDB::Query(const std::string& query, rapidjson::Document& doc, Args&&... args)
	{
		if (!PrepareStatement(query, std::forward<Args>(args)...))
		{
			return;
		}
		
		//将查询结果保存为json对象
		JsonBuilder jBuilder;
		BuildJsonObject(jBuilder);
		const char* pData = jBuilder.GetString();
		doc.Parse<0>(pData);
	}
}