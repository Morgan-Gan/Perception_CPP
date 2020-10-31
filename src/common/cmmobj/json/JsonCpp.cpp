#include "JsonCpp.h"

using namespace std;
using namespace rapidjson;
using namespace common_cmmobj;

CJsonCpp::CJsonCpp() : m_writer(m_buf)
{
}

CJsonCpp::~CJsonCpp()
{
}

void CJsonCpp::StartArray()
{
	m_writer.StartArray();
}

void CJsonCpp::EndArray()
{
	m_writer.EndArray();
}

void CJsonCpp::StartObject()
{
	m_writer.StartObject();
}

void CJsonCpp::EndObject()
{
	m_writer.EndObject();
}

void CJsonCpp::String(const char* pData)
{
	m_writer.String(pData);
}