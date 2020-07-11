#include "CfgMng.h"
#include "SubTopic.h"
#include "log4cxx/Loging.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;
using namespace MAIN_MNG;
using namespace common_cmmobj;

bool CCfgMng::LoadCfg()
{
	LoadJsonCfg("./config/config.json");

	return ParseJsonCfg();
}

void CCfgMng::GetCfgJsonObj(const std::string &strKey, std::vector<Json> &lstCfg)
{
	if (m_mapCfgObj.end() != m_mapCfgObj.find(strKey))
	{
		m_mapCfgObj[strKey].GetJsonObj(lstCfg);
	}
}

//加载json配置
bool CCfgMng::LoadJsonCfg(const string &strPath)
{
	string strJson;
	ReadJsonFile(strPath, strJson);

	if (strJson.empty())
	{
		return std::false_type::value;
	}

	m_jsObj = nlohmann::json::parse(strJson.c_str());

	return std::true_type::value;
}

void CCfgMng::ReadJsonFile(const string &strPath, string &strJson)
{
	//从文件中读取
	ifstream fin(strPath.c_str(), ios::binary);

	//创建字符串流对象
	ostringstream sin;

	//把文件流中的字符输入到字符串流中
	sin << fin.rdbuf();

	//获取字符串流中的字符串
	strJson = sin.str();

	//关闭和清除文件流对象
	fin.close();
	fin.clear();
}

bool CCfgMng::ParseJsonCfg()
{
	//transfer cfg
	ParseJsonNode(strTransferNode, strTransferSouthNode);
	ParseJsonNode(strTransferNode, strTransferNorthNode);

	//alg cfg
	ParseJsonNode(strAlgNode, strAlgDetectNode);
	ParseJsonNode(strAlgNode, strAlgActionRecgNode);

	//db cfg
	ParseJsonNode(strDbNode,strDbRemoteNode);

	return true;
}

void CCfgMng::ParseJsonNode(const string &strNode, const string &strSubNode)
{
	for (auto elm : m_jsObj[strNode][strSubNode])
	{
		string &&strKey = strNode + strSubNode + string(elm["type"]);
		if (m_mapCfgObj.end() == m_mapCfgObj.find(strKey))
		{
			CfgObj cfgObj;
			cfgObj.PushJsonObj(elm);
			m_mapCfgObj.insert(pair<string, CfgObj>(strKey, cfgObj));
		}
		else
		{
			m_mapCfgObj[strKey].PushJsonObj(elm);
		}
	}
}