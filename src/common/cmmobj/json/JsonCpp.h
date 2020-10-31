#pragma once
#include <string>
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"

namespace common_cmmobj
{
	class CJsonCpp
	{
		using JsonWriter = rapidjson::Writer<rapidjson::StringBuffer>;
	public:
		CJsonCpp();
		~CJsonCpp();
		
		/*���л��ṹ������֮ǰ��Ҫ���ýӿڣ�Ȼ����ѭ��ȥ���л�*/
		void StartArray();
		void EndArray();
		void StartObject();
		void EndObject();
		void String(const char* pData);
	
	public:
		//д��ֵ��
		template<typename T>
		void WriteJson(std::string& key, T&& value)
		{
			m_writer.Key(key.c_str());
			WriteValue(std::forward<T>(value));
		}

		template<typename T>
		void WriteJson(const char* key, T&& value)
		{
			m_writer.Key(key);
			WriteValue(std::forward<T>(value));
		}
		
		template<typename V>
		typename std::enable_if<std::is_same<V, int>::value>::type WriteValue(V value)
		{
			m_writer.Int(value);
		}

		template<typename V>
		typename std::enable_if<std::is_same<V, unsigned int>::value>::type WriteValue(V value)
		{
			m_writer.Uint(value);
		}
		
		template<typename V>
		typename std::enable_if<std::is_same<V, int64_t>::value>::type WriteValue(V value)
		{
			m_writer.Int64(value);
		}
		
		template<typename V>
		typename std::enable_if<std::is_same<V, bool>::value>::type WriteValue(V value)
		{
			m_writer.Bool(value);
		}

		template<typename V>
		typename std::enable_if<std::is_same<V, std::nullptr_t>::value>::type WriteValue(V value)
		{
			m_writer.Null();	
		}

		template<typename V>
		typename std::enable_if<std::is_floating_point<V>::value>::type WriteValue(V value)
		{
			m_writer.Double(value);
		}

		template<typename V>
		typename std::enable_if<std::is_array<V>::value>::type WriteValue(V value)
		{
			m_writer.String(value);	
		}

		template<typename V>
		typename std::enable_if<std::is_same<V, const char*>::value>::type WriteValue(V value)
		{
			m_writer.String(value);	
		}

		inline void Key(const char* pKey)
		{
			m_writer.Key(pKey);
		}

		inline void Double(double value)
		{
			m_writer.Double(value);
		}

		inline void Int64(int64_t value)
		{
			m_writer.Int64(value);
		}

		inline void Null()
		{
			m_writer.Null();
		}

		/*���ض������л����json�ַ���*/
		const char* GetString() const
		{
			return m_buf.GetString();
		}
		
	private:
		rapidjson::StringBuffer m_buf; //json�ַ�����buffer
		JsonWriter m_writer;           //jsonд����
		rapidjson::Document m_doc;
	};
}