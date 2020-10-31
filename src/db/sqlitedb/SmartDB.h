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
		* ���ܣ�����������ݿ�
		* �����ݿⲻ���ڣ��򴴽�����
		*/
		explicit CSmartDB(const std::string& strFileName);	
		
		/*���ܣ������ݿ�*/
		bool Open(const std::string& strFileName);

		/*���ܣ��ͷ���Դ���ر����ݿ�*/
		bool Close();

		/*
		 * ���ܣ�����ռλ����ִ��SQL���������ؽ��,�� insert��update��delete��
		 * @param[in] sqlStr:SQL���,����ռλ��
		 * @return bool,�ɹ�����true,ʧ�ܷ���false
		 */
		bool Excecute(const std::string& sqlStr);
		
		/*
		 *���ܣ���ռλ��,ִ��SQL,�������ؽ������ insert��update��delete��
		 * @param[in] sqlStr:SQL���,���ܴ�ռλ��"?"
		 * @param[in] args: �����б��������ռλ��
		 * @return bool,�ɹ�����true,ʧ�ܷ���false
		 */
		template<typename... Args>
		bool Excecute(const std::string& strSql, Args&&... args);
		
		/*
		*����:ִ��sql,���غ���ִ�е�һ��ֵ��ִ�м򵥵Ļ�ۺ���,��select count(x),select max(*)��,���ؽ�������ж�������,����value���ͣ�������ͨ��get����ȥ
		* @param[in]: query:sql��䣬�������ռλ��"?"
		* @param[in]: args:�����б��������ռλ��
		* @return R:���ؽ��ֵ��ʧ���򷵻���Чֵ
		*/
		template<typename R = sqlite_int64, typename... Args> R ExecuteScalar(const std::string& sqlStr, Args&&... args)
		{
			if (!Prepare(sqlStr))	
			{
				return GetErrorVal<R>();
			}
			
			//��sql�ű��еĲ���
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
		* ���ܣ�tuple��ִ�нӿ�
		*/
		template<typename Tuple>
		bool ExcecuteTuple(const std::string& sqlStr, Tuple&& t);
		
		/*���ܣ� json�ӿ�*/
		bool ExcecuteJson(const std::string& sqlStr, const char* json);
		
		/*json��ѯ�ӿ�*/
		template<typename... Args>
		void Query(const std::string&, rapidjson::Document&,Args&&... args);
		
	private:
		/*
		* ���ܣ�����ӿ�
		*/
		bool Begin();
		bool RollBack();
		bool Commit();

		/*��ȡ���Ĵ�����*/
		int GetLastErrorCode();
		
		/*�ر����ݿ���*/
		int CloseDBHandle();

		/*
		* ���ܣ����ͺͱ���SQL,���ܴ�ռλ��
		* @param[in] query: SQL���,���ܴ�ռλ��������
		* @return bool,�ɹ�����true,ʧ�ܷ���false
		*/
		bool Prepare(const std::string& sqlStr);
		
		/*ȡ�е�ֵ*/
		SqliteValue GetValue(sqlite3_stmt* stmt, const int& index);

		/*��jsonֵ*/
		void BindJsonValue(const rapidjson::Value& t, int index);
		
		/*��*/
		void BindNumber(const rapidjson::Value& t, int index);
		
		/*����json����*/
		void BuildJsonObject(JsonBuilder& jBuilder);
		
		/*����jsonֵ*/
		void BuildJsonValue(sqlite3_stmt* stmt, int index, JsonBuilder& jBuilder);
		
		/*����json�����б�*/
		void BuildJsonArray(const int& colCount, JsonBuilder& jBuilder);
		
		/*jsonִ��*/
		bool ExcecuteJson(const rapidjson::Value& val);
		
		/*����: ͨ��json��д�����ݿ���*/
		bool JsonTransaction(const rapidjson::Document& doc);
		
	private:
		/*����:�󶨲���*/
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

		/*������Чֵ*/
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
		���ܣ����������ӿ�,�����ȵ���Prepare�ӿ�
		* @param[in]:�ɱ����
		* @return bool,�ɹ�����true,ʧ�ܷ���false
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
		
		//����ѯ�������Ϊjson����
		JsonBuilder jBuilder;
		BuildJsonObject(jBuilder);
		const char* pData = jBuilder.GetString();
		doc.Parse<0>(pData);
	}
}