#include "CfgObj.h"

using namespace std;
using namespace MAIN_MNG;

void CfgObj::SaveJsonObj(const Json& jsCfg)
{
    m_lstJsonObj.emplace_back(jsCfg);
}

void CfgObj::GetJsonObj(vector<Json>& lstJson)
{
    for(auto val : m_lstJsonObj)
    {
        lstJson.emplace_back(val);
    }
}

bool CfgObj::GetJsonObj(Json& jsonCfg)
{
    if(!m_lstJsonObj.empty())
    {
        jsonCfg = m_lstJsonObj[0];
        return true;
    }
    return false;
}

CfgObj& CfgObj::operator=(const CfgObj& obj)
{
    if(this != &obj)
    {
        m_lstJsonObj.assign(obj.m_lstJsonObj.begin(),obj.m_lstJsonObj.end());
    }
}