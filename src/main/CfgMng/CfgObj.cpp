#include "CfgObj.h"

using namespace std;
using namespace MAIN_MNG;

void CfgObj::PushJsonObj(const Json& jsCfg)
{
    m_lstJsonObj.emplace_back(jsCfg);
}

void CfgObj::GetJsonObj(vector<Json>& lstJson)
{
    lstJson.assign(m_lstJsonObj.begin(),m_lstJsonObj.end());
}

CfgObj& CfgObj::operator=(const CfgObj& obj)
{
    if(this != &obj)
    {
        m_lstJsonObj.assign(obj.m_lstJsonObj.begin(),obj.m_lstJsonObj.end());
    }
}