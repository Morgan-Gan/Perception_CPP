#pragma once
#include <string>
#include <vector>
#include "comm/CommDefine.h"

namespace MAIN_MNG
{
    class CfgObj
    {
    private:
        std::vector<Json> m_lstJsonObj;

    public:
        CfgObj& operator=(const CfgObj& obj);
        
        //插入配置记录
        void SaveJsonObj(const Json& jsCfg);

        //取出配置记录
        void GetJsonObj(std::vector<Json>& lstJson);

        bool GetJsonObj(Json& jsonCfg);

    public:
        CfgObj() = default;
        ~CfgObj() = default;
    };
}